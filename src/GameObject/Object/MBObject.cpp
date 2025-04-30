//
// Created by alex- on 2025-03-22.
//

#include "MBObject.h"

#include <bitset>
#include <btBulletDynamicsCommon.h>
#include <fstream>
#include <iostream>
#include <Magnum/BulletIntegration/Integration.h>

using namespace Math::Literals;
using namespace Magnum;

#define PRECISIONMULTIPLIER 100

MBObject::MBObject(Object3D *scene, btDynamicsWorld &dynamicsWorld, float mass, Vector3 scale, Vector3 location,
                   Containers::Array<InstanceData> &InstanceData, SceneGraph::DrawableGroup3D &DrawableGroup,
                   const Color3 &Color,
                   btCollisionShape &Shape) {
    //Collision shape used for collisions
    _rigidBody = new MBRigidBody{scene, mass, &Shape, dynamicsWorld};

    _rigidBody->translate(location);
    _rigidBody->syncPose();

    _drawableObject = new MBDrawable{
        (*_rigidBody), InstanceData, Color,
        Matrix4::scaling(scale), DrawableGroup
    };
}

void MBObject::SerializeObject(char *buffer, size_t &offset) const {
    auto write = [&](const void *data, size_t size) {
        memcpy(buffer + offset, data, size);
        offset += size;
    };

    //********* Write NetworkID *********//
    uint32_t netID = GetNetworkId();
    write(&netID, sizeof(netID));

    //********* Write ClassID *********//
    NetworkClassID classID = GetClassId();
    write(&classID, sizeof(classID));

    //********* Write position *********//
    Vector3 position = _rigidBody->GetPosition();
    int32_t pX = position.x() * PRECISIONMULTIPLIER;
    int32_t pY = position.y() * PRECISIONMULTIPLIER;
    int32_t pZ = position.z() * PRECISIONMULTIPLIER;
    
    write(&pX, sizeof(pX));
    write(&pY, sizeof(pY));
    write(&pZ, sizeof(pZ));

    //********* Write scale *********//
    Vector3 scale = _rigidBody->GetScale();
    int16_t sX = scale.x() * PRECISIONMULTIPLIER;
    int16_t sY = scale.y() * PRECISIONMULTIPLIER;
    int16_t sZ = scale.z() * PRECISIONMULTIPLIER;
    write(&sX, sizeof(sX));
    write(&sY, sizeof(sY));
    write(&sZ, sizeof(sZ));

    //********* Write rotation *********//
    Quaternion rotation = _rigidBody->GetRotation();
    QuaternionFloat quat = {rotation.xyzw().w(), rotation.xyzw().y(), rotation.xyzw().z(), rotation.xyzw().w()};
    uint8_t maxIndex = 99;
    float maxValue = std::numeric_limits<float>::min();
    int sign = 1;

    float quatElements[4] = {rotation.xyzw().x(), rotation.xyzw().y(), rotation.xyzw().z(), rotation.xyzw().w()};
    //Get the largest element
    for (int i = 0; i < 4; i++) {
        float element = quatElements[i];
        float absValue = abs(element);
        if (absValue > maxValue) {
            sign = (element < 0) ? -1 : 1;

            maxIndex = i;
            maxValue = absValue;
        }
    }

    uint16_t a;
    uint16_t b;
    uint16_t c;

    switch (maxIndex) {
        case 0:
            a = (uint16_t) (quat.y * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.z * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.w * sign * PRECISIONMULTIPLIER);
            break;
        case 1:
            a = (uint16_t) (quat.x * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.z * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.w * sign * PRECISIONMULTIPLIER);
            break;
        case 2:
            a = (uint16_t) (quat.x * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.y * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.w * sign * PRECISIONMULTIPLIER);
            break;
        default:
            a = (uint16_t) (quat.x * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.y * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.z * sign * PRECISIONMULTIPLIER);
    }
    write(&maxIndex, sizeof(maxIndex));
    write(&a, sizeof(a));
    write(&b, sizeof(b));
    write(&c, sizeof(c));

    //********* Write color *********//
    Color3 drawableColor = _drawableObject->getColor();
    uint32_t color = drawableColor.toSrgbInt();
    write(&color, sizeof(color));
}

