#include "xmlrpc.hpp"

#include <xmlrpc-c/client_simple.hpp>

const bool bunsan::worker::callbacks::xmlrpc::factory_reg_hook = bunsan::worker::callback::register_new("xmlrpc",
	[](const std::string &uri, const std::vector<std::string> &args)
	{
		bunsan::worker::callback_ptr tmp(new bunsan::worker::callbacks::xmlrpc(uri, args.at(0), std::vector<std::string>(args.begin()+1, args.end())));// TODO bad error information
		return tmp;
	});

bunsan::worker::callbacks::xmlrpc::xmlrpc(const std::string &uri_, const std::string &method_, const std::vector<std::string> &args_): uri(uri_), method(method_), args(args_){}

bunsan::worker::callback::action bunsan::worker::callbacks::xmlrpc::call(status status_type, const std::string &status_message)
{
	xmlrpc_c::clientSimple proxy;
	xmlrpc_c::value result;
	xmlrpc_c::paramList argv;
	for (const std::string &s: args)
		argv.addc(s);
	argv.addc(static_cast<int>(status_type)).addc(status_message);
	proxy.call(uri, method, argv, &result);
	return static_cast<bunsan::worker::callback::action>(xmlrpc_c::value_int(result).cvalue());
}

