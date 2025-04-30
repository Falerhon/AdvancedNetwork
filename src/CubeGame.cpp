#include "CubeGame.h"
#include <Magnum/Platform/GlfwApplication.h>

#include "GameLogic/GameLogic.h"
#include "GameObject/Object/MBCubeObject.h"
#include "GameObject/Object/MBSphereObject.h"
#include "Magnum/GL/DefaultFramebuffer.h"
#include "Magnum/GL/Renderer.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Time.h"
#include "Magnum/MeshTools/Compile.h"
#include "Magnum/Primitives/Cube.h"
#include "Magnum/Primitives/UVSphere.h"
#include "Magnum/SceneGraph/Camera.h"
#include "Magnum/Trade/MeshData.h"
#include <algorithm>
#include <iostream>
#include <bits/ostream.tcc>

#include "Network/NetworkEvent.h"
#include "Network/NetworkObjectFactory.h"

#define OnlineServerUrl "http://localhost:5039"

CubeGame::CubeGame(const Arguments &arguments): Platform::Application{arguments, NoCreate} {
    {
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
}

CubeGame::~CubeGame() {
    Shutdown();
}

void CubeGame::Init() {
    //********* Game Logic *********//
    linking_context = new LinkingContext();
    API = new APIHandler(OnlineServerUrl);
    //#ifdef IS_CLIENT
    _matchmaking = new MatchmakingManager(API);
    //********* Rendering *********//
    _uiRenderer = new UiRenderer(API);

    //Create an instanced shader
    shader = Shaders::PhongGL{
        Shaders::PhongGL::Configuration{}
        .setFlags(Shaders::PhongGL::Flag::VertexColor | Shaders::PhongGL::Flag::InstancedTransformation)
    };

    shader.setAmbientColor(0x111111_rgbf)
            .setSpecularColor(0x330000_rgbf)
            .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
    GL::Renderer::setPolygonOffset(2.0f, 0.5f);

    //imGui set up
    _imguiContext = ImGuiIntegration::Context(Vector2(windowSize() / dpiScaling()),
                                              windowSize(), framebufferSize());
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
                                   GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    //#endif

    //********* Objects *********//
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

    float playersOffset = 20;
    for (int player = 0; player < 4; player++) {
        //Create the ground
        MBCubeObject(&scene, bWorld, 0.f, {8.f, .5f, 8.f}, {player * playersOffset, 0, 0}, boxInstancesDatas,
                     drawableGroup, 0xffffff_rgbf, groundShape);
    }

#ifdef IS_SERVER
    bWorld.setGravity({.0f, -10.f, .0f});

    //Create the ground
    MBCubeObject(&scene, bWorld, 0.f, {8.f, .5f, 8.f}, {0, 0, 0}, boxInstancesDatas,
                 drawableGroup, 0xffffff_rgbf, groundShape);

    int nbOfBoxPerSides = 2;
    float centerOffset = (nbOfBoxPerSides - 1) / 2.0f;


    //For each players
    for (int player = 0; player < 4; player++) {
        //Create boxes with random colors
        Deg boxHue = 42.0_degf;
        for (Int i = 0; i != nbOfBoxPerSides; ++i) {
            for (Int j = 0; j != nbOfBoxPerSides; ++j) {
                for (Int k = 0; k != nbOfBoxPerSides; ++k) {
                    MBCubeObject *cube = new MBCubeObject(&scene, bWorld, 1.f, {.5f, .5f, .5f},
                                                          {
                                                              (i + player * playersOffset) - centerOffset,
                                                              j + centerOffset, k - centerOffset
                                                          },
                                                          boxInstancesDatas,
                                                          drawableGroup, Color3::fromHsv({
                                                              boxHue += 137.5_degf, .75f, .9f
                                                          }),
                                                          boxShape);
                    cube->SetNetworkId(linking_context->Register(cube));
                    networkObjects.emplace_back(cube);
                }
            }
        }
        GameLogic::GetInstance().AddPlayer(std::pow(nbOfBoxPerSides, 3));
    }
#endif


    //TODO : REMOVE THIS
    GameLogic::GetInstance().SetGameState(GameState::InGame);

    // Loop at 60 Hz max
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    timeline.start();
    _isRunning = true;
}

void CubeGame::Init(ENetHost *_host) {
    Init();

    host = _host;
}

void CubeGame::tickEvent() {
    timeline.nextFrame();

    //Listen for packets
    ENetEvent event;
    if (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Received packet of size " << event.packet->dataLength << " on channel " << (int) event
                        .channelID << " from " << event.peer->address.port << std::endl;
                ReceivePacket(event.packet);
                enet_packet_destroy(event.packet);
                break;

            default:
                break;
        }
    }

#ifdef IS_SERVER
    //Remove any object far from the origin
    std::vector<int> objectsToDestroy;
    int x = 0;
    for (MBObject *obj: networkObjects) {
        if (obj->getMBRigidBody()->transformation().translation().y() < -2) {
            objectsToDestroy.push_back(x);
        }
        x++;
    }

    for (int i: objectsToDestroy) {
        MBObject *obj = networkObjects[i];
        networkObjects.erase(std::find(networkObjects.begin(), networkObjects.end(), obj));
        delete obj;
    }

    //Step bullet simulation
    bWorld.stepSimulation(timeline.previousFrameDuration(), 5);

    TakeSnapshot();
#endif

    //#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() == GameState::LookingForSession)
        _matchmaking->update();

    redraw();
    //#endif
}

