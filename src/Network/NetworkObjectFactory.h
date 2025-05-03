#ifndef NETWORKOBJECTFACTORY_H
#define NETWORKOBJECTFACTORY_H

#include "../GameObject/Object/MBCubeObject.h"
#include "../GameObject/Object/MBSphereObject.h"
#include "../GameObject/Player/Player.h"


class NetworkObjectFactory {
public:
    static MBObject *Create(NetworkClassID classId, Object3D *scene, btDynamicsWorld &world, float mass, Vector3 scale,
                            Vector3 location,
                            Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup,
                            const Color3 &Color, btCollisionShape &Shape, int id = -1, uint8_t playerNum = -1) {
        switch (classId) {
            case NetworkClassID::MBCube:
                return new MBCubeObject(scene, world, mass, scale, location, InstanceData, DrawableGroup,
                                        Color, Shape);
            case NetworkClassID::MBSphere:
                return new MBSphereObject(scene, world, mass, scale, location, InstanceData, DrawableGroup,
                                          Color, dynamic_cast<btSphereShape &>(Shape));
            case NetworkClassID::Player:
                return new Player(scene, world, mass, scale, location, InstanceData, DrawableGroup, Color, Shape, id,
                                  playerNum);
            default:
                return nullptr;
        }
    }
};


#endif //NETWORKOBJECTFACTORY_H
