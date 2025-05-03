#ifndef PLAYER_H
#define PLAYER_H
#include "../Object/MBObject.h"
#include "enet6/enet.h"


class Player : public MBObject {
public:
    Player(Object3D *scene, btDynamicsWorld &dynamicsWorld, float mass, Vector3 scale, Vector3 location,
           Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup,
           const Color3 &Color, btCollisionShape &Shape, int id, uint8_t playerNum);

    virtual NetworkClassID GetClassId() const override { return NetworkClassID::Player; }
    int GetOwningPlayerId() const { return _id; }
    void SetOwningPlayerId(int id) { _id = id; }
    uint8_t GetPlayerNum() const { return _playerNum; }
    void SetPlayerNum(uint8_t playerNum) { _playerNum = playerNum; }

    void SerializeObject(char *buffer, size_t &offset) const override;

    void DeserializeObject(const uint8_t *data, size_t &offset) override;

    SceneGraph::Camera3D *GetCamera() const { return _camera; }
    Object3D *GetCameraRig() const { return _cameraRig; }
    Object3D *GetCameraObject() const { return _cameraObject; }

private:
    int _id;
    uint8_t _playerNum;

    SceneGraph::Camera3D *_camera;
    Object3D *_cameraRig, *_cameraObject;

};


#endif //PLAYER_H
