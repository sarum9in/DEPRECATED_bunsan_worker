#ifndef POOL_INTERFACE_HPP
#define POOL_INTERFACE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "service.hpp"

#include "pool.hpp"

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
		static pool_interface_ptr instance(const std::string &type, const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool_);
	protected:
		static bool register_new(const std::string &type, const std::function<pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)> f);
	private:
		static std::shared_ptr<std::map<std::string, std::function<pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)>>> factory;
	};
	typedef pool_interface::pool_interface_ptr pool_interface_ptr;
}}

#endif //POOL_INTERFACE_HPP