void MBObject::DeserializeObject(const uint8_t *data, size_t &offset) {
    auto read = [&](void *dest, size_t size) {
        std::memcpy(dest, data + offset, size);
        offset += size;
    };

    //********* Read position *********//
    int32_t pX, pY, pZ;
    read(&pX, sizeof(pX));
    read(&pY, sizeof(pY));
    read(&pZ, sizeof(pZ));
    float posX = static_cast<float>(pX) / PRECISIONMULTIPLIER;
    float posY = static_cast<float>(pY) / PRECISIONMULTIPLIER;
    float posZ = static_cast<float>(pZ) / PRECISIONMULTIPLIER;

    //********* Read scale *********//
    int16_t sX, sY, sZ;
    read(&sX, sizeof(sX));
    read(&sY, sizeof(sY));
    read(&sZ, sizeof(sZ));
    float scaleX = static_cast<float>(sX) / PRECISIONMULTIPLIER;
    float scaleY = static_cast<float>(sY) / PRECISIONMULTIPLIER;
    float scaleZ = static_cast<float>(sZ) / PRECISIONMULTIPLIER;

    //********* Read rotation *********//
    uint8_t maxIndex;
    uint16_t a, b, c;
    read(&maxIndex, sizeof(maxIndex));
    read(&a, sizeof(a));
    read(&b, sizeof(b));
    read(&c, sizeof(c));

    float x = static_cast<float>(a) / PRECISIONMULTIPLIER;
    float y = static_cast<float>(b) / PRECISIONMULTIPLIER;
    float z = static_cast<float>(c) / PRECISIONMULTIPLIER;
    float w = std::sqrt(1.0f - (x * x + y * y + z * z));

    //********* Read color *********//
    uint32_t color;
    read(&color, sizeof(color));

    //********* Set values *********//
    QuaternionFloat quat;
    if (maxIndex == 0)
        quat = QuaternionFloat(w, x, y, z);
    else if (maxIndex == 1)
        quat = QuaternionFloat(x, w, y, z);
    else if (maxIndex == 2)
        quat = QuaternionFloat(x, y, w, z);
    else
        quat = QuaternionFloat(x, y, z, w);

    Quaternion translatedQuat;
    translatedQuat.xyzw().x() = quat.x;
    translatedQuat.xyzw().y() = quat.y;
    translatedQuat.xyzw().z() = quat.z;
    translatedQuat.xyzw().w() = quat.w;

    _rigidBody->SetTransform(Vector3(posX, posY, posZ), translatedQuat, Vector3(scaleX, scaleY, scaleZ));

    Color3 linearColor = Color3::fromSrgb(color);
    _drawableObject->SetColor(linearColor);
}

