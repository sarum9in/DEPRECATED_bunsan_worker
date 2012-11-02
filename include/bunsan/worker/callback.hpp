#ifndef BUNSAN_WORKER_CALLBACK_HPP
#define BUNSAN_WORKER_CALLBACK_HPP

#include <map>
#include <vector>
#include <functional>
#include <string>
#include <memory>

#include "bunsan/factory_helper.hpp"

namespace bunsan{namespace worker
{
    class callback
    BUNSAN_FACTORY_BEGIN(callback, const std::string &, const std::vector<std::string> &)
    public:
        enum class status
        {
            received,
            registered,
            preparing,
            extracting,
            preparing_execution,
            executing,
            completed,
            not_exited,
            non_zero_exit_status,
            aborted,
            error,
            server_terminated
        };
        enum class action
        {
            nop,            ///< no operation, execution should continue
            abort,          ///< execution should be aborted
            bad         ///< used when callback proxy can't connect to the server or callback was not initialized
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
        /// send status using callback with default message for this status
        static bunsan::worker::callback::action inform(callback_ptr cb, status st);
        /// send status and message using callback
        static bunsan::worker::callback::action inform(callback_ptr cb, status st, std::string msg);
    BUNSAN_FACTORY_END(callback)
}}

#endif //BUNSAN_WORKER_CALLBACK_HPP

