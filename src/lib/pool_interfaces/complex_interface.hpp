#ifndef SRC_LIB_POOL_INTERFACES_COMPLEX_INTERFACE_HPP
#define SRC_LIB_POOL_INTERFACES_COMPLEX_INTERFACE_HPP

#include <boost/property_tree/ptree.hpp>

#include "bunsan/complex_service.hpp"

#include "bunsan/worker/pool_interface.hpp"

namespace bunsan{namespace worker{namespace pool_interfaces
{
    class complex_interface: virtual public bunsan::worker::pool_interface, virtual public bunsan::complex_service
    {
    public:
        complex_interface(const boost::property_tree::ptree &config, bunsan::worker::pool_ptr pool__);
        virtual bunsan::worker::pool_ptr pool();
    private:
        bunsan::worker::pool_ptr pool_;
        static const bool factory_reg_hook;
    };
}}}

#endif //SRC_LIB_POOL_INTERFACES_COMPLEX_INTERFACE_HPP

