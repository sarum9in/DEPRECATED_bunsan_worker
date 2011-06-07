#include "pool.hpp"

// factory
std::shared_ptr<std::map<std::string, std::function<bunsan::worker::pool_ptr(const boost::property_tree::ptree &)>>> bunsan::worker::pool::factory;

void bunsan::worker::pool::register_new(const std::string &type, const std::function<pool_ptr(const boost::property_tree::ptree &)> f)
{
	if (!factory)
		factory.reset(new std::map<std::string, std::function<pool_ptr(const boost::property_tree::ptree &)>>);
	if (factory->find(type)==factory->end())
		(*factory)[type] = f;
	else
		throw std::runtime_error("factory \""+type+"\" was already registered");
}

bunsan::worker::pool_ptr bunsan::worker::pool::instance(const std::string &type, const boost::property_tree::ptree &config)
{
	if (factory)
	{
		auto iter = factory->find(type);
		if (iter==factory->end())
		{
			return pool_ptr();
		}
		else
		{
			return iter->second(config);
		}
	}
	else
	{
		return pool_ptr();
	}
}

