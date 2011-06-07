#ifndef XMLRPC_HPP
#define XMLRPC_HPP

#include "pool.hpp"
#include "pool_interface.hpp"
#include "runner.hpp"
#include "xmlrpc_service.hpp"

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
		// factory
		static bunsan::runner reg;
	};
}}}

#endif //XMLRPC_HPP

