FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        devel
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)


include(CTest)
include(Catch)

include_directories(${CMAKE_SOURCE_DIR}/samples/client)
include_directories(${CMAKE_SOURCE_DIR}/samples/server)

add_executable(tests test.cpp ${CMAKE_SOURCE_DIR}/samples/server/Server.cpp ${CMAKE_SOURCE_DIR}/samples/client/Client.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain falcon)
catch_discover_tests(tests TEST_PREFIX "UnitTests." TEST_LIST myTests)
