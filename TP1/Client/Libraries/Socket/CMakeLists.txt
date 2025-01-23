cmake_minimum_required(VERSION 3.30)
project(Socket)

set(CMAKE_CXX_STANDARD 20)

add_library(Socket STATIC library.cpp
        Socket.h
        Socket_windows.cpp
        Socket_posix.cpp
        Socket_common.cpp)

target_link_libraries(Socket ws2_32)