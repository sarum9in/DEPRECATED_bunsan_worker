#include "xmlrpc.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <xmlrpc-c/server_abyss.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/girerr.hpp>

#include "bunsan/util.hpp"

// factory

bool bunsan::worker::pool_interfaces::xmlrpc::factory_reg_hook = bunsan::worker::pool_interface::register_new("xmlrpc",
	[](const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool__)
	{
		bunsan::worker::pool_interface_ptr tmp(new bunsan::worker::pool_interfaces::xmlrpc(config, pool__));
		return tmp;
	});

// virtual class

namespace
{
	class method_add_task: public xmlrpc_c::method2
	{
	public:
		method_add_task(bunsan::worker::pool_ptr pool__): pool_(pool__){}
		virtual void execute(const xmlrpc_c::paramList &param_list, const xmlrpc_c::callInfo *call_info, xmlrpc_c::value *result)
		{
			DLOG();
			try
			{
				// 0
				std::string callback_type = param_list.getString(0);
				// 1
				std::string callback_uri = param_list.getString(1);
				// 2
				std::vector<xmlrpc_c::value> callback_args_ = param_list.getArray(2);
				std::vector<std::string> callback_args(callback_args_.size());
				std::transform(callback_args_.begin(), callback_args_.end(), callback_args.begin(), [](const xmlrpc_c::value &x){return xmlrpc_c::value_string(x).cvalue();});
				// 3
				std::string package = param_list.getString(3);
				// 4
				std::vector<xmlrpc_c::value> args_ = param_list.getArray(4);
				std::vector<std::string> args(args_.size());
				std::transform(args_.begin(), args_.end(), args.begin(), [](const xmlrpc_c::value &x){return xmlrpc_c::value_string(x).cvalue();});
				// 5
				boost::optional<std::vector<unsigned char>> stdin_file;
				if (param_list.size()>5)
				{
					stdin_file = param_list.getBytestring(5);
					// 6
					param_list.verifyEnd(6);
				}
				else
					param_list.verifyEnd(5);
				pool_->add_task(callback_type, callback_uri, callback_args, package, args, stdin_file);
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
}

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

