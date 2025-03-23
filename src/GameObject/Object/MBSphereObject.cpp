//
// Created by alex- on 2025-03-23.
//

#include "MBSphereObject.h"

MBSphereObject::MBSphereObject(Object3D *scene, btDynamicsWorld &world, float mass, Vector3 scale, Vector3 location,
    Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color,
    btSphereShape &Shape) : MBObject(scene, world, mass, scale, location, InstanceData, DrawableGroup, Color, Shape){
}
