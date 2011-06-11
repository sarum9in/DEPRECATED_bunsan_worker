#include "pool_interface.hpp"

std::map<std::string, std::function<bunsan::worker::pool_interface_ptr(const boost::property_tree::ptree &, bunsan::worker::pool_ptr)>> *bunsan::worker::pool_interface::factories;

