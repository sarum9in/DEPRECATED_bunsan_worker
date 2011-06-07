#include "zeromq.hpp"

#include <cstring>

#include <boost/lexical_cast.hpp>

// factory

bunsan::worker::pool_ptr instance(const boost::property_tree::ptree &config)
{
	bunsan::worker::pool_ptr tmp(new bunsan::worker::pools::zeromq(config));
	return tmp;
}

bunsan::runner bunsan::worker::pools::zeromq::reg(bunsan::worker::pool::register_new, "zeromq", ::instance);

// virtual class

bunsan::worker::pools::zeromq::zeromq(const boost::property_tree::ptree &config):
	iothreads(config.get<int>("iothreads")),
	worker_port(config.get<unsigned>("worker.port")),
	queue_port(config.get<unsigned>("queue.port")),
	stop_check_interval(config.get<unsigned long>("stop_check_interval")),
	workers(config.get<size_t>("size"))
{
	DLOG(creating zeromq pool instance);
	SLOG("attempt to create hub of "<<config.get<std::string>("hub.type")<<" type");
	hub = bunsan::dcs::hub::instance(config.get<std::string>("hub.type"), config.get_child("hub.config")); // you should use proxy hub
	hub->start();
	to_stop.store(false);
	context.reset(new zmq::context_t(iothreads));
	try
	{
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

void bunsan::worker::pools::zeromq::queue_func()
{
	try
	{
		zmq::socket_t pull(*context, ZMQ_PULL);
		zmq::socket_t rep(*context, ZMQ_REP);
		pull.bind(("tcp://*:"+boost::lexical_cast<std::string>(queue_port)).c_str());
		rep.bind(("tcp://*:"+boost::lexical_cast<std::string>(worker_port)).c_str());
		zmq::pollitem_t item[] =
		{
			{rep, 0, ZMQ_POLLIN, 0}
		};
		while (true)
		{
			if (to_stop.load())
				break;
			zmq::poll(item, 1, stop_check_interval);
			if (to_stop.load())
				break;
			if (item[0].revents & ZMQ_POLLIN)
			{
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
				do
				{
					zmq::message_t message;
					DLOG(receiving task...);
					pull.recv(&message);
					pull.getsockopt(ZMQ_RCVMORE, &more, &more_size);
					rep.send(message, more?ZMQ_SNDMORE:0);
				}
				while (more);
				DLOG(task was submitted);
			}
		}
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
	try
	{
		for (std::shared_ptr<std::thread> &t: workers)
			t->join();
		context.reset();
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
}

#include <unistd.h>
void bunsan::worker::pools::zeromq::worker_func()
{
	zmq::socket_t req(*context, ZMQ_REQ);
	req.connect(("tcp://localhost:"+boost::lexical_cast<std::string>(worker_port)).c_str());
	zmq::pollitem_t item[] =
	{
		{req, 0, ZMQ_POLLIN, 0}
	};
	while (true)
	{
		if (to_stop.load())
			break;
		{
			zmq::message_t message(0);
			req.send(message);
		}
		if (to_stop.load())
			break;
		zmq::poll(item, 1, stop_check_interval);
		if (to_stop.load())
			break;
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
		DLOG(============);
		sleep(3);
		DLOG(completed);
	}
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

