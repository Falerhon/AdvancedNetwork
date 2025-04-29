#ifndef CUBEGAME_H
#define CUBEGAME_H

#include <vector>
#include "Magnum/Platform/GlfwApplication.h"
#include "Magnum/Shaders/PhongGL.h"
#include "Network/LinkingContext.h"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "enet6/enet.h"
#include "GameObject/Drawable/MBUiRenderer.h"
#include "Magnum/ImGuiIntegration/Context.hpp"
#include "Magnum/Timeline.h"
#include "Magnum/GL/Mesh.h"
#include "Magnum/SceneGraph/MatrixTransformation3D.h"
#include "Magnum/SceneGraph/Scene.h"
#include "Magnum/SceneGraph/FeatureGroup.h"
#include "Network/MatchmakingManager.h"


// Forward declarations
class MBObject;
struct InstanceData;
class APIHandler;
class LinkingContext;
class MatchmakingManager;
class UiRenderer;

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class CubeGame : public Magnum::Platform::Application {
public:
    explicit CubeGame(const Arguments &arguments);
    ~CubeGame();

    void Init();
    void Init(ENetHost *host, ENetPeer* _peer = nullptr);
    void tickEvent() override;
    void Shutdown();

    bool IsGameRunning() const;

    void SpawnProjectile(Vector2 position);
private:
    void drawEvent() override;

    void TakeSnapshot();
    void ReadSnapshot(const uint8_t *data, size_t offset);

    void SendInput(KeyEvent &event);
    void SendInput(PointerEvent &event);
    void ReceiveKeyboardInput(const uint8_t *data, size_t offset);
    void ReceiveMouseInput(const uint8_t *data, size_t offset);
    void ReceivePacket(const ENetPacket* packet);

    void keyPressEvent(KeyEvent &event) override;
    void keyReleaseEvent(KeyEvent &event) override;
    void textInputEvent(TextInputEvent &event) override;
    void pointerMoveEvent(PointerMoveEvent &event) override;
    void pointerPressEvent(PointerEvent &event) override;
    void pointerReleaseEvent(PointerEvent &event) override;
    void viewportEvent(ViewportEvent &event) override;

    bool _isRunning;

    Containers::Array<InstanceData> boxInstancesDatas, sphereInstancesDatas;
    std::vector<MBObject *> networkObjects;


//#ifdef IS_CLIENT
    //********* Rendering *********//
    Shaders::PhongGL shader{NoCreate};
    ImGuiIntegration::Context _imguiContext{NoCreate};
    UiRenderer *_uiRenderer;

    //********* Matchmaking *********//
    MatchmakingManager *_matchmaking;
//#endif

    //********* Network *********//
    ENetHost *host;
    ENetPeer *peer;

    //********* Bullet Physics *********//
    btDbvtBroadphase bBroadPhase; //Using a Dynamic Bounding Volume Tree to do the broad phase collision detection
    btDefaultCollisionConfiguration bCollisionConfig;
    btCollisionDispatcher bDispatcher{&bCollisionConfig}; //Handles collision detection during the narrow phase
    btSequentialImpulseConstraintSolver bSolver; //Resolve the forces, constraints and collisions between Rigid bodies

    //********* Objects *********//
    //Construct items without initializing them
    GL::Mesh box{NoCreate}, sphere{NoCreate};
    GL::Buffer boxInstanceBuffer{NoCreate}, sphereInstanceBuffer{NoCreate};

    btBoxShape boxShape{{.5f, .5f, .5f}}; //Collision shape used for collisions
    btSphereShape sphereShape{{0.5f}};
    btBoxShape groundShape{{8.0f, .5f, 8.0f}};

    //The world has to live longer than the scene so the RigidBody instances can remove themselves
    btDiscreteDynamicsWorld bWorld{&bDispatcher, &bBroadPhase, &bSolver, &bCollisionConfig};

    //********* Scene *********//
    Scene3D scene;
    SceneGraph::DrawableGroup3D drawableGroup;
    Timeline timeline;
    SceneGraph::Camera3D *camera;
    Object3D *cameraRig, *cameraObject;

    //********* Game Logic *********//
    APIHandler *API;
    LinkingContext *linking_context;
};


#endif //CUBEGAME_H
