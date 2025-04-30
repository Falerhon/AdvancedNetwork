add_executable(server main.cpp)

target_link_libraries(server PRIVATE MagnumBootstrap)

target_compile_definitions(MagnumBootstrap PUBLIC IS_SERVER)