#include <fstream>
#include <iostream>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/GlfwApplication.h>


#include "Magnum/ImGuiIntegration/Context.hpp"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Pointer.h"
#include "entt/entt.hpp"
#include "GameObject/Drawable/MBDrawable.h"
#include "GameObject/Object/MBCubeObject.h"
#include "GameObject/Object/MBSphereObject.h"
#include "GameObject/RigidBody/MBRigidBody.h"
#include "Magnum/Timeline.h"
#include "Magnum/BulletIntegration/DebugDraw.h"
#include "Magnum/GL/Mesh.h"
#include "Magnum/GL/Renderer.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Matrix4.h"
#include "Magnum/Math/Time.h"
#include "Magnum/MeshTools/Compile.h"
#include "Magnum/Primitives/Cube.h"
#include "Magnum/Primitives/UVSphere.h"
#include "Magnum/SceneGraph/Camera.h"
#include "Magnum/SceneGraph/FeatureGroup.h"
#include "Magnum/SceneGraph/MatrixTransformation3D.h"
#include "Magnum/SceneGraph/Object.h"
#include "Magnum/SceneGraph/Scene.h"
#include "Magnum/Shaders/PhongGL.h"
#include "Magnum/Trade/MeshData.h"
#include "enet6/enet.h"
#include "GameLogic/GameLogic.h"
#include "Network/APIHandler.h"
#include "GameObject//Drawable/MBUiRenderer.h"
#include "Network/LinkingContext.h"
#include "Network/MatchmakingManager.h"

//TODO : SET THE ONLINE SERVE URL
#define OnlineServerUrl "http://localhost:5039"

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class MyApplication : public Platform::Application {
public:
    explicit MyApplication(const Arguments &arguments);

private:
    void tickEvent() override;

    void drawEvent() override;

    void keyPressEvent(KeyEvent &event) override;

    void keyReleaseEvent(KeyEvent &event) override;

    void textInputEvent(TextInputEvent &event) override;

    void pointerMoveEvent(PointerMoveEvent &event) override;

    void pointerPressEvent(PointerEvent &event) override;

    void pointerReleaseEvent(PointerEvent &event) override;

    void viewportEvent(ViewportEvent &event) override;

    void SaveWorldState(const std::vector<MBObject *> &objects, const std::string &filename);

    void LoadWorldState(std::vector<MBObject *> &objects, const std::string &filename);

    //TODO : TRANSFORM INTO CLASSES OR ECS
    //Construct items without initializing them
    GL::Mesh box{NoCreate}, sphere{NoCreate};
    GL::Buffer boxInstanceBuffer{NoCreate}, sphereInstanceBuffer{NoCreate};
    Shaders::PhongGL shader{NoCreate};
    BulletIntegration::DebugDraw debugDraw{NoCreate};
    Containers::Array<InstanceData> boxInstancesDatas, sphereInstancesDatas;

    //Bullet physics
    btDbvtBroadphase bBroadPhase; //Using a Dynamic Bounding Volume Tree to do the broad phase collision detection
    btDefaultCollisionConfiguration bCollisionConfig;
    btCollisionDispatcher bDispatcher{&bCollisionConfig}; //Handles collision detection during the narrow phase
    btSequentialImpulseConstraintSolver bSolver; //Resolve the forces, constraints and collisions between Rigid bodies

    btBoxShape boxShape{{.5f, .5f, .5f}}; //Collision shape used for collisions
    btSphereShape sphereShape{{0.5f}};
    btBoxShape groundShape{{8.0f, .5f, 8.0f}};


    //The world has to live longer than the scene so the RigidBody instances can remove themselves
    btDiscreteDynamicsWorld bWorld{&bDispatcher, &bBroadPhase, &bSolver, &bCollisionConfig};

    //Scene config
    Scene3D scene;
    SceneGraph::Camera3D *camera;
    SceneGraph::DrawableGroup3D drawableGroup;
    Timeline timeline;

    std::vector<MBObject *> objects;

    Object3D *cameraRig, *cameraObject;

    ImGuiIntegration::Context _imguiContext{NoCreate};
    MatchmakingManager *_matchmaking;

    UiRenderer *_uiRenderer;

    bool drawObjects{true}, drawDebug{false}, shootBox{false};

    APIHandler *API;
};

