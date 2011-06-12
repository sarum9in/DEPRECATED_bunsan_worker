#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <map>
#include <vector>
#include <functional>
#include <string>
#include <memory>

#include "factory.hpp"

namespace bunsan{namespace worker
{
	class callback
	{
	public:
		enum class status
		{
			received,
			registered
		};
		enum class action
		{
			nop,
			abort,
			bad
		};
		// virtual class
		/*!
		 * \brief call remote handler
		 *
		 * \param status_type represents type of message, may be used as enum: it is for program parse
		 * \param status_message human readable message
		 */
		virtual action call(status status_type, const std::string &status_message)=0;
		virtual action call(status status_type);
		// factory
		typedef std::shared_ptr<callback> callback_ptr;
		static inline callback_ptr instance(const std::string &type, const std::string &uri, const std::vector<std::string> &args)
		{
			return bunsan::factory::instance(factories, type, std::cref(uri), std::cref(args));
		}
	protected:
		static inline bool register_new(const std::string &type, const std::function<callback_ptr(const std::string &, const std::vector<std::string> &)> f)
		{
			return bunsan::factory::register_new(factories, type, f);
		}
	private:
		static std::map<std::string, std::function<callback_ptr(const std::string &, const std::vector<std::string> &)>> *factories;
	};
	typedef callback::callback_ptr callback_ptr;
}}

#endif //CALLBACK_HPP

