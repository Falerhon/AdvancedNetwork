cmake_minimum_required(VERSION 3.30)
project(falcon)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    set(FALCON_BACKEND src/falcon_windows.cpp)
    set(WINDOWS_LIBS Ws2_32)
else ()
    set(FALCON_BACKEND src/falcon_posix.cpp)
endif (WIN32)

add_library(falcon STATIC inc/falcon.h src/falcon_common.cpp ${FALCON_BACKEND})
add_library(stream STATIC inc/stream.h src/stream.cpp)
target_include_directories(falcon PUBLIC inc)
target_include_directories(stream PUBLIC inc)
target_link_libraries(falcon PUBLIC spdlog::spdlog_header_only fmt::fmt-header-only)
target_link_libraries(falcon PUBLIC stream)

if(WIN32)
    target_link_libraries(falcon PRIVATE ${WINDOWS_LIBS})
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address -g -O1 -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

add_subdirectory(externals)
add_subdirectory(samples)
add_subdirectory(tests)

enable_testing()