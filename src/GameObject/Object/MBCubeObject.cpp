//
// Created by alex- on 2025-03-22.
//

#include "MBCubeObject.h"

#include "../../GameLogic/GameLogic.h"

using namespace Math::Literals;

MBCubeObject::MBCubeObject(Object3D* scene, btDynamicsWorld &world, float mass, Vector3 scale, Vector3 location,
    Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btCollisionShape &Shape, int owner)
    : MBObject(scene, world, mass, scale, location, InstanceData, DrawableGroup, Color, Shape){
    owningPlayer = owner;
}

MBCubeObject::~MBCubeObject() {

    GameLogic::GetInstance().CubeDestroyed(owningPlayer);

    MBObject::~MBObject();
}
