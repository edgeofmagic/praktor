if(util_SOURCE_DIR)
    message(STATUS "util variables already defined. Skipping.")
else()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/util" EXCLUDE_FROM_ALL)
endif()

if(libuv_SOURCE_DIR)
    message(STATUS "libuv variables already defined. Skipping.")
else()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/libuv" EXCLUDE_FROM_ALL)
endif()