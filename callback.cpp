#include "callback.hpp"

#include "util.hpp"

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
	PROC(not_exited)
	PROC(non_zero_exit_status)
	PROC(aborted)
	PROC(error)
	PROC(server_terminated)
	PROC(bad)
	default:
		return call(status_type, "");
	}
#undef PROC
}

bunsan::worker::callback::action bunsan::worker::callback::inform(bunsan::worker::callback_ptr cb, bunsan::worker::callback::status st)
{
	if (cb)
	{
		DLOG(informing callback);
		return cb->call(st);
	}
	else
	{
		DLOG(bad callback);
		return bunsan::worker::callback::action::bad;
	}
}

bunsan::worker::callback::action bunsan::worker::callback::inform(bunsan::worker::callback_ptr cb, bunsan::worker::callback::status st, std::string msg)
{
	if (cb)
	{
		DLOG(informing callback);
		return cb->call(st, msg);
	}
	else
	{
		DLOG(bad callback);
		return bunsan::worker::callback::action::bad;
	}
}

