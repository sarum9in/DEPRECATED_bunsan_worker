#ifndef WORKER_HPP
#define WORKER_HPP

#include <memory>
#include <functional>
#include <string>
#include <map>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "simple_service.hpp"
#include "factory.hpp"

namespace bunsan{namespace worker
{
	/*!
	 * \brief class for executing tasks from pool
	 */
	class worker: private boost::noncopyable
	{
	public:
		// virtual class
		virtual bool prepare()=0;
		/*!
		 * \brief process one call
		 */
		virtual void run_once()=0;
		/*!
		 * \brief process calls, should not return
		 */
		virtual inline void run()
		{
			while (true)
				run_once();
		}
		virtual inline ~worker(){}
		// factory
		typedef std::shared_ptr<worker> worker_ptr;
		static inline worker_ptr instance(const std::string &type, const boost::property_tree::ptree &config)
		{
			return bunsan::factory::instance(factories, type, config);
		}
	protected:
		static inline bool register_new(const std::string &type, const std::function<worker_ptr(const boost::property_tree::ptree &)> f)
		{
			return bunsan::factory::register_new(factories, type, f);
		}
	private:
		static std::map<std::string, std::function<worker_ptr(const boost::property_tree::ptree &)>> *factories;
	};
	typedef worker::worker_ptr worker_ptr;
}}

#endif //WORKER_HPP

