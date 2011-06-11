#include "zmq_helpers.hpp"

#include <cstring>

bunsan::worker::pools::socket::socket(zmq::context_t &context, int type): zmq::socket_t(context, type){}

bunsan::worker::pools::socket::~socket()
{
	int zero = 0;
	setsockopt(ZMQ_LINGER, &zero, sizeof(zero));
};

void bunsan::worker::pools::string2message(const std::string &s, zmq::message_t &msg)
{
	msg.rebuild(s.size());
	memcpy(msg.data(), s.c_str(), s.size());
}

void bunsan::worker::pools::message2string(/*const */zmq::message_t &msg, std::string &s)
{
	s.resize(msg.size());
	std::copy_n(static_cast<char *>(msg.data()), msg.size(), s.begin());
}

void bunsan::worker::pools::strings2message(const std::vector<std::string> &s, zmq::message_t &msg)
{
	size_t size = 0;
	for (const std::string &i: s)
		size += s.size()+1;
	msg.rebuild(size);
	char *buf = static_cast<char *>(msg.data());
	for (const std::string &i: s)
	{
		memcpy(buf, i.c_str(), i.size());
		buf += i.size();
		*buf = '\0';
		++buf;
	}
}

void bunsan::worker::pools::message2strings(/*const */zmq::message_t &msg, std::vector<std::string> &s)
{
	char *buf = static_cast<char *>(msg.data()), *ebuf = static_cast<char *>(msg.data())+msg.size();
	size_t size = std::count(buf, ebuf, '\0');
	s.resize(size);
	for (size_t i = 0; i<size; ++i)
	{
		char *nbuf = std::find(buf, ebuf, '\0');
		s[i].resize(nbuf-buf);
		std::copy(buf, nbuf, s[i].begin());
		buf = nbuf+1;
	}
}

