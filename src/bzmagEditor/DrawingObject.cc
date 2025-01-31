#include "DrawingObject.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace bzmag;
using namespace bzmag::engine;

DrawingObject::DrawingObject(Flat3DShader* shader) :
    shader_(shader)
{

}

DrawingObject::~DrawingObject()
{

}


// 원뿔 메쉬 생성 함수
std::pair<std::vector<DrawingObject::Vertex>, std::vector<DrawingObject::Triangle>>
DrawingObject::generateCone(float radius, float height, float offset, unsigned int segments)
{
    std::vector<Vertex> vertices;
    std::vector<Triangle> indices;

    // 원뿔의 밑면 중심점
    vertices.push_back(Vertex{ 0, offset, 0 }); // 밑면의 중심

    // 원뿔의 꼭짓점
    vertices.push_back(Vertex{ 0, height + offset, 0 }); // 꼭짓점

    // 원 둘레의 정점들
    for (unsigned int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back(Vertex{ x, offset, z });
    }

    // 인덱스 생성 (밑면)
    for (unsigned int i = 2; i < segments + 1; ++i) {
        indices.push_back(Triangle{ 0, i, i + 1 });  // 밑면의 중심점
    }
    indices.push_back(Triangle{ 0, segments + 1, 2 });

    // 인덱스 생성 (측면)
    for (unsigned int i = 2; i < segments + 1; ++i) {
        indices.push_back(Triangle{ 1, i, i + 1 });  // 꼭짓점
    }
    indices.push_back(Triangle{ 1,segments + 1,2 });

    return { vertices, indices };
}

// 원기둥 메쉬 생성 함수
std::pair<std::vector<DrawingObject::Vertex>, std::vector<DrawingObject::Triangle>>
DrawingObject::generateCylinder(float radius, float height, float offset, unsigned int segments)
{
    std::vector<Vertex> vertices;
    std::vector<Triangle> indices;

    // 상단, 하단 중심점 추가
    vertices.push_back(Vertex{ 0, height + offset, 0 });  // 상단 중심
    vertices.push_back(Vertex{ 0, 0 + offset, 0 });       // 하단 중심

    // 원기둥 상단 및 하단의 정점들 생성
    for (unsigned int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back(Vertex{ x, height + offset, z }); // 상단 원둘레
        vertices.push_back(Vertex{ x, offset, z });         // 하단 원둘레
    }

    // 인덱스 생성 (상단)
    for (unsigned int i = 2; i < segments * 2; i += 2) {
        indices.push_back(Triangle{ 0, i, i + 2 });  // 상단 중심점
    }
    indices.push_back(Triangle{ 0,segments * 2,2 });

    // 인덱스 생성 (하단)
    for (unsigned int i = 3; i < segments * 2 + 1; i += 2) {
        indices.push_back(Triangle{ 1, i + 2, i });  // 하단 중심점
    }
    indices.push_back(Triangle{ 1,3,segments * 2 + 1 });


    // 인덱스 생성 (측면)
    for (unsigned int i = 2; i < segments * 2; i += 2) {
        indices.push_back(Triangle{ i, i + 1, i + 3 });

        indices.push_back(Triangle{ i, i + 3, i + 2 });
    }
    indices.push_back(Triangle{ segments * 2, segments * 2 + 1, 3 });
    indices.push_back(Triangle{ segments * 2, 3, 2 });

    return { vertices, indices };
}