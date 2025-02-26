add_executable(server main.cpp Server.cpp)
target_link_libraries(server PUBLIC falcon spdlog::spdlog_header_only)
