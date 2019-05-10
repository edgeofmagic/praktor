cmake_minimum_required(VERSION 3.0.0)
project(async VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_HOME_DIRECTORY = ${CMAKE_HOME_DIRECTORY}")

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#	else()
#		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")
	endif()

enable_testing()

include(scripts/doutil.cmake)

# add_subdirectory( ext/libuv EXCLUDE_FROM_ALL)

include_directories( 
	include 
	ext/boost 
#	ext/libuv/include 
#	ext/util/include
	${libuv_SOURCE_DIR}/include
	${util_SOURCE_DIR}/include
	ext/doctest/include)

set(ASYNC_SRCS
	src/async/loop_uv.cpp
	src/async/timer_uv.cpp
	src/async/tcp_uv.cpp
	src/async/udp_uv.cpp
	src/async/address.cpp
	src/async/error.cpp)

set(ASYNC_TEST_SRCS
	test/async/loop.cpp
	test/async/address.cpp
	test/async/endpoint.cpp
	test/async/tcp.cpp
	test/async/udp.cpp
 	test/async/event_flow.cpp
	test/test_main.cpp)

add_library(async ${ASYNC_SRCS})

add_executable(async_test ${ASYNC_TEST_SRCS})
target_link_libraries(async_test async uv_a util)

add_test(NAME async_test COMMAND async_test )
SET_TESTS_PROPERTIES(async_test
    PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(async_doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
