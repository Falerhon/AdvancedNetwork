//
// Created by alex- on 2025-03-22.
//

#include "MBObject.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include <btBulletDynamicsCommon.h>
#include <Magnum/BulletIntegration/Integration.h>

using namespace Math::Literals;
using namespace Magnum;

MBObject::MBObject(Scene3D &scene, btDynamicsWorld &dynamicsWorld, float mass, Vector3 scale, Vector3 location) {
    _drawableObject = nullptr;
    _rigidBody = nullptr;
}

void MBObject::update(float dt) {
}
