project(bunsan_worker)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(BunsanCommon)

bunsan_use_xmlrpc(c++2 libwww-client abyss-server)
bunsan_use_bunsan(common process network pm dcs)
bunsan_use_boost(system filesystem serialization program_options thread)

bunsan_add_shared_library(${PROJECT_NAME}
    src/lib/callback.cpp
    src/lib/pool.cpp
    src/lib/pool_interface.cpp
    src/lib/worker.cpp

    src/lib/callbacks/xmlrpc.cpp
    src/lib/callbacks/xmlrpc.hpp

    src/lib/pools/zeromq.cpp
    src/lib/pools/zeromq.hpp

    src/lib/pool_interfaces/complex_interface.hpp
    src/lib/pool_interfaces/xmlrpc.cpp
    src/lib/pool_interfaces/xmlrpc.hpp
    src/lib/pool_interfaces/complex_interface.cpp

    src/lib/workers/zeromq.cpp
    src/lib/workers/zeromq.hpp

)
bunsan_use(${PROJECT_NAME})

bunsan_add_executable(${PROJECT_NAME}_worker src/worker_main.cpp)
bunsan_add_executable(${PROJECT_NAME}_pool src/pool_main.cpp)

bunsan_install_headers()
bunsan_install_targets(${PROJECT_NAME} ${PROJECT_NAME}_worker ${PROJECT_NAME}_pool)

bunsan_include_tests()
