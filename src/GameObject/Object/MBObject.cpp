//
// Created by alex- on 2025-03-22.
//

#include "MBObject.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include <btBulletDynamicsCommon.h>
#include <Magnum/BulletIntegration/Integration.h>

using namespace Math::Literals;
using namespace Magnum;

MBObject::MBObject(Object3D *scene, btDynamicsWorld &dynamicsWorld, float mass,Vector3 scale, Vector3 location,
    Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color,
    btCollisionShape &Shape) {

    //Collision shape used for collisions
    _rigidBody = new MBRigidBody{scene, mass, &Shape, dynamicsWorld};

    _rigidBody->translate(location);
    _rigidBody->syncPose();

    _drawableObject = new MBDrawable{
        (*_rigidBody), InstanceData, Color,
        Matrix4::scaling(scale), DrawableGroup
    };
}