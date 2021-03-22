# cmake_minimum_required(VERSION 3.4.0)
cmake_minimum_required(VERSION 2.8.0)

project(byeredis) 
SET ( CMAKE_VERBOSE_MAKEFILE true )
set (CMAKE_EXPORT_COMPILE_COMMANDS YES) 
add_subdirectory(hiredis)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O2 -g") 

set(SRCS
    src/byeredis.cpp
    )

add_library(
    byeredis STATIC ${SRCS} 
    )


TARGET_INCLUDE_DIRECTORIES(byeredis PUBLIC 
    ${PROJECT_SOURCE_DIR}/src)

