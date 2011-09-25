project(bunsan_worker)

cmake_minimum_required(VERSION 2.8)

find_package(XMLRPC REQUIRED c++2 libwww-client abyss-server)

# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem serialization iostreams unit_test_framework program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

message(AUTHOR_WARNING "We have to check pthread library on win* platform")
message(AUTHOR_WARNING "We have to provide support for different compilers")

add_definitions(-std=c++0x -pthread -g -D_GLIBCXX_USE_NANOSLEEP)

include_directories(include ${XMLRPC_INCLUDE_DIRS})

aux_source_directory(src/lib lib_srcs)
aux_source_directory(src/lib/pools pools_srcs)
aux_source_directory(src/lib/workers workers_srcs)
aux_source_directory(src/lib/pool_interfaces pool_interfaces_srcs)
aux_source_directory(src/lib/callbacks callbacks_srcs)

add_library(${PROJECT_NAME} SHARED ${lib_srcs} ${pools_srcs} ${workers_srcs} ${pool_interfaces_srcs} ${callbacks_srcs})
target_link_libraries(${PROJECT_NAME} bunsan_common bunsan_dcs bunsan_pm ${Boost_LIBRARIES} ${XMLRPC_LIBRARIES} zmq)

add_executable(worker src/worker_main.cpp)
target_link_libraries(worker ${PROJECT_NAME})

add_executable(pool src/pool_main.cpp)
target_link_libraries(pool ${PROJECT_NAME})

# install

install(DIRECTORY include DESTINATION .)
install(TARGETS ${PROJECT_NAME} worker pool
                          RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib)
install(PROGRAMS src/worker.py DESTINATION bin)