MyApplication::MyApplication(const Arguments &arguments): Platform::Application{arguments, NoCreate} { {
        const Vector2 dpiScaling = this->dpiScaling({});
        Configuration conf;
        conf.setTitle("Network - Custom Engine")
                .setSize(conf.size(), dpiScaling);

        GLConfiguration glConf;
        //Try 8x Multisample Anti-Aliasing, fall back to 2x samples if not possible.
        glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
        if (!tryCreate(conf, glConf)) {
            create(conf, glConf.setSampleCount(0)); //Set the samples to 0 if unsupported
        }
    }

    //API set up
    API = new APIHandler(OnlineServerUrl);

    //Linking Context
    LinkingContext *linking_context = new LinkingContext();

    _uiRenderer = new UiRenderer(API);

    //Camera set up
    cameraRig = new Object3D{&scene};
    cameraRig->translate(Vector3::yAxis(3.0f))
            .rotateY(-25.0_degf);

    cameraObject = new Object3D{cameraRig};
    cameraObject->translate(Vector3::zAxis(20.0f))
            .rotateX(-25.0_degf);

    camera = new SceneGraph::Camera3D(*cameraObject);
    camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
            .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.1f, 100.0f))
            .setViewport(GL::defaultFramebuffer.viewport().size());

    //Create an instanced shader
    shader = Shaders::PhongGL{
        Shaders::PhongGL::Configuration{}
        .setFlags(Shaders::PhongGL::Flag::VertexColor | Shaders::PhongGL::Flag::InstancedTransformation)
        //Combines the 2 flags with |
    };

    shader.setAmbientColor(0x111111_rgbf)
            .setSpecularColor(0x330000_rgbf)
            .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});

    //Prep the cube and spheres
    box = MeshTools::compile(Primitives::cubeSolid());
    sphere = MeshTools::compile(Primitives::uvSphereSolid(16, 32));

    boxInstanceBuffer = GL::Buffer{};
    sphereInstanceBuffer = GL::Buffer{};

    box.addVertexBufferInstanced(boxInstanceBuffer, 1, 0,
                                 Shaders::PhongGL::TransformationMatrix{},
                                 Shaders::PhongGL::NormalMatrix{},
                                 Shaders::PhongGL::Color3{});

    sphere.addVertexBufferInstanced(sphereInstanceBuffer, 1, 0,
                                    Shaders::PhongGL::TransformationMatrix{},
                                    Shaders::PhongGL::NormalMatrix{},
                                    Shaders::PhongGL::Color3{});

    //Set up the renderer for the debug
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
    GL::Renderer::setPolygonOffset(2.0f, 0.5f);

    //Bullet Set up
    debugDraw = BulletIntegration::DebugDraw{};
    debugDraw.setMode(BulletIntegration::DebugDraw::Mode::DrawWireframe);

    bWorld.setGravity({.0f, -10.f, .0f});
    bWorld.setDebugDrawer(&debugDraw);

    //Spawn of ground and cubes
//#ifdef IS_SERVER

    int nbOfBoxPerSides = 2;
    float centerOffset = (nbOfBoxPerSides - 1) / 2.0f;
    float playersOffset = 20;
    //Create boxes with random colors
    Deg boxHue = 42.0_degf;
    //Each player
    for (int h = 0; h < 4; h++) {

        //Create the ground
        MBCubeObject(&scene, bWorld, 0.f, {8.f, .5f, 8.f}, {h * playersOffset, 0, 0}, boxInstancesDatas,
                     drawableGroup, 0xffffff_rgbf, groundShape);

        //Spawning boxes
        for (Int i = 0; i != nbOfBoxPerSides; ++i) {
            for (Int j = 0; j != nbOfBoxPerSides; ++j) {
                for (Int k = 0; k != nbOfBoxPerSides; ++k) {
                    MBCubeObject *cube = new MBCubeObject(&scene, bWorld, 1.f, {.5f, .5f, .5f},
                                                          {(i + h * playersOffset) - centerOffset, j + centerOffset, k - centerOffset},
                                                          boxInstancesDatas,
                                                          drawableGroup, Color3::fromHsv({boxHue += 137.5_degf, .75f, .9f}),
                                                          boxShape, h);
                    cube->SetNetworkId(linking_context->Register(cube));
                    objects.emplace_back(cube);
                }
            }
        }

        GameLogic::GetInstance().AddPlayer(std::pow(nbOfBoxPerSides, 3));
    }

