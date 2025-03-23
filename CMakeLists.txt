cmake_minimum_required(VERSION 3.30)
project(MagnumBootstrap)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

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

add_executable(MagnumBootstrap
        src/main.cpp
        src/GameObject/Object/MBObject.cpp
        src/GameObject/Object/MBObject.h
        src/GameObject/RigidBody/MBRigidBody.cpp
        src/GameObject/RigidBody/MBRigidBody.h
        src/GameObject/Drawable/MBDrawable.cpp
        src/GameObject/Drawable/MBDrawable.h
        src/GameObject/Object/MBCubeObject.cpp
        src/GameObject/Object/MBCubeObject.h)

target_include_directories(MagnumBootstrap PUBLIC inc)
set_property(TARGET MagnumBootstrap PROPERTY CXX_STANDARD 20)
target_link_libraries(MagnumBootstrap PRIVATE
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