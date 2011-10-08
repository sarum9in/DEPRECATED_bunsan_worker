#ifndef BUNSAN_WORKER_POOL_INTERFACE_HPP
#define BUNSAN_WORKER_POOL_INTERFACE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "bunsan/service.hpp"
#include "bunsan/factory.hpp"

#include "bunsan/worker/pool.hpp"

namespace bunsan{namespace worker
{
	class pool_interface: virtual public bunsan::service, private boost::noncopyable
	{
	public:
		// virtual class
		virtual bunsan::worker::pool_ptr pool()=0;
		virtual inline ~pool_interface(){}
		// factory
		typedef std::shared_ptr<pool_interface> pool_interface_ptr;
		static inline pool_interface_ptr instance(const std::string &type, const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool_)
		{
			return bunsan::factory::instance(factories, type, std::cref(config), std::ref(pool_));
		}
	protected:
		static inline bool register_new(const std::string &type, const std::function<pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)> f)
		{
			return bunsan::factory::register_new(factories, type, f);
		}
	private:
		static std::map<std::string, std::function<pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)>> *factories;
	};
	typedef pool_interface::pool_interface_ptr pool_interface_ptr;
}}

#endif //BUNSAN_WORKER_POOL_INTERFACE_HPP

