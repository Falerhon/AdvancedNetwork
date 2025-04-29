//
// Created by alex- on 2025-03-22.
//

#ifndef OBJECT_H
#define OBJECT_H
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "Magnum/SceneGraph/MatrixTransformation3D.h"
#include <Magnum/SceneGraph/SceneGraph.h>

#include "../Drawable/MBDrawable.h"
#include "../RigidBody/MBRigidBody.h"

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;

enum class NetworkClassID : uint8_t {
    INVALID = 0,
    MBCube,
    MBSphere
};

struct QuaternionFloat {
    float x,y,z,w;

    QuaternionFloat(): x(0), y(0), z(0), w(0) {
    }

    QuaternionFloat(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
};

class MBObject {
public:
    MBObject(Object3D* scene, btDynamicsWorld& dynamicsWorld, float mass,Vector3 scale, Vector3 location, Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup, const Color3 &Color, btCollisionShape &Shape);

    virtual ~MBObject() = default;

    virtual NetworkClassID GetClassId() const = 0;

    virtual void SerializeObject(char* buffer, size_t& offset) const;
    virtual void DeserializeObject(const uint8_t* data, size_t& offset);

    virtual void SerializeObjectToBinary(std::ofstream& outStream) const;
    virtual void DeserializeObjectToBinary(std::ifstream& inStream);

    MBRigidBody* getMBRigidBody() const {return _rigidBody; }
    MBDrawable* getMBDrawable() const {return _drawableObject; }

    void SetNetworkId(uint32_t networkID) { _networkID = networkID; }
    uint32_t GetNetworkId() const { return _networkID; }

protected:
    MBDrawable* _drawableObject;
    MBRigidBody *_rigidBody;

private:
    static QuaternionFloat MatrixToQuat(Matrix3x3 matrix);
    Matrix3x3 QuatToMatrix(QuaternionFloat quat) const;

    uint32_t _networkID;
};



#endif //OBJECT_H