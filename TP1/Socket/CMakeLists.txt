cmake_minimum_required(VERSION 3.30)
project(Socket)

set(CMAKE_CXX_STANDARD 20)

add_library(Socket STATIC library.cpp
        Socket.h
        Socket_windows.cpp
        Socket_posix.cpp
        Socket_common.cpp)

# Platform-specific flags
if(WIN32)
    add_definitions(-D_WIN32)
    # Link against the ws2_32 library (Winsock)
    target_link_libraries(Socket PRIVATE ws2_32)

elseif(UNIX)
    add_definitions(-D__unix__)
    target_link_libraries(Socket PRIVATE pthread)
endif()