#ifndef ZMQ_HELPERS_HPP
#define ZMQ_HELPERS_HPP

#include <string>
#include <vector>
#include <algorithm>

#include <boost/optional.hpp>

#include <zmq.hpp>

namespace bunsan{namespace worker{namespace pools{namespace helpers
{
	/// autoclosing socket
	class socket: public zmq::socket_t
	{
	public:
		socket(zmq::context_t &context, int type);
		~socket();
	};
	// generic
	template <typename T>
	void encode(const T &c, zmq::message_t &msg);
	template <typename T>
	void decode(/*const */zmq::message_t &msg, T &c);
	template <typename T>
	bool send(const T &c, zmq::socket_t &sock, int opt)
	{
		zmq::message_t msg;
		encode(c, msg);
		return sock.send(msg, opt);
	}
	template <typename T>
	bool recv(zmq::socket_t &sock, T &c, int opt)
	{
		zmq::message_t msg;
		if (sock.recv(&msg, opt))
		{
			decode(msg, c);
			return true;
		}
		else
			return false;
	}
	template <typename T, typename I>
	bool recv_more(zmq::socket_t &sock, T &c, I &more)
	{
		if (more)
		{
			zmq::message_t msg;
			sock.recv(&msg, ZMQ_RCVMORE);
			decode(msg, c);
			size_t more_size = sizeof(more);
			sock.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		}
		return more;
	}
	template <typename T, typename I>
	bool recv_more(zmq::socket_t &sock, boost::optional<T> &c, I &more)
	{
		if (more)
		{
			T c_;
			recv_more(sock, c_, more);
			c = c_;
		}
		return more;
	}
	// string
	template <>
	void encode(const std::string &c, zmq::message_t &msg);
	template <>
	void decode(/*const */zmq::message_t &msg, std::string &c);
	// strings
	template <>
	void encode(const std::vector<std::string> &c, zmq::message_t &msg);
	template <>
	void decode(/*const */zmq::message_t &msg, std::vector<std::string> &c);
	// binary
	template <>
	void encode(const std::vector<unsigned char> &c, zmq::message_t &msg);
	template <>
	void decode(/*const */zmq::message_t &msg, std::vector<unsigned char> &c);
}}}}

#endif //ZMQ_HELPERS_HPP

