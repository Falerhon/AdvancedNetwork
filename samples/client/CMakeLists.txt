add_executable(client main.cpp Client.cpp)
target_link_libraries(client PUBLIC falcon spdlog::spdlog_header_only)