
cmake_minimum_required(VERSION 3.12)

project(libirig106
    VERSION 0.1.1
    LANGUAGES C CXX)

# Build flags
if (MSVC)
    add_compile_options("-D_MSC_VER")
elseif(NOT APPLE)
    add_compile_options("-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE")
endif()
set(CMAKE_C_FLAGS "-ggdb -fpack-struct=1 -fPIC -std=c99 -Wall")

# Library
include_directories(include)
file(GLOB SOURCES "src/*.c")
add_library(libirig106 SHARED STATIC ${SOURCES})
install(TARGETS libirig106 DESTINATION ${PROJECT_SOURCE_DIR})

# Tests
enable_testing()
include(CTest)

# Unity (testing library)
add_subdirectory(tests/unity)
include_directories(tests/unity/src)
include_directories(tests/unity/extras/fixture/src)
include_directories(tests/unity/extras/memory/src)

file(GLOB TESTS "tests/test_*.c")
add_executable(test_runner
    tests/all_tests.c
    tests/unity/src/unity.c
    tests/unity/extras/fixture/src/unity_fixture.c
    tests/unity/extras/memory/src/unity_memory.c
    ${TESTS}
)
target_link_libraries(test_runner libirig106 unity)

add_test(all_tests run_tests)
