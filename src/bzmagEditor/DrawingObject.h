#pragma once

#include <QColor>
#include "Flat3DShader.h"
#include "engine/GeomHeadNode.h"
#include "engine/CSNode.h"
#include "engine/GeomToSurfaceMesh.h"

class DrawingObject
{
public:
    using Vertex = std::array<float, 3>;
    using Triangle = std::array<unsigned int, 3>;
    using Segment = std::array<unsigned int, 2>;

public:
    DrawingObject(Flat3DShader* shader);
    virtual ~DrawingObject();

    QVector3D getMinBounds() const { return min_bound_; }
    QVector3D getMaxBounds() const { return max_bound_; }


public:
    static std::pair<std::vector<Vertex>, std::vector<Triangle>>
        generateCone(float radius, float height, float offset, unsigned int segments);
    static std::pair<std::vector<Vertex>, std::vector<Triangle>>
        generateCylinder(float radius, float height, float offset, unsigned int segments);

protected:
    Flat3DShader* shader_;
    QVector3D min_bound_;
    QVector3D max_bound_;
};
