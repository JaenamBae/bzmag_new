#include "AxisObject.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace bzmag;
using namespace bzmag::engine;

AxisObject::AxisObject(CSNode* cs, Flat3DShader* shader) : 
    DrawingObject(shader), cs_(cs)
{
    if (cs == nullptr) return;
    generateAxis();
    update();
}

AxisObject::~AxisObject()
{

}

const QMatrix4x4& AxisObject::getMatrix() const
{
    return coordinate_;
}

void AxisObject::draw(QMatrix4x4& transform_matrix, float scale_factor)
{
    QMatrix4x4 trans;
    trans.scale(axis_length_ / scale_factor);
    shader_->use();
    shader_->setTransformation(transform_matrix, coordinate_ * trans);
    
    shader_->setColor(QColor(255, 0, 0));
    shader_->draw(x_nodes_, elements_);

    shader_->setColor(QColor(0, 255, 0));
    shader_->draw(y_nodes_, elements_);

    shader_->setColor(QColor(0, 0, 255));
    shader_->draw(z_nodes_, elements_);
}

void AxisObject::update()
{
    // QMatrix4x4에 변환 매핑
    QMatrix4x4 matrix;  // 단위행렬임
    if (cs_.valid()) {
        Transformation trans_2d = cs_->transformation();

        // 2D 변환 요소 가져오기
        double m00 = CGAL::to_double(trans_2d.hm(0, 0));    // x 변환 행렬의 첫 번째 행, 첫 번째 열
        double m01 = CGAL::to_double(trans_2d.hm(0, 1));    // x 변환 행렬의 첫 번째 행, 두 번째 열
        double m10 = CGAL::to_double(trans_2d.hm(1, 0));    // y 변환 행렬의 두 번째 행, 첫 번째 열
        double m11 = CGAL::to_double(trans_2d.hm(1, 1));    // y 변환 행렬의 두 번째 행, 두 번째 열
        double tx = CGAL::to_double(trans_2d.hm(0, 2));     // x 방향 변환
        double ty = CGAL::to_double(trans_2d.hm(1, 2));     // y 방향 변환

        matrix.setRow(0, QVector4D(m00, m01, 0.0, tx));     // 첫 번째 행: 2D 변환의 x 행
        matrix.setRow(1, QVector4D(m10, m11, 0.0, ty));     // 두 번째 행: 2D 변환의 y 행
        matrix.setRow(2, QVector4D(0.0, 0.0, 1.0, 0.0));    // z 행 (3D 변환에서 필요)
        matrix.setRow(3, QVector4D(0.0, 0.0, 0.0, 1.0));    // w 행 (동차 좌표)
    }

    coordinate_ = matrix;
}

void AxisObject::generateAxis()
{
    x_nodes_.clear();
    y_nodes_.clear();
    z_nodes_.clear();
    elements_.clear();

    QMatrix4x4 trans_x, trans_z;
    trans_x.rotate(90.0f, 0.0f, 0.0f, -1.0f); // y축을 -z축 기준으로 90도 회전
    trans_z.rotate(90.0f, 1.0f, 0.0f,  0.0f); // y축을  x축 기준으로 90도 회전

    std::pair<std::vector<Vertex>, std::vector<Triangle>> cone = generateCone(0.05f, 0.2f, 0.8f, 16);
    std::pair<std::vector<Vertex>, std::vector<Triangle>> cylinder = generateCylinder(0.015f, 0.8f, 0.0f, 16);
    for (auto& vert1 : cone.first) {
        y_nodes_.push_back(vert1);

        QVector4D node_conv_x = trans_x * (QVector4D{ vert1[0], vert1[1], vert1[2], 1.0f });
        x_nodes_.push_back(Vertex{ node_conv_x.x(), node_conv_x.y(), node_conv_x.z() });

        QVector4D node_conv_z = trans_z * (QVector4D{ vert1[0], vert1[1], vert1[2], 1.0f });
        z_nodes_.push_back(Vertex{ node_conv_z.x(), node_conv_z.y(), node_conv_z.z() });
    }
    for (auto& tri1 : cone.second) {
        elements_.push_back(tri1);
    }
    
    for (auto& vert2 : cylinder.first) {
        y_nodes_.push_back(vert2);

        QVector4D node_conv_x = trans_x * (QVector4D{ vert2[0], vert2[1], vert2[2], 1.0f });
        x_nodes_.push_back(Vertex{ node_conv_x.x(), node_conv_x.y(), node_conv_x.z() });

        QVector4D node_conv_z = trans_z * (QVector4D{ vert2[0], vert2[1], vert2[2], 1.0f });
        z_nodes_.push_back(Vertex{ node_conv_z.x(), node_conv_z.y(), node_conv_z.z() });
    }
    unsigned int offset = (unsigned int)cone.first.size();
    for (auto& tri2 : cylinder.second) {
        tri2[0] += offset;
        tri2[1] += offset;
        tri2[2] += offset;
        elements_.push_back(tri2);
    }
}
