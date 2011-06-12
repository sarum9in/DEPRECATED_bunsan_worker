#include "zmq_helpers.hpp"

#include <cstring>

bunsan::worker::pools::socket::socket(zmq::context_t &context, int type): zmq::socket_t(context, type){}

bunsan::worker::pools::socket::~socket()
{
	int zero = 0;
	setsockopt(ZMQ_LINGER, &zero, sizeof(zero));
};

// string
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

bool bunsan::worker::pools::send_string(const std::string &s, zmq::socket_t &sock, int opt)
{
	zmq::message_t msg;
	string2message(s, msg);
	return sock.send(msg, opt);
}

bool bunsan::worker::pools::recv_string(std::string &s, zmq::socket_t &sock, int opt)
{
	zmq::message_t msg;
	if (sock.recv(&msg, opt))
	{
		message2string(msg, s);
		return true;
	}
	else
		return false;
}

// strings
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

bool bunsan::worker::pools::send_strings(const std::vector<std::string> &s, zmq::socket_t &sock, int opt)
{
	zmq::message_t msg;
	strings2message(s, msg);
	return sock.send(msg, opt);
}

bool bunsan::worker::pools::recv_strings(std::vector<std::string> &s, zmq::socket_t &sock, int opt)
{
	zmq::message_t msg;
	if (sock.recv(&msg, opt))
	{
		message2strings(msg, s);
		return true;
	}
	else
		return false;
}

// binary
void bunsan::worker::pools::binary2message(const std::vector<unsigned char> &b, zmq::message_t &msg)
{
	msg.rebuild(b.size());
	memcpy(msg.data(), &b[0], b.size());
}

void bunsan::worker::pools::message2binary(/*const */zmq::message_t &msg, std::vector<unsigned char> &b)
{
	b.resize(msg.size());
	memcpy(&b[0], msg.data(), msg.size());
}

bool bunsan::worker::pools::send_binary(const std::vector<unsigned char> &b, zmq::socket_t &sock, int opt)
{
	zmq::message_t msg;
	binary2message(b, msg);
	return sock.send(msg, opt);
}

bool bunsan::worker::pools::recv_binary(std::vector<unsigned char> &b, zmq::socket_t &sock, int opt)
{
	zmq::message_t msg;
	if (sock.recv(&msg, opt))
	{
		message2binary(msg, b);
		return true;
	}
	else
		return false;
}

