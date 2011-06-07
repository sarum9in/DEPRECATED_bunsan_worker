#include "pool_interface.hpp"

// factory
std::shared_ptr<std::map<std::string, std::function<bunsan::worker::pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)>>> bunsan::worker::pool_interface::factory;

void bunsan::worker::pool_interface::register_new(const std::string &type, const std::function<pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)> f)
{
	if (!factory)
		factory.reset(new std::map<std::string, std::function<pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)>>);
	if (factory->find(type)==factory->end())
		(*factory)[type] = f;
	else
		throw std::runtime_error("factory \""+type+"\" was already registered");
}

bunsan::worker::pool_interface_ptr bunsan::worker::pool_interface::instance(const std::string &type, const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool_)
{
	if (factory)
	{
		auto iter = factory->find(type);
		if (iter==factory->end())
		{
			return pool_interface_ptr();
		}
		else
		{
			return iter->second(config, pool_);
		}
	}
	else
	{
		return pool_interface_ptr();
	}
}

