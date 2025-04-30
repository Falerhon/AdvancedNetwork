//
// Created by alex- on 2025-03-22.
//

#ifndef MBDRAWABLE_H
#define MBDRAWABLE_H


#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/SceneGraph.h>

#include "Corrade/Containers/GrowableArray.h"
#include "Magnum/Math/Color.h"
#include "Magnum/SceneGraph/Drawable.h"

class MBObject;
using namespace Magnum;

struct InstanceData {
    Matrix4 transformationMatrix;
    Matrix3x3 normalMatrix;
    Color3 color;
};

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;


class MBDrawable : public SceneGraph::Drawable3D {
public:
    explicit MBDrawable(Object3D &Object, Containers::Array<InstanceData> &InstanceData, const Color3 &Color,
                        const Matrix4 &PrimitiveTransformation, SceneGraph::DrawableGroup3D &DrawableGroup)
        : SceneGraph::Drawable3D{Object, &DrawableGroup}, instanceData(InstanceData), color(Color),
          primitiveTransformation(PrimitiveTransformation) {
    };

    Color3 getColor() const { return color; }
    void SetColor(Color3 color) { this->color = color; }

private:
    void draw(const Matrix4 &transformation, SceneGraph::Camera3D &) override;

    Containers::Array<InstanceData> &instanceData;
    Color3 color;
    Matrix4 primitiveTransformation;
};


#endif //MBDRAWABLE_H
