    cmake_minimum_required(VERSION 3.30)
project(Client)

set(CMAKE_CXX_STANDARD 20)

add_executable(Client main.cpp)

add_subdirectory("./Libraries/Socket")
include_directories("./Libraries/Socket")
target_link_libraries(Client Socket)
