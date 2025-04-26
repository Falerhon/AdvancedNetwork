include(FetchContent)

if(WIN32)
    set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4.bin.WIN64 ${CMAKE_PREFIX_PATH})
endif()

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

set(MAGNUM_WITH_GLFWAPPLICATION ON CACHE BOOL "" FORCE)
set(MAGNUM_WITH_BULLET ON CACHE BOOL "" FORCE)
set(MAGNUM_WITH_IMGUI ON CACHE BOOL "" FORCE)

add_subdirectory(bullet)

add_subdirectory(corrade)
add_subdirectory(magnum)
add_subdirectory(magnum-integration)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.15.0
)

FetchContent_MakeAvailable(entt)

FetchContent_Declare(
        enet
        GIT_REPOSITORY https://github.com/VALERE91/enet6.git
        GIT_TAG main
)

FetchContent_MakeAvailable(enet)

FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG        1.11.2
)

set(CPR_USE_SYSTEM_CURL OFF)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(json)


set(CMAKE_CXX_STANDARD 20)