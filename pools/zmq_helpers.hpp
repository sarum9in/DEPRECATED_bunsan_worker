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
	// string
	void string2message(const std::string &s, zmq::message_t &msg);
	void message2string(/*const */zmq::message_t &msg, std::string &s);
	bool send_string(const std::string &s, zmq::socket_t &sock, int opt);
	bool recv_string(std::string &s, zmq::socket_t &sock, int opt);
	// strings
	void strings2message(const std::vector<std::string> &s, zmq::message_t &msg);
	void message2strings(/*const */zmq::message_t &msg, std::vector<std::string> &s);
	bool send_strings(const std::vector<std::string> &s, zmq::socket_t &sock, int opt);
	bool recv_strings(std::vector<std::string> &s, zmq::socket_t &sock, int opt);
	// binary
	void binary2message(const std::vector<unsigned char> &b, zmq::message_t &msg);
	void message2binary(/*const */zmq::message_t &msg, std::vector<unsigned char> &b);
	bool send_binary(const std::vector<unsigned char> &s, zmq::socket_t &sock, int opt);
	bool recv_binary(std::vector<unsigned char> &s, zmq::socket_t &sock, int opt);
}}}

#endif //ZMQ_HELPERS_HPP

