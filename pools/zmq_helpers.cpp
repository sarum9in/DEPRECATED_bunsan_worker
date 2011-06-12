#include "zmq_helpers.hpp"

#include <cstring>

bunsan::worker::pools::helpers::socket::socket(zmq::context_t &context, int type): zmq::socket_t(context, type){}

bunsan::worker::pools::helpers::socket::~socket()
{
	int zero = 0;
	setsockopt(ZMQ_LINGER, &zero, sizeof(zero));
};

// string
template <>
void bunsan::worker::pools::helpers::encode(const std::string &c, zmq::message_t &msg)
{
	msg.rebuild(c.size());
	memcpy(msg.data(), c.c_str(), c.size());
}

template <>
void bunsan::worker::pools::helpers::decode(/*const */zmq::message_t &msg, std::string &c)
{
	c.resize(msg.size());
	std::copy_n(static_cast<char *>(msg.data()), msg.size(), c.begin());
}

// strings
template <>
void bunsan::worker::pools::helpers::encode(const std::vector<std::string> &c, zmq::message_t &msg)
{
	size_t size = 0;
	for (const std::string &i: c)
		size += i.size()+1;
	msg.rebuild(size);
	char *buf = static_cast<char *>(msg.data());
	for (const std::string &i: c)
	{
		memcpy(buf, i.c_str(), i.size());
		buf += i.size();
		*buf = '\0';
		++buf;
	}
}

template <>
void bunsan::worker::pools::helpers::decode(/*const */zmq::message_t &msg, std::vector<std::string> &c)
{
	char *buf = static_cast<char *>(msg.data()), *ebuf = static_cast<char *>(msg.data())+msg.size();
	size_t size = std::count(buf, ebuf, '\0');
	c.resize(size);
	for (size_t i = 0; i<size; ++i)
	{
		char *nbuf = std::find(buf, ebuf, '\0');
		c[i].resize(nbuf-buf);
		std::copy(buf, nbuf, c[i].begin());
		buf = nbuf+1;
	}
}

// binary
template <>
void bunsan::worker::pools::helpers::encode(const std::vector<unsigned char> &c, zmq::message_t &msg)
{
	msg.rebuild(c.size());
	memcpy(msg.data(), &c[0], c.size());
}

template <>
void bunsan::worker::pools::helpers::decode(/*const */zmq::message_t &msg, std::vector<unsigned char> &c)
{
	c.resize(msg.size());
	memcpy(&c[0], msg.data(), msg.size());
}

