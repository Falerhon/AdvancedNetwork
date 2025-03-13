#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/GlfwApplication.h>

#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Pointer.h"
#include "Magnum/Timeline.h"
#include "Magnum/BulletIntegration/DebugDraw.h"
#include "Magnum/BulletIntegration/MotionState.h"
#include "Magnum/GL/Mesh.h"
#include "Magnum/GL/Renderer.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Matrix4.h"
#include "Magnum/Math/Time.h"
#include "Magnum/MeshTools/Compile.h"
#include "Magnum/Primitives/Axis.h"
#include "Magnum/Primitives/Cube.h"
#include "Magnum/Primitives/UVSphere.h"
#include "Magnum/SceneGraph/Camera.h"
#include "Magnum/SceneGraph/Drawable.h"
#include "Magnum/SceneGraph/FeatureGroup.h"
#include "Magnum/SceneGraph/MatrixTransformation3D.h"
#include "Magnum/SceneGraph/Object.h"
#include "Magnum/SceneGraph/Scene.h"
#include "Magnum/Shaders/PhongGL.h"
#include "Magnum/Trade/MeshData.h"

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

//TODO : Attributes of the instanced objects
struct InstanceData {
    Matrix4 transformationMatrix;
    Matrix3x3 normalMatrix;
    Color3 color;
};

class MyApplication : public Platform::Application {
public:
    explicit MyApplication(const Arguments &arguments);

private:
    void drawEvent() override;

    void keyPressEvent(KeyEvent &event) override;

    void pointerPressEvent(PointerEvent &event) override;

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
    btSphereShape sphereShape{{.25f}};
    btBoxShape groundShape{{8.0f, .5f, 8.0f}};


    //The world has to live longer than the scene so the RigidBody instances can remove themselves
    btDiscreteDynamicsWorld bWorld{&bDispatcher, &bBroadPhase, &bSolver, &bCollisionConfig};

    //Scene config
    Scene3D scene;
    SceneGraph::Camera3D *camera;
    SceneGraph::DrawableGroup3D drawableGroup;
    Timeline timeline;

    Object3D *cameraRig, *cameraObject;

    bool drawObjects{true}, drawDebug{false}, shootBox{true};
};

class RigidBody : public Object3D {
public:
    RigidBody(Object3D *Parent, Float Mass, btCollisionShape *Shape, btDynamicsWorld &World)
        : Object3D(Parent), bWorld(World) {
        //Calculate inertia so the object reacts as it should with the rotation
        btVector3 inertia{0.f, .0f, .0f};
        if (!Math::TypeTraits<Float>::equals(Mass, 0.0f))
            Shape->calculateLocalInertia(Mass, inertia);

        //Set up the RigidBody
        auto *motionState = new BulletIntegration::MotionState{*this};
        bRigidBody.emplace(btRigidBody::btRigidBodyConstructionInfo{
            Mass,
            &motionState->btMotionState(),
            Shape,
            inertia
        });

        //Prevents the physics engine from deactivating the RBs when it stop moving
        bRigidBody->forceActivationState(DISABLE_DEACTIVATION);

        bWorld.addRigidBody(bRigidBody.get());
    }

    ~RigidBody() override {
        bWorld.removeRigidBody(bRigidBody.get());
    }

    btRigidBody &getRigidBody() { return *bRigidBody; }

    void syncPose() {
        bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
    }

private:
    btDynamicsWorld &bWorld;
    Containers::Pointer<btRigidBody> bRigidBody;
};

class ColoredDrawable : public SceneGraph::Drawable3D {
public:
    explicit ColoredDrawable(Object3D &Object, Containers::Array<InstanceData> &InstanceData, const Color3 &Color,
                             const Matrix4 &PrimitiveTransformation, SceneGraph::DrawableGroup3D &DrawableGroup)
        : SceneGraph::Drawable3D{Object, &DrawableGroup}, instanceData(InstanceData), color(Color),
          primitiveTransformation(PrimitiveTransformation) {
    };

private:
    void draw(const Matrix4 &transformation, SceneGraph::Camera3D &) override {
        const Matrix4 t = transformation * primitiveTransformation;
        arrayAppend(instanceData, InPlaceInit, t, t.normalMatrix(), color);
    }

    Containers::Array<InstanceData> &instanceData;
    Color3 color;
    Matrix4 primitiveTransformation;
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

    sphere.addVertexBufferInstanced(boxInstanceBuffer, 1, 0,
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

    //Create the ground
    auto *ground = new RigidBody{&scene, 0.f, &groundShape, bWorld};
    new ColoredDrawable{*ground, boxInstancesDatas, 0xffffff_rgbf, Matrix4::scaling({8.f, .5f, 8.f}), drawableGroup};

    int nbOfBoxPerSides = 5;
    float centerOffset = (nbOfBoxPerSides - 1) / 2.0f;
    //Create boxes with random colors
    Deg boxHue = 42.0_degf;
    for (Int i = 0; i != nbOfBoxPerSides; ++i) {
        for (Int j = 0; j != nbOfBoxPerSides; ++j) {
            for (Int k = 0; k != nbOfBoxPerSides; ++k) {
                auto *o = new RigidBody{&scene, 1.f, &boxShape, bWorld};
                o->translate({i - centerOffset, j + centerOffset, k - centerOffset});
                o->syncPose();

                new ColoredDrawable{
                    *o, boxInstancesDatas, Color3::fromHsv({boxHue += 137.5_degf, .75f, .9f}),
                    Matrix4::scaling(Vector3{.5f}), drawableGroup
                };
            }
        }
    }

    // Loop at 60 Hz max
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    timeline.start();
}

void MyApplication::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    //Remove any object far from the origin
    for (Object3D *obj = scene.children().first(); obj;) {
        Object3D *next = obj->nextSibling();

        if (obj->transformation().translation().dot() > 100 * 100)
            delete obj;

        obj = next;
    }

    //Step bullet simulation
    bWorld.stepSimulation(timeline.previousFrameDuration(), 5);

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

    swapBuffers();
    timeline.nextFrame();
    redraw();
}

void MyApplication::keyPressEvent(KeyEvent &event) {
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
    }

    event.setAccepted();
}

void MyApplication::pointerPressEvent(PointerEvent &event) {
    //Shoot an object on click
    if (!event.isPrimary() || !(event.pointer() & Pointer::MouseLeft))
        return;

    //Scale the position from relative to the window size to relative to the framebuffer size
    //Since the HiDPI can vary
    const Vector2 position = event.position() * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.f) * (position / Vector2{framebufferSize()} - Vector2{.5f}) * camera->
                               projectionSize();
    const Vector3 direction = (cameraObject->absoluteTransformation().rotationScaling() * Vector3(clickPoint, -1.f)).
            normalized();

    auto *object = new RigidBody{
        &scene,
        shootBox ? 1.f : 5.f,
        shootBox ? static_cast<btCollisionShape *>(&boxShape) : &sphereShape,
        bWorld
    };
    object->translate(cameraObject->absoluteTransformation().translation());
    object->syncPose(); //has to be done explicitly (only done implicitly for kinematic objects)

    //Create the projectile
    new ColoredDrawable{
        *object,
        shootBox ? boxInstancesDatas : sphereInstancesDatas,
        shootBox ? 0x880000_rgbf : 0x220000_rgbf,
        Matrix4::scaling(Vector3{shootBox ? .5f : .25f}),
        drawableGroup
    };

    //Set initial velocity
    object->getRigidBody().setLinearVelocity(btVector3{direction * 25.f});

    event.setAccepted();
}

MAGNUM_APPLICATION_MAIN(MyApplication)
