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

    Vector3 GetPosition();
    void SetPosition(Vector3 newPosition);

    Quaternion GetRotation();
    void SetRotation(Quaternion newRotation);

    Vector3 GetScale();
    void SetScale(Vector3 newScale);

    void syncPose();

private:
    btDynamicsWorld &bWorld;
    Containers::Pointer<btRigidBody> bRigidBody;
};



#endif //RIGIDBODY_H
