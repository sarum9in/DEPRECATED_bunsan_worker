#ifndef BUNSAN_WORKER_WORKER_HPP
#define BUNSAN_WORKER_WORKER_HPP

#include <memory>
#include <functional>
#include <string>
#include <map>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/noncopyable.hpp>

#include "bunsan/simple_service.hpp"
#include "bunsan/factory_helper.hpp"

namespace bunsan{namespace worker
{
    /*!
     * \brief class for executing tasks from pool
     */
    class worker: private boost::noncopyable
    BUNSAN_FACTORY_BEGIN(worker, const boost::property_tree::ptree &)
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
    BUNSAN_FACTORY_END(worker)
}}

#endif //BUNSAN_WORKER_WORKER_HPP

