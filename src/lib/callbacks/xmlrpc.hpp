#ifndef SRC_LIB_CALLBACKS_XMLRPC_HPP
#define SRC_LIB_CALLBACKS_XMLRPC_HPP

#include "bunsan/worker/callback.hpp"

namespace bunsan{namespace worker{namespace callbacks
{
    class xmlrpc: public bunsan::worker::callback
    {
    public:
        xmlrpc(const std::string &uri_, const std::string &method_, const std::vector<std::string> &args_);
        virtual action call(status status_type, const std::string &status_message);
    private:
        const std::string uri;
        const std::string method;
        const std::vector<std::string> args;
        static const bool factory_reg_hook;
    };
}}}

#endif //SRC_LIB_CALLBACKS_XMLRPC_HPP

