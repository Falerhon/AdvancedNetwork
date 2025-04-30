add_executable(client main.cpp)

target_link_libraries(client PRIVATE MagnumBootstrap)

target_compile_definitions(MagnumBootstrap PRIVATE IS_CLIENT)