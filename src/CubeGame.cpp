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

#include "GameObject/Player/Player.h"
#include "Magnum/Math/Quaternion.h"
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
    GameLogic::GetInstance().SetAPI(API);
#ifdef IS_CLIENT
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

#endif

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


    float playersOffset = 20;
    for (int player = 0; player < 4; player++) {
        //Create the ground
        auto ground = MBCubeObject(&scene, bWorld, 0.f, {8.f, .5f, 8.f}, {player * playersOffset, 0, 0},
                                   boxInstancesDatas,
                                   drawableGroup, 0x1fffff_rgbf, groundShape);
    }

#ifdef IS_SERVER

    GameLogic::GetInstance().SetGameState(GameState::InGame);

    if (API->loginServer("Zip", "Zap")) {
        nlohmann::json payload = {
            {"id", 0},
            {"ip", "localhost"},
            {"port", 1234},
            {"maxPlayers", 4},
            {"currentPlayers", 0},
            {"isOnline", true},
            {"isOccupied", false},
            {"lastHeartbeat", "2025-04-30T20:22:02.445Z"}
        };

        auto response = API->post("/api/GameServer/register", payload);

        if (response.status_code != 201 && response.status_code != 200) {
            std::cout << "Could not register to the online server : " << response.status_code << std::endl;
        }
    } else {
        std::cout << "Failed to login to the online server" << std::endl;
    }


    bWorld.setGravity({.0f, -10.f, .0f});

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
        GameLogic::GetInstance().numbOfBoxesPerPlayers = std::pow(nbOfBoxPerSides, 3);
    }
#endif

#ifdef IS_CLIENT
    GameLogic::GetInstance().SetGameState(GameState::Login);
    /*
    GameLogic::GetInstance().SetGameState(GameState::InGame);

    Player *player = new Player(&scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, 0, 0},
                                        boxInstancesDatas,
                                        drawableGroup, Color3::cyan(), boxShape, 0, playerNum);
    player->SetNetworkId(linking_context->Register(player));
    GameLogic::GetInstance().SetLocalPlayerNetID(linking_context->GetNetworkId(player));
    playerNum++;
    players.push_back(player);
    networkObjects.push_back(player);
    */
#endif
    // Loop at 60 Hz max
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    timeline.start();
    _isRunning = true;
}

void CubeGame::Init(ENetHost *_host) {
    GameLogic::GetInstance().SetHost(_host);
    host = _host;
    Init();
}

void CubeGame::tickEvent() {
    timeline.nextFrame();

    //Listen for packets
    ENetEvent event;
    if (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
#ifdef IS_CLIENT
            {
                char buffer[2048];
                size_t offset = 0;

                NetworkEventType connectionType = NetworkEventType::CONNECTION;
                memcpy(buffer + offset, &connectionType, sizeof(NetworkEventType));
                offset += sizeof(NetworkEventType);

                int uuid = API->GetUserID();
                memcpy(buffer + offset, &uuid, sizeof(int));
                offset += sizeof(uuid);

                ENetPacket *packet = enet_packet_create(buffer, offset, 0);
                enet_peer_send(event.peer, 0, packet);
            }

#endif
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                ReceivePacket(event, event.packet);
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
        if (obj->getMBDrawable())
            drawableGroup.remove(*obj->getMBDrawable());
        destroyedObjects.push_back(obj->GetNetworkId());
        delete obj;
    }

    //Step bullet simulation
    bWorld.stepSimulation(timeline.previousFrameDuration(), 5);

    TakeSnapshot();
#endif

#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() == GameState::LookingForSession)
        _matchmaking->update();

    redraw();
#endif
}

void CubeGame::Shutdown() {
    _isRunning = false;
    delete linking_context;
    delete API;

#ifdef IS_CLIENT
    delete _matchmaking;
    delete _uiRenderer;
#endif

#ifdef IS_SERVER
    nlohmann::json payload = {
        {"id", 0},
        {"ip", "localhost"},
        {"port", 1234},
        {"maxPlayers", 4},
        {"currentPlayers", 0},
        {"isOnline", true},
        {"isOccupied", false},
        {"lastHeartbeat", "2025-04-30T20:22:02.445Z"}
    };
    API->post("/api/server/remove", payload);
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
#ifdef IS_CLIENT
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);
    //Draw the cubes and spheres
    //Repopulate the instance datas with updated transforms and colors
    arrayResize(boxInstancesDatas, 0);
    arrayResize(sphereInstancesDatas, 0);

    Player *player = static_cast<Player *>(linking_context->GetObjectByNetwordId(
        GameLogic::GetInstance().GetLocalPlayerNetID()));

    if (player) {
        player->GetCamera()->draw(drawableGroup);
        shader.setProjectionMatrix(player->GetCamera()->projectionMatrix());
        shader.setTransformationMatrix(player->GetCamera()->cameraMatrix());
    }


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

