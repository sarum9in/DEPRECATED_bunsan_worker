#include <iostream>
#include <exception>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <boost/filesystem.hpp>

#include "util.hpp"
#include "pool.hpp"
#include "pool_interface.hpp"

int main(int argc, char **argv)
{
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
		//pool object
		DLOG(creating pool);
		bunsan::worker::pool_ptr pool = bunsan::worker::pool::instance(config.get<std::string>("pool.type"), config.get_child("pool.config"));
		if (!pool)
			throw std::runtime_error("pool was not created");
		//DLOG(starting infinite serve);
		//DLOG(waiting: should not return);
		//pool->join();
		DLOG(creating pool interface);
		bunsan::worker::pool_interface_ptr iface = bunsan::worker::pool_interface::instance(config.get<std::string>("interface.type"), config.get_child("interface.config"), pool);
		if (!iface)
			throw std::runtime_error("pool interface was not created");
		//start interface in current thread
		DLOG(starting infinite serve);
		iface->start();
		DLOG(waiting: should not return);
		iface->join();// should not return*/
	}
	catch(std::exception &e)
	{
		DLOG(Oops! An exception has occured);
		std::cerr<<e.what()<<std::endl;
		return 200;
	}
}

