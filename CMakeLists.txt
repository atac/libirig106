
cmake_minimum_required(VERSION 3.10)

project(libirig106
    VERSION 0.1.1
    LANGUAGES C CXX)

set(CMAKE_C_FLAGS "-ggdb -fpack-struct=1 -fPIC -std=c99 -Wall")

include_directories(include)
file(GLOB SOURCES "src/*.c")
add_library(libirig106 SHARED STATIC ${SOURCES})

install(TARGETS libirig106 DESTINATION ${PROJECT_SOURCE_DIR})

# add_subdirectory(tests)