//#endif

//#ifdef IS_CLIENT

    //imGui set up
    _imguiContext = ImGuiIntegration::Context(Vector2(windowSize() / dpiScaling()),
                                              windowSize(), framebufferSize());
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
                                   GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
//#endif

    _matchmaking = new MatchmakingManager(API);

    //TODO : REMOVE THIS
    GameLogic::GetInstance().SetGameState(GameState::InGame);

    // Loop at 60 Hz max
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    timeline.start();
}

void MyApplication::tickEvent() {
    timeline.nextFrame();
    //Step bullet simulation
    bWorld.stepSimulation(timeline.previousFrameDuration(), 5);

    if (GameLogic::GetInstance().GetGameState() == GameState::LookingForSession)
        _matchmaking->update();

    //TODO : Do the check somewhere else
    //TODO : Destroy MBObjects, not just objects
    //Remove any object far from the origin
    std::vector<int> objectsToDestroy;
    int x = 0;
    for (MBObject *obj : objects) {
        if (obj->getMBRigidBody()->transformation().translation().y() < -2) {
            objectsToDestroy.push_back(x);
        }
        x++;
    }

    for (int i : objectsToDestroy) {
        MBObject *obj = objects[i];
        objects.erase(std::find(objects.begin(), objects.end(), obj));
        delete obj;
    }

//#ifdef IS_CLIENT
    redraw();
//#endif
}

void MyApplication::drawEvent() {
//#ifdef IS_CLIENT

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    //Draw the cubes and spheres
    if (drawObjects) {
        //Repopulate the instance datas with updated transforms and colors
        arrayResize(boxInstancesDatas, 0);
        arrayResize(sphereInstancesDatas, 0);
        camera->draw(drawableGroup);

        shader.setProjectionMatrix(camera->projectionMatrix());

        //Upload the instance dayas to the GPU
        //Take one draw call per object type
        boxInstanceBuffer.setData(boxInstancesDatas, GL::BufferUsage::DynamicDraw);
        box.setInstanceCount(boxInstancesDatas.size());
        shader.draw(box);

        sphereInstanceBuffer.setData(sphereInstancesDatas, GL::BufferUsage::DynamicDraw);
        sphere.setInstanceCount(sphereInstancesDatas.size());
        shader.draw(sphere);
    }

    //Draw the debugs
    if (drawDebug) {
        if (drawObjects) {
            //GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::LessOrEqual);
            //Avoid flickering when drawing on top of objects

            debugDraw.setTransformationProjectionMatrix(
                camera->projectionMatrix() * camera->cameraMatrix()
            );

            bWorld.debugDrawWorld();
        }

        //Reset the parameter
        if (drawObjects) {
            GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
        }
    }

    //Draw the UI
    _imguiContext.newFrame();

    _uiRenderer->draw(GameLogic::GetInstance().GetGameState());

    if (ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    //Sets the renderer for 2d rendering
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    _imguiContext.drawFrame();

    /* Update application cursor */
    _imguiContext.updateApplicationCursor(*this);

    //Reset the renderer state for 3d rendering
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);

    swapBuffers();
//#endif
}

void MyApplication::keyPressEvent(KeyEvent &event) {
//#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() != GameState::InGame) {
        _imguiContext.handleKeyPressEvent(event);
        event.setAccepted();
        return;
    }
