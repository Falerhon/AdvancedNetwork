//
// Created by alex- on 2025-03-22.
//

#include "MBCubeObject.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "LinearMath/btDefaultMotionState.h"
#include "Magnum/SceneGraph/Scene.h"

using namespace Math::Literals;

MBCubeObject::MBCubeObject(Scene3D &scene, btDynamicsWorld &world, float mass, Vector3 scale, Vector3 location,
    Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btBoxShape groundShape)
    : MBObject(scene, world, mass, scale, location){

    float x = scale.x();
    float y = scale.y();
    float z = scale.z();

    btBoxShape boxShape{btVector3(x, y, z)}; //Collision shape used for collisions
    _rigidBody = new MBRigidBody{&scene, mass, &groundShape, world};

    //_rigidBody->translate(location);
    //_rigidBody->syncPose();

    _drawableObject = new MBDrawable{
        (*_rigidBody), InstanceData, Color,
        Matrix4::scaling(scale), DrawableGroup
    };
}

void MBCubeObject::update(float dt) {
    MBObject::update(dt);
}

void MBCubeObject::draw(const Matrix4 &projectionMatrix, SceneGraph::Camera3D &camera) {
}
