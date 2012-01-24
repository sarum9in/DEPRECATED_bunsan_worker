#ifndef SRC_LIB_POOL_INTERFACES_XMLRPC_HPP
#define SRC_LIB_POOL_INTERFACES_XMLRPC_HPP

#include "bunsan/xmlrpc_service.hpp"

#include "bunsan/worker/pool.hpp"
#include "bunsan/worker/pool_interface.hpp"

namespace bunsan{namespace worker{namespace pool_interfaces
{
	class xmlrpc: virtual public bunsan::worker::pool_interface, virtual public bunsan::xmlrpc_service
	{
	public:
		xmlrpc(const boost::property_tree::ptree &config, pool_ptr pool__);
		virtual bunsan::worker::pool_ptr pool();
	protected:
		virtual void create_server();
		// factory
		static pool_interface_ptr instance(const boost::property_tree::ptree &config, pool_ptr pool__);
	private:
		bunsan::xmlrpc_service::registry_ptr registry;
		unsigned port;
		bunsan::worker::pool_ptr pool_;
		static bool factory_reg_hook;
	};
}}}

#endif //SRC_LIB_POOL_INTERFACES_XMLRPC_HPP

