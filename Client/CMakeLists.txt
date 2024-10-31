cmake_minimum_required(VERSION 3.29)
project(Client)

set(CMAKE_CXX_STANDARD 20)

add_executable(Client main.cpp)

target_link_libraries(Client ws2_32)
