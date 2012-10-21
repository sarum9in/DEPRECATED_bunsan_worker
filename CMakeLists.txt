project(bunsan_worker)

cmake_minimum_required(VERSION 2.8)

if(${CMAKE_COMPILER_IS_GNUCXX})
	set(gxx_flags "-std=c++0x -Wall -Wextra")
	if(${UNIX})
		set(gxx_flags "-pthread ${gxx_flags}")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "-g ${gxx_flags}")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 ${gxx_flags}")
endif()

include(${CMAKE_SOURCE_DIR}/user-config.cmake OPTIONAL)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

if(NOT DEFINED ENABLE_TESTS)
	set(ENABLE_TESTS ON CACHE BOOL "Do you want to enable testing?" FORCE)
endif()

find_package(XMLRPC REQUIRED c++2 libwww-client abyss-server)

# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem serialization iostreams unit_test_framework program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(include ${XMLRPC_INCLUDE_DIRS})

aux_source_directory(src/lib lib_srcs)
aux_source_directory(src/lib/pools pools_srcs)
aux_source_directory(src/lib/workers workers_srcs)
aux_source_directory(src/lib/pool_interfaces pool_interfaces_srcs)
aux_source_directory(src/lib/callbacks callbacks_srcs)

add_library(${PROJECT_NAME} SHARED ${lib_srcs} ${pools_srcs} ${workers_srcs} ${pool_interfaces_srcs} ${callbacks_srcs})
target_link_libraries(${PROJECT_NAME} bunsan_common bunsan_dcs bunsan_pm ${Boost_LIBRARIES} ${XMLRPC_LIBRARIES} zmq)

add_executable(${PROJECT_NAME}_worker src/worker_main.cpp)
target_link_libraries(${PROJECT_NAME}_worker ${PROJECT_NAME})

add_executable(${PROJECT_NAME}_pool src/pool_main.cpp)
target_link_libraries(${PROJECT_NAME}_pool ${PROJECT_NAME})

# install

install(DIRECTORY include DESTINATION .)
install(TARGETS
	${PROJECT_NAME}
	${PROJECT_NAME}_worker
	${PROJECT_NAME}_pool
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
install(PROGRAMS src/worker.py DESTINATION bin)

if(${ENABLE_TESTS})
	enable_testing()
	#add_subdirectory(tests ${PROJECT_BINARY_DIR}/tests)
endif()

