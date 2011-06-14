#include "zeromq.hpp"

#include <sstream>

#include <cstring>

#include <boost/lexical_cast.hpp>

#include <unistd.h>

#include "repository.hpp"
#include "tempfile.hpp"
#include "execute.hpp"
#include "callback.hpp"

#include "zmq_helpers.hpp"

bool bunsan::worker::pools::zeromq::factory_reg_hook = bunsan::worker::pool::register_new("zeromq",
	[](const boost::property_tree::ptree &config)
	{
		bunsan::worker::pool_ptr tmp(new bunsan::worker::pools::zeromq(config));
		return tmp;
	});

bunsan::worker::pools::zeromq::zeromq(const boost::property_tree::ptree &config):
	iothreads(config.get<int>("iothreads")),
	worker_port(config.get<unsigned>("worker.port")),
	queue_port(config.get<unsigned>("queue.port")),
	stop_check_interval(config.get<unsigned long>("stop_check_interval")),
	uri(config.get<std::string>("uri")),
	machine(config.get<std::string>("machine")),
	resources(config.get_child("resources")),
	worker_tempdir(config.get<std::string>("worker.tmp"))
{
	DLOG(creating zeromq pool instance);
	bunsan::reset_dir(worker_tempdir);
	SLOG("attempt to create hub of "<<config.get<std::string>("hub.type")<<" type");
	hub = bunsan::dcs::hub::instance(config.get<std::string>("hub.type"), config.get_child("hub.config")); // you should use proxy hub
	if (!hub)
		throw std::runtime_error("hub was not created");
	hub->start();
	to_stop.store(false);
	capacity.store(0);
	//add_to_hub();
	context.reset(new zmq::context_t(iothreads));
	try
	{
		queue = std::thread(&bunsan::worker::pools::zeromq::queue_func, this);
	}
	catch (std::exception &e)
	{
		to_stop.store(true);
		if (queue.joinable())
			queue.join();
		throw;
	}
}

class interrupted_error: public std::runtime_error
{
	static const char message[];
public:
	interrupted_error(): std::runtime_error(::interrupted_error::message){}
};
const char interrupted_error::message[] = "execution was interrupted";

void bunsan::worker::pools::zeromq::check_running()
{
	if (to_stop.load())
		throw interrupted_error();
}

void bunsan::worker::pools::zeromq::add_task(const std::string &callback_type, const std::string &callback_uri, const std::vector<std::string> &callback_args, const std::string &package, const std::vector<std::string> &args, const boost::optional<std::vector<unsigned char>> &stdin_file)
{
	DLOG(registrating new task);
	check_running();
	zmq::socket_t push(*context, ZMQ_PUSH);
#warning deadlock possible, use ZMQ_LINGER?
	push.connect(("tcp://localhost:"+boost::lexical_cast<std::string>(queue_port)).c_str());
	SLOG("creating callback instance with type=\""<<callback_type<<"\" and uri=\""<<callback_uri<<"\"");
	bunsan::worker::callback_ptr cb = bunsan::worker::callback::instance(callback_type, callback_uri, callback_args);
	if (bunsan::worker::callback::action::abort==bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::received))
	{
		bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::aborted);
		return;
	}
	bunsan::zmq_helpers::send(callback_type, push, ZMQ_SNDMORE);
	bunsan::zmq_helpers::send(callback_uri, push, ZMQ_SNDMORE);
	bunsan::zmq_helpers::send(callback_args, push, ZMQ_SNDMORE);
	bunsan::zmq_helpers::send(package, push, ZMQ_SNDMORE);
	bunsan::zmq_helpers::send(args, push, stdin_file?ZMQ_SNDMORE:0);
	if (stdin_file)
		bunsan::zmq_helpers::send(stdin_file.get(), push, 0);
	if (cb)
	{
		DLOG(informing callback);
		(void) bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::registered);
		DLOG(informed);
	}
	else
		DLOG(bad callback type);
	DLOG(new task was registered);
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
		bunsan::zmq_helpers::socket pull(*context, ZMQ_PULL);
		bunsan::zmq_helpers::socket rep(*context, ZMQ_REP);
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
				check_running();
				zmq::poll(rep_item, 1, stop_check_interval);
				check_running();
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
				check_running();
				zmq::poll(pull_item, 1, stop_check_interval);
				check_running();
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
		//remove_from_hub();
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
	try
	{
		DLOG(stopping hub);
		hub->stop();
	}
	catch (std::exception &e)
	{
		SLOG("Oops! "<<e.what());
	}
}

void add_to_hub(const std::string &prefix, const boost::property_tree::ptree &resources, const std::string &machine, const std::string &uri, bunsan::dcs::hub_ptr hub)
{
	for (const auto &value: resources)
	{
		if (value.second.empty())
			hub->add_resource(machine, prefix+value.first, uri);
		else
			add_to_hub(prefix+value.first, value.second, machine, uri, hub);
	}
}

void bunsan::worker::pools::zeromq::add_to_hub()
{
	DLOG(inserting instance to hub);
	hub->add_machine(machine, 0);
	::add_to_hub("", resources, machine, uri, hub);
}

void bunsan::worker::pools::zeromq::hub_update()
{
	hub->set_capacity(machine, capacity.load());
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
	DLOG(removing instance from hub);
	hub->remove_machine(machine);
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

