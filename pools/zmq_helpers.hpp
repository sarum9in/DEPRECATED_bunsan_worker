#ifndef ZMQ_HELPERS_HPP
#define ZMQ_HELPERS_HPP

#include <string>
#include <vector>
#include <algorithm>

#include <zmq.hpp>

namespace bunsan{namespace worker{namespace pools
{
	/// autoclosing socket
	class socket: public zmq::socket_t
	{
	public:
		socket(zmq::context_t &context, int type);
		~socket();
	};
	void string2message(const std::string &s, zmq::message_t &msg);
	void message2string(/*const */zmq::message_t &msg, std::string &s);
	void strings2message(const std::vector<std::string> &s, zmq::message_t &msg);
	void message2strings(/*const */zmq::message_t &msg, std::vector<std::string> &s);
}}}

#endif //ZMQ_HELPERS_HPP

