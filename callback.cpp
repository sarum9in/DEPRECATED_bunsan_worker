#include "callback.hpp"

std::map<std::string, std::function<bunsan::worker::callback_ptr(const std::string &, const std::vector<std::string> &)>> *bunsan::worker::callback::factories;

bunsan::worker::callback::action bunsan::worker::callback::call(status status_type)
{
#define PROC(X) case status:: X: return call(status_type, #X);
	switch (status_type)
	{
	PROC(received)
	PROC(registered)
	PROC(preparing)
	PROC(extracting)
	PROC(preparing_executing)
	PROC(executing)
	PROC(completed)
	PROC(not_zero_code)
	PROC(aborted)
	PROC(error)
	PROC(server_terminated)
	PROC(bad)
	default:
		return call(status_type, "");
	}
#undef PROC
}

