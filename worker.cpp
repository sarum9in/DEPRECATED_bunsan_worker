#include "worker.hpp"

std::map<std::string, std::function<bunsan::worker::worker_ptr(const boost::property_tree::ptree &)>> *bunsan::worker::worker::factories;

