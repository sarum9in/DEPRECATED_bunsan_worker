#ifndef ZEROMQ_HPP
#define ZEROMQ_HPP

#include <memory>

#include <zmq.hpp>

#include "bunsan/zmq.hpp"

#include "bunsan/dcs/hub.hpp"

#include "bunsan/worker/worker.hpp"
#include "bunsan/worker/callback.hpp"

namespace bunsan{namespace worker{namespace workers
{
	class zeromq: public bunsan::worker::worker
	{
	public:
		zeromq(const boost::property_tree::ptree &config);
		virtual bool prepare();
		virtual void run_once();
		virtual ~zeromq();
	private:
		void do_task(const std::string &callback_type, const std::string &callback_uri, const std::vector<std::string> &callback_args, const std::string &package, const std::vector<std::string> &args, const boost::optional<std::vector<unsigned char>> &stdin_file);
		zmq::context_t context;
		std::unique_ptr<zmq::socket_t> req;
		bunsan::dcs::hub_ptr hub;
		bool have_task;
		bool wait_task;
		const std::string worker_tempdir;
		const boost::property_tree::ptree repository_config;
		const unsigned stop_check_interval;
		static const bool factory_reg_hook;
	};
}}}

#endif //ZEROMQ_HPP

