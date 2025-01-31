#pragma once

#include <QColor>
#include "Flat3DShader.h"
#include "engine/CSNode.h"
#include "core/ref.h"
#include "DrawingObject.h"

class AxisObject : public DrawingObject
{
public:
    AxisObject(bzmag::engine::CSNode* cs, Flat3DShader* shader);
    virtual ~AxisObject();

    const QMatrix4x4& getMatrix() const;
    void draw(QMatrix4x4& transform_matrix, float scale_factor);
    void update();

protected:
    void generateAxis();


protected:
    bzmag::Ref<bzmag::engine::CSNode> cs_;
    QMatrix4x4 coordinate_;
    float axis_length_ = 50.0f;

    std::vector<Vertex> x_nodes_;
    std::vector<Vertex> y_nodes_;
    std::vector<Vertex> z_nodes_;
    std::vector<Triangle> elements_;
};
