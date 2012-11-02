#include "complex_interface.hpp"

const bool bunsan::worker::pool_interfaces::complex_interface::factory_reg_hook = bunsan::worker::pool_interface::register_new("complex_interface",
    [](const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool_)
    {
        bunsan::worker::pool_interface_ptr tmp(new bunsan::worker::pool_interfaces::complex_interface(config, pool_));
        return tmp;
    });

bunsan::worker::pool_interfaces::complex_interface::complex_interface(const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool__): pool_(pool__)
{
    services.clear();
    for (const auto &c: config)
    {
        bunsan::service_ptr stmp = bunsan::worker::pool_interface::instance
                (
                    c.second.get<std::string>("type"),
                    c.second.get_child("config"),
                    pool_
                );
        if (!stmp)
            throw std::runtime_error("unable to create pool_interface instance of \""+c.second.get<std::string>("type")+"\"");
        services.push_back(stmp);
    }
}

bunsan::worker::pool_ptr bunsan::worker::pool_interfaces::complex_interface::pool()
{
    return pool_;
}

