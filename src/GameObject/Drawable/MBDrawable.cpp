//
// Created by alex- on 2025-03-22.
//

#include "MBDrawable.h"

#include <iostream>

#include "GameObject/Object/MBObject.h"
#include "LinearMath/btTransform.h"
#include "Magnum/Math/Quaternion.h"

void MBDrawable::draw(const Matrix4 &transformation, SceneGraph::Camera3D &) {
    const Matrix4 t = transformation * primitiveTransformation;
    arrayAppend(instanceData, InPlaceInit, t, t.normalMatrix(), color);
}
