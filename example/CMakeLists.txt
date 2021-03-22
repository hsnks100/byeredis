# cmake_minimum_required(VERSION 3.4.0)
cmake_minimum_required(VERSION 2.8.0)

project(ksoo_example)


SET ( CMAKE_VERBOSE_MAKEFILE true )
set (CMAKE_EXPORT_COMPILE_COMMANDS YES) 
add_subdirectory(../ build)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O2 -g") 
# add_subdirectory(fmt) 

set(SRCS
    main.cpp	    
    )

add_definitions( 
)

link_directories(
    )
# Tell CMake to create the helloworld executable
add_executable(${PROJECT_NAME} ${SRCS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(${PROJECT_NAME}
    pthread
    hiredis_static
    rt
    curl
    byeredis
    )