void CubeGame::Shutdown() {
    _isRunning = false;
    delete linking_context;
    delete API;
    delete camera;
    delete cameraRig;
    delete cameraObject;

#ifdef IS_CLIENT
    delete _matchmaking;
    delete _uiRenderer;
#endif

    for (auto obj: networkObjects) {
        delete obj;
    }
    networkObjects.clear();
}

bool CubeGame::IsGameRunning() const {
    return _isRunning;
}

void CubeGame::drawEvent() {
    //#ifdef IS_CLIENT
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    //Draw the cubes and spheres
    //Repopulate the instance datas with updated transforms and colors
    arrayResize(boxInstancesDatas, 0);
    arrayResize(sphereInstancesDatas, 0);
    camera->draw(drawableGroup);

    shader.setProjectionMatrix(camera->projectionMatrix());

    //Upload the instance datas to the GPU
    //Take one draw call per object type
    boxInstanceBuffer.setData(boxInstancesDatas, GL::BufferUsage::DynamicDraw);
    box.setInstanceCount(boxInstancesDatas.size());
    shader.draw(box);

    sphereInstanceBuffer.setData(sphereInstancesDatas, GL::BufferUsage::DynamicDraw);
    sphere.setInstanceCount(sphereInstancesDatas.size());
    shader.draw(sphere);


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

void CubeGame::TakeSnapshot() {
    std::ofstream saveFile("../SaveFile.bin");
    for (auto object: networkObjects) {
        object->SerializeObjectToBinary(saveFile);
    }

    saveFile.close();
#ifdef IS_SERVER
    char buffer[2048];
    size_t offset = 0;

    NetworkEventType packetType = NetworkEventType::SNAPSHOT;
    memcpy(buffer + offset, &packetType, sizeof(NetworkEventType));
    offset += sizeof(NetworkEventType);

    uint8_t numObjects = networkObjects.size();
    memcpy(buffer + offset, &numObjects, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    for (auto obj: networkObjects) {
        obj->SerializeObject(buffer, offset);
    }

    //Vector3 cameraPosition = cameraObject->absoluteTransformationMatrix().translation();


    ENetPacket *packet = enet_packet_create(buffer, offset, 0);
    enet_host_broadcast(host, 1, packet);
#endif
}

void CubeGame::ReadSnapshot(const uint8_t *data, size_t offset) {
    std::ifstream saveFile("../SaveFile.bin");
    for (auto object: networkObjects) {
        object->DeserializeObjectToBinary(saveFile);
    }

    saveFile.close();
#ifdef IS_CLIENT
    uint8_t numObjects;
    std::memcpy(&numObjects, data + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    for (int i = 0; i < numObjects; i++) {
        NetworkId netID;
        std::memcpy(&netID, data + offset, sizeof(NetworkId));
        offset += sizeof(NetworkId);

        NetworkClassID classId;
        std::memcpy(&classId, data + offset, sizeof(NetworkClassID));
        offset += sizeof(NetworkClassID);

        MBObject *obj = linking_context->GetObjectByNetwordId(netID);
        if (!obj) {
            switch (classId) {
                case NetworkClassID::MBCube:
                    obj = NetworkObjectFactory::Create(classId, &scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, 0, 0},
                                                       boxInstancesDatas, drawableGroup,
                                                       Color3::fromHsv({137.5_degf, .75f, .9f}), boxShape);
                    break;
                case NetworkClassID::MBSphere:
                    obj = NetworkObjectFactory::Create(classId, &scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, 0, 0},
                                                       sphereInstancesDatas, drawableGroup,
                                                       Color3::fromHsv({137.5_degf, .75f, .9f}), sphereShape);
                    break;
                case NetworkClassID::Camera:

                default:
                    break;
            }
        }

        obj->DeserializeObject(data, offset);
    }
#endif
}

void CubeGame::SendInput(KeyEvent &event) {
    char buffer[2048];
    size_t offset = 0;

    NetworkEventType packetType = NetworkEventType::KEYBOARD_INPUT;
    memcpy(buffer + offset, &packetType, sizeof(NetworkEventType));
    offset += sizeof(NetworkEventType);

    Key inputKey = event.key();
    memcpy(buffer + offset, &inputKey, sizeof(Key));
    offset += sizeof(Key);

    ENetPacket *packet = enet_packet_create(buffer, offset, 1);

    enet_host_broadcast(host, 0, packet);
    enet_host_flush(host);
}

void CubeGame::SendInput(PointerEvent &event) {
    char buffer[2048];
    size_t offset = 0;

    NetworkEventType packetType = NetworkEventType::MOUSE_INPUT;
    memcpy(buffer + offset, &packetType, sizeof(NetworkEventType));
    offset += sizeof(NetworkEventType);

    Vector2 eventPos = event.position();
    memcpy(buffer + offset, &eventPos, sizeof(Vector2));
    offset += sizeof(Vector2);

    ENetPacket *packet = enet_packet_create(buffer, offset, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(host, 0, packet);
    enet_host_flush(host);
}

void CubeGame::ReceiveKeyboardInput(const uint8_t *data, size_t offset) {
    Key inputKey;
    memcpy(&inputKey, data + offset, sizeof(Key));

    if (inputKey == Key::Down || inputKey == Key::S) {
        cameraObject->translate(Vector3({0.f, 0.f, 5.f}));
    } else if (inputKey == Key::Up || inputKey == Key::W) {
        cameraObject->translate(Vector3({0.f, 0.f, -5.f}));
    } else if (inputKey == Key::Left || inputKey == Key::A) {
        cameraObject->translate(Vector3({-5.f, 0.f, 0.f}));
    } else if (inputKey == Key::Right || inputKey == Key::D) {
        cameraObject->translate(Vector3({5.f, 0.f, 0.f}));
    } else if (inputKey == Key::Q) {
        cameraObject->translate(Vector3({0.f, 5.f, 0.f}));
    } else if (inputKey == Key::E) {
        cameraObject->translate(Vector3({0.f, -5.f, 0.f}));
    }
}

void CubeGame::ReceiveMouseInput(const uint8_t *data, size_t offset) {
    Vector2 pos;
    memcpy(&pos, data + offset, sizeof(Vector2));

    SpawnProjectile(pos);
}

void CubeGame::ReceivePacket(const ENetPacket *packet) {
    size_t offset = 0;
    NetworkEventType packetType;
    std::memcpy(&packetType, packet->data + offset, sizeof(NetworkEventType));
    offset += sizeof(NetworkEventType);
    std::cout << "Packet is of type " << static_cast<int>(packetType) << "\n";
    switch (packetType) {
        case NetworkEventType::SNAPSHOT:
            ReadSnapshot(packet->data, offset);
            break;
        case NetworkEventType::KEYBOARD_INPUT:
            ReceiveKeyboardInput(packet->data, offset);
            break;
        case NetworkEventType::MOUSE_INPUT:
            ReceiveMouseInput(packet->data, offset);
            break;
        case NetworkEventType::ENDGAME:
            break;
        default:
            break;
    }
}

void CubeGame::keyPressEvent(KeyEvent &event) {
#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() != GameState::InGame) {
        _imguiContext.handleKeyPressEvent(event);
        event.setAccepted();
        return;
    }

    SendInput(event);

#endif

    event.setAccepted();
}

void CubeGame::keyReleaseEvent(KeyEvent &event) {
#ifdef IS_CLIENT
    if (_imguiContext.handleKeyReleaseEvent(event)) {
        event.setAccepted();
        return;
    }
#endif
    event.setAccepted();
}

void CubeGame::textInputEvent(TextInputEvent &event) {
#ifdef IS_CLIENT
    if (_imguiContext.handleTextInputEvent(event)) {
        event.setAccepted();
        return;
    }
#endif
    event.setAccepted();
}

void CubeGame::pointerMoveEvent(PointerMoveEvent &event) {
#ifdef IS_CLIENT
    if (_imguiContext.handlePointerMoveEvent(event)) {
        event.setAccepted();
        return;
    }
#endif
    event.setAccepted();
}

void CubeGame::pointerPressEvent(PointerEvent &event) {
    //Shoot an object on click
    if (!event.isPrimary() || !(event.pointer() & Pointer::MouseLeft)) {
        event.setAccepted();
        return;
    }


#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() != GameState::InGame) {
        _imguiContext.handlePointerPressEvent(event);
        event.setAccepted();
        return;
    }

    SendInput(event);

#endif
    event.setAccepted();
}

