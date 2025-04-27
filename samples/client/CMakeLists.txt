add_executable(client main.cpp
        ../../src/Network/LinkingContext.cpp
        ../../src/Network/LinkingContext.h)
target_link_libraries(client PRIVATE enet)
target_compile_definitions(client PRIVATE IS_CLIENT)