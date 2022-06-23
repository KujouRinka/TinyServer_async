cmake_minimum_required(VERSION 3.16)
project(TinyServer_async)

set(CMAKE_CXX_STANDARD 17)

link_libraries(pthread)

file(GLOB_RECURSE conn connection/*.*)
file(GLOB_RECURSE common common/*.*)
file(GLOB_RECURSE threadpool threadpool/*.*)

file(GLOB_RECURSE src ${conn} ${common} ${threadpool})
file(GLOB_RECURSE test test/*.*)


include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

add_executable(
        test_tinyserver
        ${test}
        ${src}
)
target_link_libraries(
        test_tinyserver
        gtest_main
        gmock_main
)
include(GoogleTest)
gtest_discover_tests(test_tinyserver)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

include_directories(./)

add_executable(TinyServer_async main.cpp ${src})
