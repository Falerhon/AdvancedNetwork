cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(client main.cpp
        ../../src/CubeGame.cpp
        ../../src/CubeGame.h
        ../../src/GameObject/Object/MBObject.cpp
        ../../src/GameObject/Object/MBObject.h
        ../../src/GameObject/RigidBody/MBRigidBody.cpp
        ../../src/GameObject/RigidBody/MBRigidBody.h
        ../../src/GameObject/Drawable/MBDrawable.cpp
        ../../src/GameObject/Drawable/MBDrawable.h
        ../../src/GameObject/Object/MBCubeObject.cpp
        ../../src/GameObject/Object/MBCubeObject.h
        ../../src/GameObject/Object/MBSphereObject.cpp
        ../../src/GameObject/Object/MBSphereObject.h
        ../../src/Network/APIHandler.cpp
        ../../src/Network/APIHandler.h
        ../../src/GameObject/Object/GameState.h
        ../../src/GameObject/Drawable/MBUiRenderer.cpp
        ../../src/GameObject/Drawable/MBUiRenderer.h
        ../../src/Network/MatchmakingManager.cpp
        ../../src/Network/MatchmakingManager.h
        ../../src/Network/LinkingContext.cpp
        ../../src/Network/LinkingContext.h
        ../../src/Network/NetworkObjectFactory.cpp
        ../../src/Network/NetworkObjectFactory.h
        ../../src/GameLogic/GameLogic.cpp
        ../../src/GameLogic/GameLogic.h
        ../../src/Network/NetworkEvent.h)

target_include_directories(client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(client PRIVATE
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
        Bullet3Common
        cpr::cpr
        nlohmann_json::nlohmann_json
        enet)

target_compile_definitions(client PRIVATE IS_CLIENT)