#include "GeomObject.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace bzmag;
using namespace bzmag::engine;

GeomObject::GeomObject(GeomHeadNode* head, Flat3DShader* shader) : 
    DrawingObject(shader), head_(head), id_(0), selected_(false)
{
    if (head == nullptr) return;
    id_ = head->getID();
    update();
}

GeomObject::~GeomObject()
{
}

GeomHeadNode* GeomObject::getHead()
{
    return head_; 
}

unsigned int GeomObject::getID() const
{
    return id_;
}

void GeomObject::setColor(const QColor& color)
{
    color_ = color;
}

void GeomObject::setSelected(bool selected)
{
    selected_ = selected;
}

bool GeomObject::isSelected() const
{
    return selected_;
}

void GeomObject::setReferedMatrix(const QMatrix4x4& matrix)
{
    coordinate_ = matrix;
    calculateModelBoundarySize();
}

void GeomObject::setVertexRadius(float radii)
{
    v_radii_ = radii;
}

void GeomObject::setPickingBoundaryWidth(float width)
{
    p_width_ = width;
}

void GeomObject::drawSurface(QMatrix4x4& transform_matrix, bool draw_boundary, bool force_draw)
{
    if (head_->isHide() && !force_draw) return;

    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<Vertex> nodes = snapshot_data_.nodes_;
    std::vector<Triangle> elements = snapshot_data_.elements_;
    locker.unlock();

    shader_->use();
    shader_->setColor(selected_ ? color_.lighter(105) : color_);
    shader_->setTransformation(transform_matrix);
    shader_->draw(nodes, elements, draw_boundary);
}

void GeomObject::drawBoundary(QMatrix4x4& transform_matrix, bool force_draw)
{
    if (head_->isHide() && !force_draw) return;

    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<Vertex> nodes = snapshot_data_.nodes_;
    std::vector<Segment> segments = snapshot_data_.segments_;
    locker.unlock();

    QColor color = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 0.8f);
    if (selected_) color = QColor::fromRgbF(1.0f, 1.0f, 0.0f);

    shader_->use();
    shader_->setColor(color);
    shader_->setTransformation(transform_matrix);
    shader_->draw(nodes, segments);
}

void GeomObject::drawVertices(QMatrix4x4& transform_matrix, bool force_draw)
{
    if (head_->isHide() && !force_draw) return;

    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<Vertex> vnodes = snapshot_data_.vnodes_;
    std::vector<Segment> vsegments = snapshot_data_.vsegments_;
    locker.unlock();

    QColor color(0, 0, 255);;
    if (selected_) color = QColor(255, 0, 0);

    shader_->use();
    shader_->setColor(color);
    shader_->setTransformation(transform_matrix);
    shader_->draw(vnodes, vsegments);
}

void GeomObject::drawForPicking(QMatrix4x4& transform_matrix)
{
    if (head_->isHide()) return;

    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<Vertex> nodes = snapshot_data_.nodes_;
    std::vector<Triangle> elements = snapshot_data_.elements_;
    std::vector<Vertex> pnodes = snapshot_data_.pnodes_;
    std::vector<Triangle> pelements = snapshot_data_.pelements_;
    locker.unlock();

    // ID를 색상으로 변환하여 설정
    QColor idColor(
        (id_ & 0xFF0000) >> 16, // R
        (id_ & 0x00FF00) >> 8,  // G
        (id_ & 0x0000FF)        // B
    );

    shader_->use();
    shader_->setColor(idColor);  // ID 색상 설정
    shader_->setTransformation(transform_matrix);

    // 닫힌 면인경우 
    if (elements.size() > 0) {
        shader_->draw(nodes, elements);
    }

    // 열린 선인경우
    else {
        shader_->draw(pnodes, pelements);
    }
}

void GeomObject::update()
{
    GeomToSurfaceMesh* converter;

    // 트리갱신 보호
    {
        //QMutexLocker locker(&lock_);
        converter = new GeomToSurfaceMesh(head_.get<GeomHeadNode*>());
    }
    rendering_data_.nodes_.clear();
    rendering_data_.segments_.clear();
    rendering_data_.elements_.clear();

    for (const auto& node : converter->nodes_) {
        rendering_data_.nodes_.push_back(Vertex{ (float)node[0], (float)node[1], 0.0f });
    }

    for (const auto& seg : converter->segments_) {
        rendering_data_.segments_.push_back(Segment{ (unsigned int)seg[0], (unsigned int)seg[1] });
    }

    for (const auto& ele : converter->elements_) {
        rendering_data_.elements_.push_back(Triangle{ (unsigned int)ele[0], (unsigned int)ele[1], (unsigned int)ele[2] });
    }

    bzmag::Color color = head_->getColor();
    setColor(QColor(color.r_, color.g_, color.b_, color.a_));

    // 스냅샷 갱신 보호
    {
        QMutexLocker locker(&lock_);
        snapshot_data_ = rendering_data_;
    }
    delete converter;
    generateScaleFactorDenpendantData(scale_factor_);
}

