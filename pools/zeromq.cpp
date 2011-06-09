#include "zeromq.hpp"

#include <algorithm>

#include <cstring>

#include <boost/lexical_cast.hpp>

#include <unistd.h>

#include "repository.hpp"
#include "tempfile.hpp"
#include "execute.hpp"

// factory

bunsan::worker::pool_ptr instance(const boost::property_tree::ptree &config)
{
	bunsan::worker::pool_ptr tmp(new bunsan::worker::pools::zeromq(config));
	return tmp;
}

bool bunsan::worker::pools::zeromq::factory_reg_hook = bunsan::worker::pool::register_new("zeromq",
	[](const boost::property_tree::ptree &config)
	{
		bunsan::worker::pool_ptr tmp(new bunsan::worker::pools::zeromq(config));
		return tmp;
	});

// virtual class

bunsan::worker::pools::zeromq::zeromq(const boost::property_tree::ptree &config):
	iothreads(config.get<int>("iothreads")),
	worker_port(config.get<unsigned>("worker.port")),
	queue_port(config.get<unsigned>("queue.port")),
	stop_check_interval(config.get<unsigned long>("stop_check_interval")),
	workers(config.get<size_t>("size")),
	repository_config(config.get_child("repository")),
	worker_tempdir(config.get<std::string>("worker.tmp")),
	uri(config.get<std::string>("uri"))
{
	DLOG(creating zeromq pool instance);
	SLOG("attempt to create hub of "<<config.get<std::string>("hub.type")<<" type");
	hub = bunsan::dcs::hub::instance(config.get<std::string>("hub.type"), config.get_child("hub.config")); // you should use proxy hub
	if (!hub)
		throw std::runtime_error("hub was not created");
	hub->start();
	to_stop.store(false);
	capacity.store(0);
	add_to_hub();
	context.reset(new zmq::context_t(iothreads));
	try
	{
		check_dirs();
		queue = std::thread(&bunsan::worker::pools::zeromq::queue_func, this);
		for (std::shared_ptr<std::thread> &w: workers)
		{
			w.reset(new std::thread(&bunsan::worker::pools::zeromq::worker_func, this));
		}
	}
	catch (std::exception &e)
	{
		to_stop.store(true);
		if (queue.joinable())
			queue.join();
		throw;
	}
}

void bunsan::worker::pools::zeromq::check_dirs()
{
	bunsan::reset_dir(worker_tempdir);
}

void bunsan::worker::pools::zeromq::check_running()
{
	if (to_stop.load())
		throw std::runtime_error("pool execution has already completed");
}

void bunsan::worker::pools::zeromq::add_task(const std::string &callback, const std::string &package, const std::vector<std::string> &args)
{
	check_running();
	zmq::socket_t push(*context, ZMQ_PUSH);
	push.connect(("tcp://localhost:"+boost::lexical_cast<std::string>(queue_port)).c_str());
	std::vector<zmq::message_t> task(args.size()+2);
	task[0].rebuild(callback.size());
	memcpy(task[0].data(), callback.c_str(), callback.size());
	task[1].rebuild(package.size());
	memcpy(task[1].data(), package.c_str(), package.size());
	for (size_t i = 0; i<args.size(); ++i)
	{
		task[i+2].rebuild(args[i].size());
		memcpy(task[i+2].data(), args[i].c_str(), args[i].size());
	}
	for (size_t i = 0; i<task.size(); ++i)
		push.send(task[i], i+1==task.size()?0:ZMQ_SNDMORE);
}

class reportable_error: public std::runtime_error// TODO is it needed?
{
public:
	reportable_error(const std::string &callback);
};

