#include "xmlrpc.hpp"

#include <string>
#include <stdexcept>

#include <xmlrpc-c/server_abyss.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/girerr.hpp>

#include "util.hpp"

// factory

bool bunsan::worker::pool_interfaces::xmlrpc::factory_reg_hook = bunsan::worker::pool_interface::register_new("xmlrpc",
	[](const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool__)
	{
		bunsan::worker::pool_interface_ptr tmp(new bunsan::worker::pool_interfaces::xmlrpc(config, pool__));
		return tmp;
	});

// virtual class

class method_add_task: public xmlrpc_c::method2
{
public:
	method_add_task(bunsan::worker::pool_ptr pool__): pool_(pool__){}
	virtual void execute(const xmlrpc_c::paramList &param_list, const xmlrpc_c::callInfo *call_info, xmlrpc_c::value *result)
	{
		DLOG();
		try
		{
			std::string callback = param_list.getString(0);
			std::string package = param_list.getString(1);
			std::vector<xmlrpc_c::value> xargs = param_list.getArray(2);
			param_list.verifyEnd(3);
			std::vector<std::string> args(xargs.size());
			for (size_t i = 0; i<args.size(); ++i)
				args[i] = xmlrpc_c::value_string(xargs[i]);
			pool_->add_task(callback, package, args);
			*result = xmlrpc_c::value_nil();
		}
		catch(std::exception &e)
		{
			SLOG("fault: \""<<e.what()<<"\"");
			throw xmlrpc_c::fault(e.what());
		}
	}
private:
	bunsan::worker::pool_ptr pool_;
};

void bunsan::worker::pool_interfaces::xmlrpc::create_server()
{
	server.reset(new xmlrpc_c::serverAbyss(xmlrpc_c::serverAbyss::constrOpt().registryPtr(registry).portNumber(port)));
}

bunsan::worker::pool_interfaces::xmlrpc::xmlrpc(const boost::property_tree::ptree &config, pool_ptr pool__): pool_(pool__), port(config.get<unsigned>("server.port")), registry(new xmlrpc_c::registry)
{
	xmlrpc_c::methodPtr add_task(new method_add_task(pool_));
	registry->addMethod("add_task", add_task);
}

bunsan::worker::pool_ptr bunsan::worker::pool_interfaces::xmlrpc::xmlrpc::pool()
{
	return pool_;
}

