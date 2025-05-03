cmake_minimum_required(VERSION 3.30)
project(MagnumBootstrap)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(externals)
find_package(Magnum REQUIRED
        GL
        GlfwApplication
        Primitives
        Shaders
        Trade
        MeshTools
        Primitives
        SceneGraph)
find_package(MagnumIntegration REQUIRED ImGui Bullet)
find_package(Bullet REQUIRED Dynamics)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_library(MagnumBootstrap STATIC
        src/main.cpp
        src/GameObject/Player/Player.cpp
        src/GameObject/Player/Player.h
)

target_include_directories(MagnumBootstrap PUBLIC inc)
set_property(TARGET MagnumBootstrap PROPERTY CXX_STANDARD 20)
target_link_libraries(MagnumBootstrap PUBLIC
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        MagnumIntegration::Bullet
        Bullet::Dynamics
        MagnumIntegration::ImGui
        BulletDynamics
        BulletCollision
        LinearMath
        Bullet3Common)
target_link_libraries(MagnumBootstrap PUBLIC EnTT::EnTT)

target_link_libraries(MagnumBootstrap PUBLIC enet)

target_link_libraries(MagnumBootstrap PUBLIC cpr::cpr)
target_link_libraries(MagnumBootstrap PUBLIC nlohmann_json::nlohmann_json)

#Including windows DLL required to run the app
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    set(MINGW_DLLS
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
    )

    foreach(DLL ${MINGW_DLLS})
        add_custom_command(TARGET MagnumBootstrap POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/${DLL}"
                "$<TARGET_FILE_DIR:MagnumBootstrap>/${DLL}"
        )
    endforeach()
endif()

# Link platform-specific networking libraries
if(WIN32)
    target_link_libraries(MagnumBootstrap PUBLIC ws2_32)
else()
    target_link_libraries(MagnumBootstrap PUBLIC pthread)
endif()

add_subdirectory(samples)
