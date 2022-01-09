cmake_minimum_required(VERSION 3.18)
project(Semestralka C)

set(CMAKE_C_STANDARD 99)

add_executable(TCPServer TCPServer.c)
add_executable(Client Client.c)
target_link_libraries(TCPServer pthread m)