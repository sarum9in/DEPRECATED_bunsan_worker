#ifndef POOL_HPP
#define POOL_HPP

#include <memory>
#include <functional>
#include <string>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "simple_service.hpp"

namespace bunsan{namespace worker
{
	class pool: virtual public bunsan::simple_service, private boost::noncopyable
	{
	public:
		// virtual class
		virtual void add_task(const std::string &callback, const std::string &package, const std::vector<std::string> &args)=0;
		virtual inline ~pool(){}
		// factory
		typedef std::shared_ptr<pool> pool_ptr;
		static pool_ptr instance(const std::string &type, const boost::property_tree::ptree &config);
	protected:
		static void register_new(const std::string &type, const std::function<pool_ptr(const boost::property_tree::ptree &)> f);
	private:
		static std::shared_ptr<std::map<std::string, std::function<pool_ptr(const boost::property_tree::ptree &)>>> factory;
	};
	typedef pool::pool_ptr pool_ptr;
}}

#endif //POOL_HPP

