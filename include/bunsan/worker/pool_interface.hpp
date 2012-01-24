#ifndef BUNSAN_WORKER_POOL_INTERFACE_HPP
#define BUNSAN_WORKER_POOL_INTERFACE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "bunsan/service.hpp"
#include "bunsan/factory_helper.hpp"

#include "bunsan/worker/pool.hpp"

namespace bunsan{namespace worker
{
	class pool_interface: virtual public bunsan::service, private boost::noncopyable
	BUNSAN_FACTORY_BEGIN(pool_interface, const boost::property_tree::ptree &, const bunsan::worker::pool_ptr &)
	public:
		// virtual class
		virtual bunsan::worker::pool_ptr pool()=0;
		virtual inline ~pool_interface(){}
	BUNSAN_FACTORY_END(pool_interface)
}}

#endif //BUNSAN_WORKER_POOL_INTERFACE_HPP

