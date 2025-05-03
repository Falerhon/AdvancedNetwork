#include "Player.h"

#include "Magnum/GL/DefaultFramebuffer.h"
#include "Magnum/SceneGraph/Camera.h"

Player::Player(Object3D *scene, btDynamicsWorld &dynamicsWorld, float mass, Vector3 scale, Vector3 location,
               Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup,
               const Color3 &Color, btCollisionShape &Shape, int id, uint8_t playerNum): MBObject(scene, dynamicsWorld,
    mass, scale, location, InstanceData, DrawableGroup, Color, Shape), _id(id), _playerNum(playerNum) {
    _cameraRig = new Object3D{scene};

    _cameraObject = new Object3D{_cameraRig};
    _cameraObject->translate(Vector3::zAxis(-5.f));
    _cameraObject->translate(Vector3::xAxis(5.f));
    _cameraObject->translate(Vector3::yAxis(1.f));
    //_cameraObject->rotateXLocal(-25.0_degf);

    /*Vector3 cameraTarget = _cameraObject->absoluteTransformation().translation() +
                           Vector3(-_playerNum * 10.f, 0.f, 0.f);
    Vector3 upDirection(0.0f, 1.0f, 0.0f);
    _cameraObject->setTransformation(Matrix4::translation(_cameraObject->absoluteTransformation().translation())
                                     * Matrix4::lookAt(_cameraObject->absoluteTransformation().translation(),
                                                       cameraTarget, upDirection));*/

    _camera = new SceneGraph::Camera3D(*_cameraObject);
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
            .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.1f, 100.0f))
            .setViewport(GL::defaultFramebuffer.viewport().size());
}

void Player::SerializeObject(char *buffer, size_t &offset) const {
    //********* Write NetworkID *********//
    uint32_t netID = GetNetworkId();
    memcpy(buffer + offset, &netID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    //********* Write ClassID *********//
    NetworkClassID classID = GetClassId();
    memcpy(buffer + offset, &classID, sizeof(NetworkClassID));
    offset += sizeof(NetworkClassID);

    Vector3 cameraPosition = _cameraObject->absoluteTransformationMatrix().translation();
    int32_t px = cameraPosition.x() * 100;
    int32_t py = cameraPosition.y() * 100;
    int32_t pz = cameraPosition.z() * 100;
    memcpy(buffer + offset, &px, sizeof(int32_t));
    offset += sizeof(int32_t);
    memcpy(buffer + offset, &py, sizeof(int32_t));
    offset += sizeof(int32_t);
    memcpy(buffer + offset, &pz, sizeof(int32_t));
    offset += sizeof(int32_t);

    memcpy(buffer + offset, &_id, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &_playerNum, sizeof(uint8_t));
    offset += sizeof(uint8_t);
}

void Player::DeserializeObject(const uint8_t *data, size_t &offset) {
    int32_t px;
    int32_t py;
    int32_t pz;
    memcpy(&px, data + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    memcpy(&py, data + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    memcpy(&pz, data + offset, sizeof(int32_t));
    offset += sizeof(int32_t);

    memcpy(&_id, data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&_playerNum, data + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    float playerOffset = _playerNum * 20.f;


    Vector3 cameraPosition(px / 100.0f, py / 100.0f, pz / 100.0f);
    Vector3 cameraTarget = cameraPosition + Vector3(-_playerNum * 10.f, 0.f, 0.f);
    Vector3 upDirection(0.0f, 1.0f, 0.0f);
    _cameraObject->setTransformation(Matrix4::translation(cameraPosition)
                                     * Matrix4::lookAt(cameraPosition, cameraTarget, upDirection));
}
