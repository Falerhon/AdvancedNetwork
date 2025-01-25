    cmake_minimum_required(VERSION 3.30)
    project(Client)

    set(CMAKE_CXX_STANDARD 20)

    # Add the Socket library (source is outside, binary is inside the Client build folder)
    add_subdirectory(../Socket ${CMAKE_BINARY_DIR}/Client_Socket_build)

    # Add the Client executable (main.cpp is in the Client folder)
    add_executable(Client main.cpp)

    # Link the Client executable with the Socket library
    target_link_libraries(Client PRIVATE Socket)

    # Add include directories for the Client
    target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/../Socket)