void MBObject::SerializeObjectToBinary(std::ofstream &outStream) const {
    //********* Write NetworkID *********//
    uint32_t netID = GetNetworkId();
    outStream.write(reinterpret_cast<const char *>(&netID), sizeof(uint32_t));

    //********* Write ClassID *********//
    NetworkClassID classID = GetClassId();
    outStream.write(reinterpret_cast<const char *>(&classID), sizeof(NetworkClassID));

    //********* Write position *********//
    Vector3 position = _rigidBody->GetPosition();
    int32_t pX = position.x() * PRECISIONMULTIPLIER;
    int32_t pY = position.y() * PRECISIONMULTIPLIER;
    int32_t pZ = position.z() * PRECISIONMULTIPLIER;

    outStream.write(reinterpret_cast<const char *>(&pX), sizeof(int32_t));
    outStream.write(reinterpret_cast<const char *>(&pY), sizeof(int32_t));
    outStream.write(reinterpret_cast<const char *>(&pZ), sizeof(int32_t));

    //********* Write scale *********//
    Vector3 scale = _rigidBody->GetScale();
    int16_t sX = scale.x() * PRECISIONMULTIPLIER;
    int16_t sY = scale.y() * PRECISIONMULTIPLIER;
    int16_t sZ = scale.z() * PRECISIONMULTIPLIER;
    outStream.write(reinterpret_cast<const char *>(&sX), sizeof(int16_t));
    outStream.write(reinterpret_cast<const char *>(&sY), sizeof(int16_t));
    outStream.write(reinterpret_cast<const char *>(&sZ), sizeof(int16_t));

    //********* Write rotation *********//
    Quaternion rotation = _rigidBody->GetRotation();
    QuaternionFloat quat = {rotation.xyzw().w(), rotation.xyzw().y(), rotation.xyzw().z(), rotation.xyzw().w()};
    uint8_t maxIndex = 99;
    float maxValue = std::numeric_limits<float>::min();
    int sign = 1;

    float quatElements[4] = {rotation.xyzw().x(), rotation.xyzw().y(), rotation.xyzw().z(), rotation.xyzw().w()};
    //Get the largest element
    for (int i = 0; i < 4; i++) {
        float element = quatElements[i];
        float absValue = abs(element);
        if (absValue > maxValue) {
            sign = (element < 0) ? -1 : 1;

            maxIndex = i;
            maxValue = absValue;
        }
    }

    uint16_t a;
    uint16_t b;
    uint16_t c;

    switch (maxIndex) {
        case 0:
            a = (uint16_t) (quat.y * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.z * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.w * sign * PRECISIONMULTIPLIER);
            break;
        case 1:
            a = (uint16_t) (quat.x * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.z * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.w * sign * PRECISIONMULTIPLIER);
            break;
        case 2:
            a = (uint16_t) (quat.x * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.y * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.w * sign * PRECISIONMULTIPLIER);
            break;
        default:
            a = (uint16_t) (quat.x * sign * PRECISIONMULTIPLIER);
            b = (uint16_t) (quat.y * sign * PRECISIONMULTIPLIER);
            c = (uint16_t) (quat.z * sign * PRECISIONMULTIPLIER);
    }
    outStream.write(reinterpret_cast<const char *>(&maxIndex), sizeof(uint8_t));
    outStream.write(reinterpret_cast<const char *>(&a), sizeof(uint16_t));
    outStream.write(reinterpret_cast<const char *>(&b), sizeof(uint16_t));
    outStream.write(reinterpret_cast<const char *>(&c), sizeof(uint16_t));


    //********* Write color *********//
    Color3 drawableColor = _drawableObject->getColor();
    uint32_t color = drawableColor.toSrgbInt();
    outStream.write(reinterpret_cast<const char *>(&color), sizeof(uint32_t));
}

