cmake_minimum_required(VERSION 2.8)
project(final CXX)
add_executable(final server.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
target_link_libraries(final ev pthread)
