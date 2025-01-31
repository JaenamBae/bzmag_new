#pragma once

#include "DrawingObject.h"
#include "core/ref.h"
#include <QMutex>
#include <memory>

class GeomObject : public DrawingObject
{
public:
    GeomObject(bzmag::engine::GeomHeadNode* head, Flat3DShader* shader);
    virtual ~GeomObject();

    bzmag::engine::GeomHeadNode* getHead();
    unsigned int getID() const;
    void setColor(const QColor& color);
    void setSelected(bool selected);
    bool isSelected() const;
    void setReferedMatrix(const QMatrix4x4& matrix);
    void setVertexRadius(float radii);
    void setPickingBoundaryWidth(float radii);

public:
    virtual void generateScaleFactorDenpendantData(float scale_factor);
    virtual void drawSurface(QMatrix4x4& transform_matrix, bool draw_boundary = false, bool force_draw = false);
    virtual void drawBoundary(QMatrix4x4& transform_matrix, bool force_draw = false);
    virtual void drawVertices(QMatrix4x4& transform_matrix, bool force_draw = false);
    virtual void drawForPicking(QMatrix4x4& transform_matrix);
    virtual void update();

protected:
    void calculateModelBoundarySize();

protected:
    bzmag::Ref<bzmag::engine::GeomHeadNode> head_;
    unsigned int id_;
    bool selected_ = false;

    QMatrix4x4 coordinate_;
    QColor color_;
    
    struct Data {
        // 렌더링에 필요한 기본 데이터
        std::vector<Vertex> nodes_;
        std::vector<Segment> segments_;
        std::vector<Triangle> elements_;

        // 기저 절점을 그리기 위한 좌표 및 세그먼트 데이터
        std::vector<Vertex> vnodes_;
        std::vector<Segment> vsegments_;

        // Closed Surface가 아닌 오브젝트에 대해
        // 피킹 바운드리를 그리기 위한 좌표 및 요소 데이터
        std::vector<Vertex> pnodes_;
        std::vector<Triangle> pelements_;
    };

    QMutex lock_;
    Data snapshot_data_;
    Data rendering_data_; // 최신 데이터

    float v_radii_ = 3.0f;  // 기저 절점을 나타내기위한 원의 반지름
    float p_width_ = 5.0f;  // 선 오브젝트에 대한 피킹 바운드리 너비

    float scale_factor_ = 1.0f;
};
