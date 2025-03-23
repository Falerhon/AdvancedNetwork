//
// Created by alex- on 2025-03-22.
//

#ifndef OBJECT_H
#define OBJECT_H
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "Magnum/SceneGraph/MatrixTransformation3D.h"
#include <Magnum/SceneGraph/SceneGraph.h>
#include "Magnum/Math/Matrix4.h"

#include "../Drawable/MBDrawable.h"
#include "../RigidBody/MBRigidBody.h"

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;

class MBObject {
public:
    MBObject(Object3D* scene, btDynamicsWorld& dynamicsWorld, float mass,Vector3 scale, Vector3 location, Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btCollisionShape &Shape);

    virtual ~MBObject() = default;

    MBRigidBody* getMBRigidBody() const {return _rigidBody;};

protected:
    MBDrawable* _drawableObject;
    MBRigidBody *_rigidBody;
};



#endif //OBJECT_H