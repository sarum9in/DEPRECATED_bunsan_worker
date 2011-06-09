#ifndef ZEROMQ_HPP
#define ZEROMQ_HPP

#include <memory>
#include <thread>
#include <atomic>
#include <vector>

#include "zmq.hpp"

#include "executor.hpp"
#include "hub.hpp"
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
		void queue_func();
		void worker_func();
		void check_running();
		void check_dirs();
		void do_task(const std::vector<std::string> &task);
		void add_to_hub();
		void register_worker();
		void hub_update();
		void unregister_worker();
		void remove_from_hub();
		std::shared_ptr<zmq::context_t> context;
		bunsan::dcs::hub_ptr hub;
		std::thread queue;
		std::vector<std::shared_ptr<std::thread>> workers;
		std::atomic<bool> to_stop;
		std::atomic<size_t> capacity;
		const unsigned iothreads;
		const unsigned worker_port;
		const unsigned queue_port;
		const unsigned long stop_check_interval;
		const std::string worker_tempdir;
		const boost::property_tree::ptree repository_config;
		const std::string uri;
		static bool factory_reg_hook;
	};
}}}

#endif //ZEROMQ_HPP

