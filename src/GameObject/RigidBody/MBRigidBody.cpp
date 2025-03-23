//
// Created by alex- on 2025-03-22.
//

#include "MBRigidBody.h"

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

void MBRigidBody::syncPose() {
    bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
}
