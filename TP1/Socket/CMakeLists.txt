cmake_minimum_required(VERSION 3.30)
project(Socket)

set(CMAKE_CXX_STANDARD 20)

add_library(Socket STATIC library.cpp)