//#endif


    //Movement
    if (event.key() == Key::Down || event.key() == Key::S) {
        cameraObject->translate(Vector3({0.f, 0.f, 5.f}));
    } else if (event.key() == Key::Up || event.key() == Key::W) {
        cameraObject->translate(Vector3({0.f, 0.f, -5.f}));
    } else if (event.key() == Key::Left || event.key() == Key::A) {
        cameraObject->translate(Vector3({-5.f, 0.f, 0.f}));
    } else if (event.key() == Key::Right || event.key() == Key::D) {
        cameraObject->translate(Vector3({5.f, 0.f, 0.f}));
    } else if (event.key() == Key::Q) {
        cameraObject->translate(Vector3({0.f, 5.f, 0.f}));
    } else if (event.key() == Key::E) {
        cameraObject->translate(Vector3({0.f, -5.f, 0.f}));
    } else if (event.key() == Key::K) {
        std::string filename = "../SaveFile.bin";
        SaveWorldState(objects, filename);
    } else if (event.key() == Key::L) {
        std::string filename = "../SaveFile.bin";
        LoadWorldState(objects, filename);
    } else if (event.key() == Key::P) {
        auto response = API->get("/api/Stats/info");
        if (response.status_code == 200) {
            Debug{} << "Stats request successful";
        } else {
            Error{} << "Failed to request stats : " << response.status_code;
        }
    }

    event.setAccepted();
}

void MyApplication::keyReleaseEvent(KeyEvent &event) {
//#ifdef IS_CLIENT
    if (_imguiContext.handleKeyReleaseEvent(event)) return;
//#endif
}

void MyApplication::textInputEvent(TextInputEvent &event) {
//#ifdef IS_CLIENT
    if (_imguiContext.handleTextInputEvent(event)) return;
//#endif
}

void MyApplication::pointerMoveEvent(PointerMoveEvent &event) {
//#ifdef IS_CLIENT
    if (_imguiContext.handlePointerMoveEvent(event)) return;
//#endif
}

void MyApplication::pointerPressEvent(PointerEvent &event) {
    //Shoot an object on click
    if (!event.isPrimary() || !(event.pointer() & Pointer::MouseLeft))
        return;

//#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() != GameState::InGame) {
        _imguiContext.handlePointerPressEvent(event);
        event.setAccepted();
        return;
    }
//#endif


    //Spawn the projectile
//#ifdef IS_SERVER
    //Scale the position from relative to the window size to relative to the framebuffer size
    //Since the HiDPI can vary
    const Vector2 position = event.position() * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.f) * (position / Vector2{framebufferSize()} - Vector2{.5f}) * camera->
                               projectionSize();
    const Vector3 direction = (cameraObject->absoluteTransformation().rotationScaling() * Vector3(clickPoint, -1.f)).
            normalized();

    auto *object = new MBSphereObject(&scene, bWorld, 1.f, Vector3{0.5f},
                                      {cameraObject->absoluteTransformation().translation()}, sphereInstancesDatas,
                                      drawableGroup, 0x221111_rgbf,
                                      sphereShape);
    //has to be done explicitly (only done implicitly for kinematic objects)

    //Set initial velocity
    object->getMBRigidBody()->getRigidBody().setLinearVelocity(btVector3{direction * 25.f});

    //object->SetNetworkId(linking_context->Register(object));
    //Add to snapshot
    objects.push_back(object);

//#endif
    event.setAccepted();
}

void MyApplication::pointerReleaseEvent(PointerEvent &event) {
//#ifdef IS_CLIENT
    if (_imguiContext.handlePointerReleaseEvent(event)) return;
//#endif
}

void MyApplication::viewportEvent(ViewportEvent &event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
//#ifdef IS_CLIENT
    _imguiContext.relayout(Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(),
                          event.framebufferSize());
//#endif
}

void MyApplication::SaveWorldState(const std::vector<MBObject *> &objects, const std::string &filename) {
    std::ofstream saveFile(filename);
    for (auto object: objects) {
        object->SerializeObjectToBinary(saveFile);
    }

    saveFile.close();
}

void MyApplication::LoadWorldState(std::vector<MBObject *> &objects, const std::string &filename) {
    std::ifstream saveFile(filename);
    for (auto object: objects) {
        object->DeserializeObjectToBinary(saveFile);
    }

    saveFile.close();
}


MAGNUM_APPLICATION_MAIN(MyApplication)
