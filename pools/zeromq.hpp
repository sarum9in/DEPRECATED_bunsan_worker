#ifndef ZEROMQ_HPP
#define ZEROMQ_HPP

#include <memory>
#include <thread>
#include <atomic>
#include <vector>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "zmq.hpp"

#include "executor.hpp"
#include "hub.hpp"
#include "runner.hpp"
#include "pool.hpp"

namespace bunsan{namespace worker{namespace pools
{
	class zeromq: virtual public bunsan::worker::pool
	{
	public:
		zeromq(const boost::property_tree::ptree &config);
		virtual void add_task(const std::string &callback, const std::string &package, const std::vector<std::string> &args);
		virtual void join();
		virtual void stop();
		virtual ~zeromq();
	private:
		//typedef boost::unique_lock<boost::shared_mutex> unique_guard;
		//typedef boost::shared_lock<boost::shared_mutex> shared_guard;
		void queue_func();
		void worker_func();
		void check_running();
		void check_dirs();
		std::shared_ptr<zmq::context_t> context;
		bunsan::dcs::hub_ptr hub;
		std::thread queue;
		std::vector<std::shared_ptr<std::thread>> workers;
		std::atomic_bool to_stop;
		const unsigned iothreads;
		const unsigned worker_port;
		const unsigned queue_port;
		const unsigned long stop_check_interval;
		const std::string worker_tempdir;
		const boost::property_tree::ptree repository_config;
		static bunsan::runner reg;
	};
}}}

#endif //ZEROMQ_HPP

