#include "callback.hpp"

std::map<std::string, std::function<bunsan::worker::callback_ptr(const std::string &, const std::vector<std::string> &)>> *bunsan::worker::callback::factories;

bunsan::worker::callback::action bunsan::worker::callback::call(status status_type)
{
	switch (status_type)
	{
	case status::received:
	case status::registered:
	default:
		return call(status_type, "");
	}
}

