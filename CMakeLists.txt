
cmake_minimum_required(VERSION 3.12)

project(libirig106
    VERSION 0.1.2
    LANGUAGES C CXX)

# Build flags
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Library
include_directories(include)
file(GLOB SOURCES "src/*.c")
add_library(libirig106 SHARED STATIC ${SOURCES})
set_property(TARGET libirig106 PROPERTY C_STANDARD 99)
set_property(TARGET libirig106 PROPERTY POSITION_INDEPENDENT_CODE ON)
install(TARGETS libirig106 DESTINATION ${PROJECT_SOURCE_DIR})

# Unity (testing library)
add_subdirectory(tests/unity)
include_directories(tests/unity/src)
include_directories(tests/unity/extras/fixture/src)
include_directories(tests/unity/extras/memory/src)

# Test runner (includes unity and test suite)
file(GLOB TESTS "tests/test_*.c")
add_executable(test_runner
    tests/all_tests.c
    tests/unity/src/unity.c
    tests/unity/extras/fixture/src/unity_fixture.c
    tests/unity/extras/memory/src/unity_memory.c
    ${TESTS}
)
target_link_libraries(test_runner libirig106 unity)
