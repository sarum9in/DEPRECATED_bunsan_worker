project(bunsan_worker)

cmake_minimum_required(VERSION 2.8)

if (NOT DEFINED BUNSAN_COMMON_DIR)
	message(SEND_ERROR "you have to define BUNSAN_COMMON_DIR variable")
endif()

if (NOT DEFINED BUNSAN_DCS_DIR)
	message(SEND_ERROR "you have to define BUNSAN_DCS_DIR variable")
endif()

if (NOT DEFINED BUNSAN_PM_DIR)
	message(SEND_ERROR "you have to define BUNSAN_PM_DIR variable")
endif()

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
find_package(Boost COMPONENTS system filesystem serialization iostreams unit_test_framework program_options thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
#set(Boost_LIBRARIES boost_system boost_filesystem boost_serialization boost_iostreams boost_unit_test_framework boost_program_options)
#include_directories(/usr/local/include)
#link_directories(/usr/local/lib)
message("We have to check pthread library on win* platform")
#find_package (Threads)
#target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread)

add_definitions(-std=c++0x -pthread -g)

if (NOT TARGET bunsan_common)
	add_subdirectory(${BUNSAN_COMMON_DIR} ${PROJECT_BINARY_DIR}/common EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET bunsan_dcs)
	add_subdirectory(${BUNSAN_DCS_DIR} ${PROJECT_BINARY_DIR}/dcs EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET bunsan_pm)
	add_subdirectory(${BUNSAN_PM_DIR} ${PROJECT_BINARY_DIR}/pm EXCLUDE_FROM_ALL)
endif()

include_directories(. ${BUNSAN_COMMON_DIR} ${BUNSAN_DCS_DIR} ${BUNSAN_PM_DIR})

aux_source_directory(. srcs)
aux_source_directory(pools pools_srcs)
aux_source_directory(pool_interfaces pool_interfaces_srcs)

add_executable(${PROJECT_NAME} ${srcs} ${pools_srcs} ${pool_interfaces_srcs})
target_link_libraries(${PROJECT_NAME} bunsan_common bunsan_dcs bunsan_pm ${Boost_LIBRARIES} ${xmlrpc_link_lib} zmq)

