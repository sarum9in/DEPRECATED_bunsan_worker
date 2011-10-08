#ifndef BUNSAN_WORKER_POOL_HPP
#define BUNSAN_WORKER_POOL_HPP

#include <memory>
#include <functional>
#include <string>
#include <map>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "bunsan/simple_service.hpp"
#include "bunsan/factory.hpp"

namespace bunsan{namespace worker
{
	class pool: virtual public bunsan::simple_service, private boost::noncopyable
	{
	public:
		// virtual class
		/*!
		 * \brief add new task
		 *
		 * \param callback_type callback name that was used to register it in factory
		 * \param callback_uri address that will be used to send data
		 * \param callback_args arguments for callback, see callback specification
		 * \param package package name that will be used for specified task
		 * \param args this will be argc/argv of executable,
		 *	where argv[0] is path to executable relative to temporary package dir
		 * \param stdin_file data that will be stdin filler
		 *
		 * program will have temporary package extracton dir as current path (working directory)
		 *
		 */
		virtual void add_task(
			const std::string &callback_type,
			const std::string &callback_uri,
			const std::vector<std::string> &callback_args,
			const std::string &package,
			const std::vector<std::string> &args,
			const boost::optional<std::vector<unsigned char>> &stdin_file)=0;
		virtual inline ~pool(){}
		// factory
		typedef std::shared_ptr<pool> pool_ptr;
		static inline pool_ptr instance(const std::string &type, const boost::property_tree::ptree &config)
		{
			return bunsan::factory::instance(factories, type, config);
		}
	protected:
		static inline bool register_new(const std::string &type, const std::function<pool_ptr(const boost::property_tree::ptree &)> f)
		{
			return bunsan::factory::register_new(factories, type, f);
		}
	private:
		static std::map<std::string, std::function<pool_ptr(const boost::property_tree::ptree &)>> *factories;
	};
	typedef pool::pool_ptr pool_ptr;
}}

#endif //BUNSAN_WORKER_POOL_HPP