void MBObject::DeserializeObjectToBinary(std::ifstream &inStream) {
    //NetworkID
    uint32_t netID;
    inStream.read(reinterpret_cast<char *>(&netID), sizeof(uint32_t));

    //ClassID
    NetworkClassID classID;
    inStream.read(reinterpret_cast<char *>(&classID), sizeof(NetworkClassID));

    //Position
    int32_t pX;
    int32_t pY;
    int32_t pZ;
    inStream.read(reinterpret_cast<char *>(&pX), sizeof(int32_t));
    inStream.read(reinterpret_cast<char *>(&pY), sizeof(int32_t));
    inStream.read(reinterpret_cast<char *>(&pZ), sizeof(int32_t));
    float posX = float(pX) / PRECISIONMULTIPLIER;
    float posY = float(pY) / PRECISIONMULTIPLIER;
    float posZ = float(pZ) / PRECISIONMULTIPLIER;

    //Scale
    int16_t sX;
    int16_t sY;
    int16_t sZ;
    inStream.read(reinterpret_cast<char *>(&sX), sizeof(int16_t));
    inStream.read(reinterpret_cast<char *>(&sY), sizeof(int16_t));
    inStream.read(reinterpret_cast<char *>(&sZ), sizeof(int16_t));
    float scaleX = float(sX) / PRECISIONMULTIPLIER;
    float scaleY = float(sY) / PRECISIONMULTIPLIER;
    float scaleZ = float(sZ) / PRECISIONMULTIPLIER;

    //Rotation
    uint8_t maxIndex;
    uint16_t a, b, c;
    inStream.read(reinterpret_cast<char *>(&maxIndex), sizeof(int8_t));
    inStream.read(reinterpret_cast<char *>(&a), sizeof(uint16_t));
    inStream.read(reinterpret_cast<char *>(&b), sizeof(uint16_t));
    inStream.read(reinterpret_cast<char *>(&c), sizeof(uint16_t));

    // Read the other three fields and derive the value of the omitted field
    float x = float(a) / PRECISIONMULTIPLIER;
    float y = float(b) / PRECISIONMULTIPLIER;
    float z = float(c) / PRECISIONMULTIPLIER;
    float w = std::sqrt(1 - (x * x + y * y + z * z));

    QuaternionFloat quat;
    if (maxIndex == 0)
        quat = QuaternionFloat(w, x, y, z);
    else if (maxIndex == 1)
        quat = QuaternionFloat(x, w, y, z);
    else if (maxIndex == 2)
        quat = QuaternionFloat(x, y, w, z);
    else
        quat = QuaternionFloat(x, y, z, w);

    uint32_t color;
    inStream.read(reinterpret_cast<char *>(&color), sizeof(uint32_t));

    Quaternion translatedQuat;
    translatedQuat.xyzw().x() = quat.x;
    translatedQuat.xyzw().y() = quat.y;
    translatedQuat.xyzw().z() = quat.z;
    translatedQuat.xyzw().w() = quat.w;

    _rigidBody->SetTransform(Vector3(posX, posY, posZ), translatedQuat, Vector3(scaleX, scaleY, scaleZ));

    Color3 linearColor = Color3::fromSrgb(color);
    _drawableObject->SetColor(linearColor);
}

QuaternionFloat MBObject::MatrixToQuat(Matrix3x3 m) {
    QuaternionFloat q;

    float trace = m[0][0] + m[1][1] + m[2][2];

    if (trace > 0.0f) {
        float s = sqrtf(trace + 1.0f) * 2.f;
        q.w = 0.25f * s;
        q.x = (m[2][1] - m[1][2]) / s;
        q.y = (m[0][2] - m[2][0]) / s;
        q.z = (m[1][0] - m[0][1]) / s;
    } else {
        if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
            float s = sqrtf(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f;
            q.w = (m[2][1] - m[1][2]) / s;
            q.x = 0.25f * s;
            q.y = (m[0][1] + m[1][0]) / s;
            q.z = (m[0][2] + m[2][0]) / s;
        } else if (m[1][1] > m[2][2]) {
            float s = sqrtf(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f;
            q.w = (m[0][2] - m[2][0]) / s;
            q.x = (m[0][1] + m[1][0]) / s;
            q.y = 0.25f * s;
            q.z = (m[1][2] + m[2][1]) / s;
        } else {
            float s = sqrtf(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.0f;
            q.w = (m[1][0] - m[0][1]) / s;
            q.x = (m[0][2] + m[2][0]) / s;
            q.y = (m[1][2] + m[2][1]) / s;
            q.z = 0.25f * s;
        }
    }

    return q;
}

Matrix3x3 MBObject::QuatToMatrix(QuaternionFloat q) const {
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    Matrix3x3 RotMatrix;
    RotMatrix[0][0] = 1.0f - 2.0f * (yy + zz);
    RotMatrix[0][1] = 2.0f * (xy - wz);
    RotMatrix[0][2] = 2.0f * (xz + wy);

    RotMatrix[1][0] = 2.0f * (xy + wz);
    RotMatrix[1][1] = 1.0f - 2.0f * (xx + zz);
    RotMatrix[1][2] = 2.0f * (yz - wx);

    RotMatrix[2][0] = 2.0f * (xz - wy);
    RotMatrix[2][1] = 2.0f * (yz + wx);
    RotMatrix[2][2] = 1.0f - 2.0f * (xx + yy);

    return RotMatrix;
}
