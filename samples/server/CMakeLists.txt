add_executable(server
        main.cpp
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
        ../../src/Network/NetworkEvent.h
)

target_link_libraries(server PRIVATE MagnumBootstrap)

target_compile_definitions(server PRIVATE IS_SERVER)