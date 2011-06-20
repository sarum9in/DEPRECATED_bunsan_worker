#include "zeromq.hpp"

#include <boost/process.hpp>

#include "tempfile.hpp"
#include "repository.hpp"

bool bunsan::worker::workers::zeromq::factory_reg_hook = bunsan::worker::worker::register_new("zeromq",
	[](const boost::property_tree::ptree &config)
	{
		bunsan::worker::worker_ptr tmp(new bunsan::worker::workers::zeromq(config));
		return tmp;
	});

bunsan::worker::workers::zeromq::zeromq(const boost::property_tree::ptree &config):
	have_task(false),
	wait_task(false),
	context(config.get<unsigned>("iothreads")),
	stop_check_interval(config.get<unsigned long>("stop_check_interval")),
	repository_config(config.get_child("repository")),
	worker_tempdir(config.get<std::string>("worker.tmp"))
{
	hub = bunsan::dcs::hub::instance(config.get<std::string>("hub.type"), config.get_child("hub.config"));
	if (!hub)
		throw std::runtime_error("hub was not created");
	hub->start();
	req.reset(new bunsan::zmq_helpers::socket(context, ZMQ_REQ));
	req->connect(("tcp://localhost:"+config.get<std::string>("worker.port")).c_str());
}

bool bunsan::worker::workers::zeromq::prepare()
{
	namespace zh = bunsan::zmq_helpers;
	if (have_task)
		return true;
	if (!wait_task)
	{
		zh::send("", *req);
		wait_task = true;
	}
	zmq::pollitem_t items[] =
	{
		{*req, 0, ZMQ_POLLIN, 0}
	};
	zmq::poll(items, 1, stop_check_interval);
	return have_task = items[0].revents & ZMQ_POLLIN;
}

void bunsan::worker::workers::zeromq::run_once()
{
	while (!prepare());
	namespace zh = bunsan::zmq_helpers;
	std::string callback_type, callback_uri;
	std::vector<std::string> callback_args;
	std::string package;
	std::vector<std::string> args;
	boost::optional<std::vector<unsigned char>> stdin_file;
	int more = 1;
	DLOG(receiving a task);
	zh::recv_more(*req, callback_type, more);
	zh::recv_more(*req, callback_uri, more);
	zh::recv_more(*req, callback_args, more);
	zh::recv_more(*req, package, more);
	zh::recv_more(*req, args, more);
#warning error check if (!more)
	zh::recv_more(*req, stdin_file, more);
	have_task = wait_task = false;
	DLOG(task was received);
	do_task(callback_type, callback_uri, callback_args, package, args, stdin_file);
}

void bunsan::worker::workers::zeromq::do_task(const std::string &callback_type, const std::string &callback_uri, const std::vector<std::string> &callback_args, const std::string &package, const std::vector<std::string> &args, const boost::optional<std::vector<unsigned char>> &stdin_file)
{
	bunsan::worker::callback_ptr cb = bunsan::worker::callback::instance(callback_type, callback_uri, callback_args);
	try
	{
		if (bunsan::worker::callback::action::abort==bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::preparing))
		{
			bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::aborted);
			return;
		}
		DLOG(preparing package manager);
		if (bunsan::worker::callback::action::abort==bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::extracting))
		{
			bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::aborted);
			return;
		}
		boost::property_tree::ptree repo_config = repository_config.get_child("pm");
		boost::optional<std::string> uri_substitution = repository_config.get_optional<std::string>("uri_substitution");
		if (uri_substitution)
		{
			std::string repo_uri = hub->select_resource(repo_config.get<std::string>("resource_name"));
			repo_config.put(uri_substitution.get(), repo_uri);
		}
		DLOG(creating repository);
		bunsan::pm::repository repo(repo_config);
		bunsan::tempfile_ptr tmpdir = bunsan::tempfile::in_dir(worker_tempdir);
		bunsan::reset_dir(tmpdir->path());
		repo.extract(package, tmpdir->path());
		if (bunsan::worker::callback::action::abort==bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::preparing_executing))
		{
			bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::aborted);
			return;
		}
		DLOG(preparing async execute);
		if (args.empty())
			throw std::runtime_error("Nothing to execute!");
		SLOG("exec in "<<tmpdir->path()<<" file "<<tmpdir->path()/args.at(0));
		//DLOG(starting child wait loop);
#warning bad implementation, workaround only, TODO
		boost::process::context ctx;
		ctx.work_directory = tmpdir->path().native();
		if (stdin_file)
			ctx.stdin_behavior = boost::process::capture_stream();
		ctx.stdout_behavior = boost::process::inherit_stream();
		ctx.stderr_behavior = boost::process::inherit_stream();
		ctx.environment = boost::process::self::get_environment();
		boost::process::child child = boost::process::launch((tmpdir->path()/args.at(0)).native(), args, ctx);
		if (stdin_file)
		{
			boost::process::postream &out = child.get_stdin();
			for (unsigned char c: stdin_file.get())
			{
				if (out.bad())
					break;
				out.put(c);
			}
			out.close();
		}
		boost::process::status status = child.wait();
		if (status.exited())
		{
			if (status.exit_status()==0)
			{
				(void) bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::completed);
				DLOG(completed);
			}
			else
			{
				(void) bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::non_zero_exit_status,
					"process return code is not null: \""+boost::lexical_cast<std::string>(status.exit_status())+"\"");
				SLOG("process return code is not null: \""<<status.exit_status()<<"\"");
			}
		}
		else
		{
			(void) bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::not_exited);
			DLOG(not exited);
		}
	}
	/*catch (interrupted_error &e)
	{
		bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::server_terminated);
	}*/
	catch (std::exception &e)
	{
		SLOG("Oops! \""<<e.what()<<"\"");
		bunsan::worker::callback::inform(cb, bunsan::worker::callback::status::error, e.what());
	}
}

bunsan::worker::workers::zeromq::~zeromq()
{
}

