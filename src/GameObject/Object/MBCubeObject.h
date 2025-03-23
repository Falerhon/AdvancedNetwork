﻿//
// Created by alex- on 2025-03-22.
//

#ifndef CUBEOBJECT_H
#define CUBEOBJECT_H
#include "MBObject.h"
#include "Magnum/GL/Mesh.h"

class MBCubeObject : public MBObject{
public:
    MBCubeObject(Object3D* scene, btDynamicsWorld &world, float mass, Vector3 scale, Vector3 location,
        Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btCollisionShape &Shape);

};



#endif //CUBEOBJECT_H