void bunsan::worker::pools::zeromq::queue_func()
{
	try
	{
		zmq::socket_t pull(*context, ZMQ_PULL);
		zmq::socket_t rep(*context, ZMQ_REP);
		pull.bind(("tcp://*:"+boost::lexical_cast<std::string>(queue_port)).c_str());
		rep.bind(("tcp://*:"+boost::lexical_cast<std::string>(worker_port)).c_str());
		while (true)
		{
			zmq::pollitem_t rep_item[] =
			{
				{rep, 0, ZMQ_POLLIN, 0}
			};
			do
			{
				DLOG(tick);
				if (to_stop.load())
					break;
				zmq::poll(rep_item, 1, stop_check_interval);
				if (to_stop.load())
					break;
			} while (!(rep_item[0].revents & ZMQ_POLLIN));
			DLOG(attached to new worker);
			int more;
			size_t more_size = sizeof(more);
			do
			{
				zmq::message_t message;
				rep.recv(&message);
				rep.getsockopt(ZMQ_RCVMORE, &more, &more_size);
			} while (more);
			DLOG(waiting for task);
			zmq::pollitem_t pull_item[] =
			{
				{pull, 0, ZMQ_POLLIN, 0}
			};
			do
			{
				DLOG(tick);
				if (to_stop.load())
					break;
				zmq::poll(pull_item, 1, stop_check_interval);
				if (to_stop.load())
					break;
			} while (!(pull_item[0].revents & ZMQ_POLLIN));
			do
			{
				zmq::message_t message;
				pull.recv(&message);
				DLOG(receiving task...);
				pull.getsockopt(ZMQ_RCVMORE, &more, &more_size);
				rep.send(message, more?ZMQ_SNDMORE:0);
			}
			while (more);
			DLOG(task was submitted);// TODO inform callback
		}
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
	try
	{
		remove_from_hub();
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
	try
	{
		hub->stop();
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
	try
	{
		for (std::shared_ptr<std::thread> &t: workers)
			if (t->joinable())
				t->join();
		context.reset();
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
}

void bunsan::worker::pools::zeromq::worker_func()
{
	while (true)
	{
		try
		{
			zmq::socket_t req(*context, ZMQ_REQ);
			req.connect(("tcp://localhost:"+boost::lexical_cast<std::string>(worker_port)).c_str());
			zmq::pollitem_t item[] =
			{
				{req, 0, ZMQ_POLLIN, 0}
			};
			{
				zmq::message_t message(0);
				req.send(message);
			}
			try
			{
				register_worker();
				do
				{
					DLOG(tick);
					if (to_stop.load())
						break;
					zmq::poll(item, 1, stop_check_interval);
					if (to_stop.load())
						break;
				} while (!(item[0].revents & ZMQ_POLLIN));
			} catch (std::exception &e)
			{
				unregister_worker();
				throw;
			}
			unregister_worker();
			int more;
			size_t more_size = sizeof(more);
			std::vector<std::string> task;
			do
			{
				zmq::message_t message;
				req.recv(&message);
				req.getsockopt(ZMQ_RCVMORE, &more, &more_size);
				std::string msg(message.size(), '\0');
				for (size_t i = 0; i<message.size(); ++i)
					msg[i] = static_cast<char *>(message.data())[i];
				task.push_back(std::move(msg));
			} while (more);
			DLOG(attempt to do);
			for (const std::string &i: task)
				SLOG('\t'<<i);
			DLOG(======================================);
			do_task(task);
		}
		catch (std::exception &e)
		{
			SLOG("Oops! \""<<e.what()<<"\"");
		}
	}
}

void bunsan::worker::pools::zeromq::do_task(const std::vector<std::string> &task)
{
	// TODO callback may be informed here
	if (task.size()<2)
		throw std::runtime_error("task incorrect format: too small, FIXME: this should not happen");
	DLOG(extracting task);
	std::string callback = task[0];
	std::string package = task[1];
	std::vector<std::string> args(task.size()-2);
	std::copy(task.begin()+2, task.end(), args.begin());
	DLOG(extracted);// TODO inform callback
#warning inform callback on error
	DLOG(preparing package manager);
	boost::property_tree::ptree repo_config = repository_config.get_child("pm");
	boost::optional<std::string> uri_substitution = repository_config.get_optional<std::string>("uri_substitution");
	if (uri_substitution)
	{
		std::string repo_uri = hub->select_resource(repo_config.get<std::string>("resource_name"));
		repo_config.put(uri_substitution.get(), repo_uri);
	}
	bunsan::pm::repository repo(repo_config);
	bunsan::tempfile_ptr tmpdir = bunsan::tempfile::in_dir(worker_tempdir);
	bunsan::reset_dir(tmpdir->path());
	repo.extract(package, tmpdir->path());
	bunsan::process_ptr process = bunsan::async_execute(tmpdir->path(), args, false);// TODO inform callback
	process->wait();// TODO timeout
	if (process->return_code())
		SLOG("process return code is not null: \""<<process->return_code()<<"\"");
	else
		DLOG(completed);// TODO inform callback
}

void bunsan::worker::pools::zeromq::add_to_hub()
{
#warning TODO
}

void bunsan::worker::pools::zeromq::hub_update()
{
#warning TODO
}

void bunsan::worker::pools::zeromq::register_worker()
{
	++capacity;
	hub_update();
}

void bunsan::worker::pools::zeromq::unregister_worker()
{
	--capacity;
	hub_update();
}

void bunsan::worker::pools::zeromq::remove_from_hub()
{
#warning TODO
}

void bunsan::worker::pools::zeromq::join()
{
	queue.join();
}

void bunsan::worker::pools::zeromq::stop()
{
	to_stop.store(true);
}

bunsan::worker::pools::zeromq::~zeromq()
{
	stop();
	join();
}

