add_executable(server main.cpp
        ../../src/Network/LinkingContext.cpp
        ../../src/Network/LinkingContext.h)
target_link_libraries(server PRIVATE enet)
target_compile_definitions(server PRIVATE IS_SERVER)