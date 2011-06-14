#include <iostream>
#include <exception>

#include <csignal>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <boost/filesystem.hpp>

#include "util.hpp"
#include "worker.hpp"

volatile sig_atomic_t interrupted = false;

static void sighandler(int signum)
{
	interrupted = true;
}

static void signal_init()
{
	signal(SIGINT, &sighandler);
	signal(SIGTERM, &sighandler);
}

int main(int argc, char **argv)
{
	signal_init();
	std::string config_file;
	try
	{
		//command line parse
		boost::program_options::options_description desc(argv[0]);
		desc.add_options()
			("help,h", "Print this information")
			("version,V", "Program version")
			("config,c", boost::program_options::value<std::string>(&config_file)->default_value("config.rc"), "Configuration file");
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
		if (vm.count("help"))
		{
			std::cerr<<desc<<std::endl;
			return 1;
		}
		if (vm.count("version"))
		{
			std::cerr<<"It is too early to announce project version"<<std::endl;
			return 1;
		}
		//end parse
		//config parse
		DLOG(config parse);
		boost::property_tree::ptree config;
		bunsan::read_info(config_file, config);
		//end parse
		//worker object
		DLOG(creating worker);
		bunsan::worker::worker_ptr worker = bunsan::worker::worker::instance(config.get<std::string>("pool.type"), config.get_child("pool.config"));
		if (!worker)
			throw std::runtime_error("worker was not created");
		DLOG(starting infinite serve);
		while (!interrupted)
		{
			DLOG(waiting for a task);
			if (worker->prepare())
			{
				DLOG(task was found);
				worker->run_once();
				DLOG(task was completed);
			}
		}
		DLOG(program was completed);
	}
	catch(std::exception &e)
	{
		DLOG(Oops! An exception has occured);
		std::cerr<<e.what()<<std::endl;
		return 200;
	}
}

