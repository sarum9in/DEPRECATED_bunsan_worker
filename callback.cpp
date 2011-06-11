#include "callback.hpp"

std::map<std::string, std::function<bunsan::worker::callback_ptr(const std::string &, const std::vector<std::string> &)>> *bunsan::worker::callback::factories;

