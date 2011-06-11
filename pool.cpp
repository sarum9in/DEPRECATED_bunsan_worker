#include "pool.hpp"

std::map<std::string, std::function<bunsan::worker::pool_ptr(const boost::property_tree::ptree &)>> *bunsan::worker::pool::factories;

