#include "zeromq.hpp"

#include <sstream>

#include <cstring>

#include <boost/lexical_cast.hpp>

#include <unistd.h>

#include "bunsan/tempfile.hpp"
#include "bunsan/zmq_helpers.hpp"

#include "bunsan/pm/repository.hpp"

#include "bunsan/worker/callback.hpp"

bool bunsan::worker::pools::zeromq::factory_reg_hook = bunsan::worker::pool::register_new("zeromq",
	[](const boost::property_tree::ptree &config)
	{
		bunsan::worker::pool_ptr tmp(new bunsan::worker::pools::zeromq(config));
		return tmp;
	});

bunsan::worker::pools::zeromq::zeromq(const boost::property_tree::ptree &config):
	iothreads(config.get<unsigned>("iothreads")),
	linger(config.get<int>("linger")),
	worker_port(config.get<unsigned>("worker.port")),
	queue_port(config.get<unsigned>("queue.port")),
	stop_check_interval(config.get<unsigned long>("stop_check_interval"))
{
	DLOG(creating zeromq pool instance);
	to_stop.store(false);
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

namespace
{
	class interrupted_error: public std::runtime_error
	{
		static constexpr const char *message = "execution was interrupted";
	public:
		interrupted_error(): std::runtime_error(::interrupted_error::message){}
	};
}

void bunsan::worker::pools::zeromq::check_running()
{
	if (to_stop.load())
		throw interrupted_error();
}

void bunsan::worker::pools::zeromq::add_task(const std::string &callback_type, const std::string &callback_uri, const std::vector<std::string> &callback_args, const std::string &package, const std::vector<std::string> &args, const boost::optional<std::vector<unsigned char>> &stdin_file)
{
	DLOG(registrating new task);
	check_running();
	bunsan::zmq_helpers::socket push(*context, ZMQ_PUSH);
#warning TODO check for deadlock
	push.set_linger(linger);
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
		if (bunsan::worker::callback::action::bad==bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::registered))
			DLOG(unable to inform invalid callback);
		else
			DLOG(informed);
	}
	else
		DLOG(bad callback type);
	DLOG(new task was registered);
}

namespace
{
	class reportable_error: public std::runtime_error// TODO is it needed?
	{
	public:
		reportable_error(const std::string &callback);
	};
}

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