void GeomObject::generateScaleFactorDenpendantData(float scale_factor)
{
    scale_factor_ = scale_factor;

    // 절점 드로잉을 위한 데이터 생성
    rendering_data_.vnodes_.clear();
    rendering_data_.vsegments_.clear();

    unsigned int num_seg = 6;
    for (auto& node : rendering_data_.nodes_) {

        // 8개 세그먼트로 만든다
        unsigned int offset = (unsigned int)rendering_data_.vnodes_.size();
        for (unsigned int i = 0; i < num_seg - 1; ++i) {
            rendering_data_.vsegments_.push_back(Segment{ i + offset, i + 1 + offset });
        }
        rendering_data_.vsegments_.push_back(Segment{ num_seg - 1 + offset, offset });

        for (unsigned int i = 0; i < num_seg; i++) {
            float x = node[0] + v_radii_ * cos(2.0f * M_PI * i / num_seg) / scale_factor;
            float y = node[1] + v_radii_ * sin(2.0f * M_PI * i / num_seg) / scale_factor;
            rendering_data_.vnodes_.push_back(Vertex{ x, y, 0.0f });
        }
    }

    rendering_data_.pnodes_.clear();
    rendering_data_.pelements_.clear();

    unsigned int i = 0;
    for (const auto& seg : rendering_data_.segments_) {
        // 시작점과 끝점
        QVector3D start(rendering_data_.nodes_[seg[0]][0], rendering_data_.nodes_[seg[0]][1], rendering_data_.nodes_[seg[0]][2]);
        QVector3D end(rendering_data_.nodes_[seg[1]][0], rendering_data_.nodes_[seg[1]][1], rendering_data_.nodes_[seg[1]][2]);

        // 선의 방향 벡터와 정규화
        QVector3D direction = end - start;
        direction.normalize();

        // 선의 수직 벡터 계산 (화면 상의 z축을 고려한 2D 평면 상 수직 벡터)
        QVector3D up(0.0f, 0.0f, 1.0f);
        QVector3D perpendicular = QVector3D::crossProduct(direction, up).normalized() * p_width_ / (scale_factor * 2.0f);

        // 사각형의 네 꼭짓점 계산
        QVector3D p1 = start + perpendicular;
        QVector3D p2 = start - perpendicular;
        QVector3D p3 = end + perpendicular;
        QVector3D p4 = end - perpendicular;

        // 삼각형 두 개로 사각형 구성
        rendering_data_.pnodes_.insert(rendering_data_.pnodes_.end(), {
            {p1.x(), p1.y(), p1.z()},  // 첫 번째 삼각형
            {p2.x(), p2.y(), p2.z()},
            {p3.x(), p3.y(), p3.z()},

            {p2.x(), p2.y(), p2.z()},  // 두 번째 삼각형
            {p4.x(), p4.y(), p4.z()},
            {p3.x(), p3.y(), p3.z()},
            });
        rendering_data_.pelements_.insert(rendering_data_.pelements_.end(), {
            {i + 0, i + 1, i + 2},
            {i + 3, i + 4, i + 5},
            });

        i = i + 6;
    }

    // 스냅샷 갱신만 보호
    {
        QMutexLocker locker(&lock_);
        snapshot_data_.vnodes_ = rendering_data_.vnodes_;
        snapshot_data_.vsegments_ = rendering_data_.vsegments_;
        snapshot_data_.pnodes_ = rendering_data_.pnodes_;
        snapshot_data_.pelements_ = rendering_data_.pelements_;
    }
}


void GeomObject::calculateModelBoundarySize()
{
    if (rendering_data_.nodes_.size() == 0) return;

    QMatrix4x4 adjust_coord;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            adjust_coord(i, j) = coordinate_(i, j);
        }
    }
    adjust_coord = adjust_coord.inverted();

    QVector3D min_bound, max_bound;
    QVector4D node_conv = adjust_coord * QVector4D{ rendering_data_.nodes_[0][0],
                                                    rendering_data_.nodes_[0][1],
                                                    rendering_data_.nodes_[0][2], 1.0f };
    min_bound = max_bound = QVector3D{ node_conv.x(), node_conv.y(), node_conv.z() };
    for (const auto& node : rendering_data_.nodes_) {
        node_conv = adjust_coord * QVector4D{ node[0], node[1], node[2], 1.0f };
        min_bound.setX(std::min(min_bound.x(), node_conv.x()));
        min_bound.setY(std::min(min_bound.y(), node_conv.y()));
        min_bound.setZ(std::min(min_bound.z(), node_conv.z()));

        max_bound.setX(std::max(max_bound.x(), node_conv.x()));
        max_bound.setY(std::max(max_bound.y(), node_conv.y()));
        max_bound.setZ(std::max(max_bound.z(), node_conv.z()));
    }

    min_bound_ = min_bound;
    max_bound_ = max_bound;
}