#endif
}

void CubeGame::TakeSnapshot() {
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

    uint8_t numOjectsToDestroy = destroyedObjects.size();
    memcpy(buffer + offset, &numOjectsToDestroy, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    for (auto netId: destroyedObjects) {
        memcpy(buffer + offset, &netId, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

    ENetPacket *packet = enet_packet_create(buffer, offset, 0);
    enet_host_broadcast(host, 1, packet);
#endif
}

void CubeGame::ReadSnapshot(const uint8_t *data, size_t offset) {
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
                    linking_context->Register(netID, obj);
                    break;
                case NetworkClassID::MBSphere:
                    obj = NetworkObjectFactory::Create(classId, &scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, 0, 0},
                                                       sphereInstancesDatas, drawableGroup,
                                                       Color3::fromHsv({137.5_degf, .75f, .9f}), sphereShape);
                    linking_context->Register(netID, obj);
                    break;
                case NetworkClassID::Player: {
                    obj = NetworkObjectFactory::Create(classId, &scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, 0, 0},
                                                       boxInstancesDatas, drawableGroup,
                                                       Color3::fromHsv({137.5_degf, .75f, .9f}), boxShape, -1, 0);
                    linking_context->Register(netID, obj);
                }

                default:
                    obj = NetworkObjectFactory::Create(classId, &scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, 0, 0},
                                                       boxInstancesDatas, drawableGroup, Color3::blue(), boxShape);
                    linking_context->Register(netID, obj);
                    break;
            }
        }

        obj->DeserializeObject(data, offset);
    }

    uint8_t numObjectsToDestroy;
    std::memcpy(&numObjectsToDestroy, data + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    for (int i = 0; i < numObjectsToDestroy; i++) {
        NetworkId netId;
        std::memcpy(&netId, data + offset, sizeof(NetworkId));
        offset += sizeof(NetworkId);

        MBObject *obj = linking_context->GetObjectByNetwordId(netId);
        linking_context->Unregister(obj);
        delete obj;
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

    NetworkId id = GameLogic::GetInstance().GetLocalPlayerNetID();
    memcpy(buffer + offset, &id, sizeof(NetworkId));
    offset += sizeof(NetworkId);

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

    NetworkId id = GameLogic::GetInstance().GetLocalPlayerNetID();
    memcpy(buffer + offset, &id, sizeof(NetworkId));
    offset += sizeof(NetworkId);

    ENetPacket *packet = enet_packet_create(buffer, offset, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(host, 0, packet);
    enet_host_flush(host);
}

void CubeGame::ReceiveKeyboardInput(const uint8_t *data, size_t offset) {
#ifdef IS_SERVER
    Key inputKey;
    memcpy(&inputKey, data + offset, sizeof(Key));
    offset += sizeof(Key);

    NetworkId playerID;
    memcpy(&playerID, data + offset, sizeof(NetworkId));
    Player *player = static_cast<Player *>(linking_context->GetObjectByNetwordId(playerID));

    if (player) {
        if (inputKey == Key::Down || inputKey == Key::S) {
            player->GetCameraObject()->translate(Vector3({0.f, 0.f, 5.f}));
        } else if (inputKey == Key::Up || inputKey == Key::W) {
            player->GetCameraObject()->translate(Vector3({0.f, 0.f, -5.f}));
        } else if (inputKey == Key::Left || inputKey == Key::A) {
            player->GetCameraObject()->translate(Vector3({-5.f, 0.f, 0.f}));
        } else if (inputKey == Key::Right || inputKey == Key::D) {
            player->GetCameraObject()->translate(Vector3({5.f, 0.f, 0.f}));
        } else if (inputKey == Key::E) {
            player->GetCameraObject()->translate(Vector3({0.f, 5.f, 0.f}));
        } else if (inputKey == Key::Q) {
            player->GetCameraObject()->translate(Vector3({0.f, -5.f, 0.f}));
        }
    }

#endif
}

void CubeGame::ReceiveMouseInput(const uint8_t *data, size_t offset) {
#ifdef IS_SERVER
    Vector2 pos;
    memcpy(&pos, data + offset, sizeof(Vector2));
    offset += sizeof(Vector2);

    NetworkId netId;
    memcpy(&netId, data + offset, sizeof(NetworkId));

    SpawnProjectile(pos, netId);
#endif
}

void CubeGame::ReceivePacket(const ENetEvent event, const ENetPacket *packet) {
    if (packet == nullptr) {
        return;
    }

    size_t offset = 0;
    NetworkEventType packetType;
    std::memcpy(&packetType, packet->data + offset, sizeof(NetworkEventType));
    offset += sizeof(NetworkEventType);

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
        case NetworkEventType::CONNECTION:
#ifdef  IS_SERVER
        {
            int uuid;
            memcpy(&uuid, packet->data + offset, sizeof(int));

            Player *player = new Player(&scene, bWorld, 1.f, {.5f, .5f, .5f}, {0, -1, 0},
                                        boxInstancesDatas,
                                        drawableGroup, Color3::cyan(), boxShape, uuid, playerNum);
            player->SetNetworkId(linking_context->Register(player));

            GameLogic::GetInstance().AddPlayer(uuid);
            playerNum++;
            players.push_back(player);
            networkObjects.push_back(player);

            char msg[2048];
            size_t msgOffset = 0;

            NetworkEventType packetType = NetworkEventType::CONNECTION;
            memcpy(msg + msgOffset, &packetType, sizeof(NetworkEventType));
            msgOffset += sizeof(NetworkEventType);

            NetworkId localId = player->GetNetworkId();
            memcpy(msg + msgOffset, &localId, sizeof(NetworkId));
            msgOffset += sizeof(NetworkId);

            ENetPacket *msg_packet = enet_packet_create(msg, msgOffset, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, msg_packet);
            enet_host_flush(host);
        }

#endif
#ifdef IS_CLIENT
            NetworkId localId;
            memcpy(&localId, packet->data + offset, sizeof(NetworkId));
            GameLogic::GetInstance().SetLocalPlayerNetID(localId);
#endif
            break;
        default:
            break;
    }
    enet_packet_destroy(event.packet);
}

void CubeGame::keyPressEvent(KeyEvent &event) {
#ifdef IS_CLIENT
    if (GameLogic::GetInstance().GetGameState() != GameState::InGame) {
        _imguiContext.handleKeyPressEvent(event);
        event.setAccepted();
        return;
    }

    /*
    auto inputKey = event.key();
    Player *player = static_cast<Player *>(linking_context->GetObjectByNetwordId(GameLogic::GetInstance().GetLocalPlayerNetID()));

    if (inputKey == Key::Up || inputKey == Key::S) {
        player->GetCameraObject()->translate(Vector3({0.f, 0.f, 5.f}));
    } else if (inputKey == Key::Down  || inputKey == Key::W) {
        player->GetCameraObject()->translate(Vector3({0.f, 0.f, -5.f}));
    } else if (inputKey == Key::Right || inputKey == Key::A) {
        player->GetCameraObject()->translate(Vector3({-5.f, 0.f, 0.f}));
    } else if (inputKey == Key::Left || inputKey == Key::D ) {
        player->GetCameraObject()->translate(Vector3({5.f, 0.f, 0.f}));
    } else if (inputKey == Key::E) {
        player->GetCameraObject()->translate(Vector3({0.f, 5.f, 0.f}));
    } else if (inputKey == Key::Q) {
        player->GetCameraObject()->translate(Vector3({0.f, -5.f, 0.f}));
    }
    */
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

auto CubeGame::SpawnProjectile(Vector2 position, NetworkId playerNetId) -> void {
#ifdef IS_SERVER
    Player *player = static_cast<Player *>(linking_context->GetObjectByNetwordId(playerNetId));

    if (player) {
        //Scale the position from relative to the window size to relative to the framebuffer size
        //Since the HiDPI can vary
        const Vector2 scaledPos = position * Vector2{framebufferSize()} / Vector2{windowSize()};
        const Vector2 clickPoint = Vector2::yScale(-1.f) * (scaledPos / Vector2{framebufferSize()} - Vector2{.5f}) *
                                   player->GetCamera()->projectionSize();

        const Vector3 direction = (player->GetCameraObject()->absoluteTransformation().rotationScaling() *
                                   Vector3(clickPoint, -1.f)).normalized();
        const Vector3 spawnPoint = Vector3(player->GetCameraObject()->absoluteTransformation().translation().x() * 2.f,
                                           player->GetCameraObject()->absoluteTransformation().translation().y() * 3.f,
                                           player->GetCameraObject()->absoluteTransformation().translation().z() - 2.f);

        auto *object = new MBSphereObject(&scene, bWorld, 1.f, Vector3{0.5f},
                                          spawnPoint, sphereInstancesDatas,
                                          drawableGroup, 0x221111_rgbf,
                                          sphereShape);

        //Set initial velocity
        auto velocity = direction * 25.f;
        object->getMBRigidBody()->getRigidBody().setLinearVelocity(btVector3(velocity.x(), velocity.y(), velocity.z()));

        object->SetNetworkId(linking_context->Register(object));
        //Add to snapshot
        networkObjects.push_back(object);
    }


#endif
}
