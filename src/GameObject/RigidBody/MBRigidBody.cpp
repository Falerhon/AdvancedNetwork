//
// Created by alex- on 2025-03-22.
//

#include "MBRigidBody.h"

#include <iostream>
#include <ostream>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>

MBRigidBody::MBRigidBody(Object3D *Parent, Float Mass, btCollisionShape *Shape, btDynamicsWorld &World)
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

MBRigidBody::~MBRigidBody() {
    bWorld.removeRigidBody(bRigidBody.get());
}

void MBRigidBody::SetTransform(Vector3 newPosition, Quaternion newRotation, Vector3 newScale) {
    bWorld.removeRigidBody(bRigidBody.get());

    bRigidBody->setGravity(bWorld.getGravity());
    bRigidBody->clearForces();
    bRigidBody->setLinearVelocity(btVector3(0, 0, 0));
    bRigidBody->setAngularVelocity(btVector3(0, 0, 0));

    btTransform t;
    t.setOrigin(btVector3(newPosition.x(), newPosition.y(), newPosition.z()));
    t.setRotation(btQuaternion(newRotation.xyzw().x(), newRotation.xyzw().y(), newRotation.xyzw().z(), newRotation.xyzw().w()));

    bRigidBody->setWorldTransform(t);
    if (bRigidBody->getMotionState()) {
        bRigidBody->getMotionState()->setWorldTransform(t);
    }

    btCollisionShape *shape = bRigidBody->getCollisionShape();
    shape->setLocalScaling(btVector3(newScale.x(), newScale.y(), newScale.z()));

    bRigidBody->setGravity(bWorld.getGravity());
    bWorld.addRigidBody(bRigidBody.get());
    bRigidBody->activate();

}

Vector3 MBRigidBody::GetPosition() {
    btTransform trans;
    bRigidBody->getMotionState()->getWorldTransform(trans);
    btVector3 position = trans.getOrigin();
    return {position.x(), position.y(), position.z()};
}

Quaternion MBRigidBody::GetRotation() {
    btTransform trans;
    bRigidBody->getMotionState()->getWorldTransform(trans);
    btQuaternion rot = trans.getRotation();
    return Quaternion(rot);
}

Vector3 MBRigidBody::GetScale() {
    btVector3 scale = bRigidBody->getCollisionShape()->getLocalScaling();
    return {scale.x(), scale.y(), scale.z()};
}

void MBRigidBody::syncPose() {
    bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
}
