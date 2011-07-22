project(bunsan_worker)

cmake_minimum_required(VERSION 2.8)

set(
	xmlrpc_link_lib
	xmlrpc++
	xmlrpc_client++
	xmlrpc_cpp
	xmlrpc_server++
	xmlrpc_server_abyss++
	xmlrpc_server_cgi++
	xmlrpc_server_pstream++
	xmlrpc_util
)

# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem serialization iostreams unit_test_framework program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
#set(Boost_LIBRARIES boost_system boost_filesystem boost_serialization boost_iostreams boost_unit_test_framework boost_program_options)
#include_directories(/usr/local/include)
#link_directories(/usr/local/lib)
message("We have to check pthread library on win* platform")
#find_package(Threads)
#target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread)

add_definitions(-std=c++0x -pthread -g -D_GLIBCXX_USE_NANOSLEEP)

include_directories(include)

aux_source_directory(src/lib lib_srcs)
aux_source_directory(src/lib/pools pools_srcs)
aux_source_directory(src/lib/workers workers_srcs)
aux_source_directory(src/lib/pool_interfaces pool_interfaces_srcs)
aux_source_directory(src/lib/callbacks callbacks_srcs)

add_library(${PROJECT_NAME} SHARED ${lib_srcs})
target_link_libraries(${PROJECT_NAME} bunsan_common bunsan_dcs bunsan_pm ${Boost_LIBRARIES} ${xmlrpc_link_lib} zmq)

add_executable(worker src/worker_main.cpp)
target_link_libraries(worker ${PROJECT_NAME})

add_executable(pool src/pool_main.cpp)
target_link_libraries(pool ${PROJECT_NAME})

# install

install(DIRECTORY include DESTINATION .)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(PROGRAMS worker.py DESTINATION bin)

