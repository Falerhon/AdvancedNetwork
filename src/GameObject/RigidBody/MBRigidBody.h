//
// Created by alex- on 2025-03-22.
//

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <btBulletDynamicsCommon.h>

#include <Corrade/Containers/Pointer.h>

#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/SceneGraph.h>

class btDynamicsWorld;
class btCollisionShape;
using namespace Magnum;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;

//TODO : REMOVE PARENT

class MBRigidBody : public Object3D {
public:
    MBRigidBody(Object3D *Parent, Float Mass, btCollisionShape *Shape, btDynamicsWorld &World);

    ~MBRigidBody() override;

    btRigidBody &getRigidBody() { return *bRigidBody; }

    void SetTransform(Vector3 newPosition, Quaternion newRotation, Vector3 newScale);

    Vector3 GetPosition();

    Quaternion GetRotation();

    Vector3 GetScale();

    void syncPose();

private:
    btDynamicsWorld &bWorld;
    Containers::Pointer<btRigidBody> bRigidBody;
};



#endif //RIGIDBODY_H