void CubeGame::pointerReleaseEvent(PointerEvent &event) {
#ifdef IS_CLIENT
    if (_imguiContext.handlePointerReleaseEvent(event)) {
        event.setAccepted();
        return;
    }
#endif
    event.setAccepted();
}

void CubeGame::viewportEvent(ViewportEvent &event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
#ifdef IS_CLIENT
    _imguiContext.relayout(Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(),
                           event.framebufferSize());
#endif
}

void CubeGame::SpawnProjectile(Vector2 position) {
#ifdef IS_SERVER
    //Scale the position from relative to the window size to relative to the framebuffer size
    //Since the HiDPI can vary
    const Vector2 scaledPos = position * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.f) * (scaledPos / Vector2{framebufferSize()} - Vector2{.5f}) * camera
                               ->
                               projectionSize();
    const Vector3 direction = (cameraObject->absoluteTransformation().rotationScaling() * Vector3(clickPoint, -1.f)).
            normalized();

    auto *object = new MBSphereObject(&scene, bWorld, 1.f, Vector3{0.5f},
                                      {cameraObject->absoluteTransformation().translation()}, sphereInstancesDatas,
                                      drawableGroup, 0x221111_rgbf,
                                      sphereShape);
    //has to be done explicitly (only done implicitly for kinematic objects)

    //Set initial velocity
    auto velocity = direction * 25.f;
    object->getMBRigidBody()->getRigidBody().setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));

    object->SetNetworkId(linking_context->Register(object));
    //Add to snapshot
    networkObjects.push_back(object);
#endif
}
