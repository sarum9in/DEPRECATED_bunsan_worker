#ifndef ZEROMQ_HPP
#define ZEROMQ_HPP

#include <memory>
#include <thread>
#include <atomic>
#include <vector>

#include <boost/optional.hpp>

#include "bunsan/utility/executor.hpp"
#include "bunsan/zmq.hpp"

#include "bunsan/dcs/hub.hpp"

#include "bunsan/worker/pool.hpp"
#include "bunsan/worker/callback.hpp"

namespace bunsan{namespace worker{namespace pools
{
    class zeromq: virtual public bunsan::worker::pool
    {
    public:
        zeromq(const boost::property_tree::ptree &config);
        virtual void add_task(const std::string &callback_type, const std::string &callback_uri, const std::vector<std::string> &callback_args, const std::string &package, const std::vector<std::string> &args, const boost::optional<std::vector<unsigned char>> &stdin_file);
        virtual void join();
        virtual void stop();
        virtual ~zeromq();
    private:
        void queue_func();
        void check_running();
        std::shared_ptr<zmq::context_t> context;
        std::thread queue;
        std::atomic<bool> to_stop;
        const unsigned iothreads;
        const int linger;
        const unsigned worker_port;
        const unsigned queue_port;
        const unsigned long stop_check_interval;
        static const bool factory_reg_hook;
    };
}}}

#endif //ZEROMQ_HPP

