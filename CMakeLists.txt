cmake_minimum_required(VERSION 3.16)
project(TinyServer_async)

set(CMAKE_CXX_STANDARD 14)

link_libraries(pthread)

include_directories(common threadpool)

file(GLOB_RECURSE conn connection/*.*)
file(GLOB_RECURSE common common/*.*)
file(GLOB_RECURSE threadpool threadpool/*.*)

add_executable(TinyServer_async main.cpp ${conn} ${common} ${threadpool})
