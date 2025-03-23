//
// Created by alex- on 2025-03-22.
//

#ifndef CUBEOBJECT_H
#define CUBEOBJECT_H
#include "MBObject.h"
#include "Magnum/GL/Mesh.h"

class MBCubeObject : public MBObject{
public:
    MBCubeObject(Scene3D& scene, btDynamicsWorld& world, float mass, Vector3 scale, Vector3 location, Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btBoxShape groundShape);
    void update(float dt) override;
    void draw(const Matrix4& projectionMatrix, SceneGraph::Camera3D& camera) override;

};



#endif //CUBEOBJECT_H
