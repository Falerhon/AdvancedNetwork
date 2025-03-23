//
// Created by alex- on 2025-03-23.
//

#ifndef MBSPHEREOBJECT_H
#define MBSPHEREOBJECT_H

#include "MBObject.h"
#include "Magnum/GL/Mesh.h"

class MBSphereObject : public MBObject{
    public:
    MBSphereObject(Object3D* scene, btDynamicsWorld& world, float mass, Vector3 scale, Vector3 location, Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btSphereShape &Shape);
};

#endif //MBSPHEREOBJECT_H
