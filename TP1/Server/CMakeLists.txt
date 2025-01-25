cmake_minimum_required(VERSION 3.30)
project(Server)

set(CMAKE_CXX_STANDARD 20)

# Add the Socket library (source is outside, binary is inside the Server build folder)
add_subdirectory(../Socket ${CMAKE_BINARY_DIR}/Server_Socket_build)

# Add the Server executable (main.cpp is in the Server folder)
add_executable(Server main.cpp)

# Link the Server executable with the Socket library
target_link_libraries(Server PRIVATE Socket)

# Add include directories for the Server
target_include_directories(Server PRIVATE ${CMAKE_SOURCE_DIR}/../Socket)