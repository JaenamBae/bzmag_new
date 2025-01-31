#pragma once

#include "DrawingObject.h"
#include <string>
#include <vector>

class GmshObject : public DrawingObject
{
public:
    GmshObject(Flat3DShader* shader);
    virtual ~GmshObject();

    bool loadMesh(const QString& file_path);
    void setColor(const QColor& color);
    void drawMesh(QMatrix4x4& transform_matrix);

private:
    void parseNodes(std::ifstream& file);
    void parseElements(std::ifstream& file);

protected:
    QColor color_;

    unsigned int total_num_nodes_;    // Gmsh 파일에서의 총 노드 수
    unsigned int total_num_elements_; // Gmsh 파일에서의 총 요소 수

    // 렌더링에 필요한 기본 데이터
    std::vector<Vertex> nodes_;
    std::vector<Segment> segments_;
    std::vector<Triangle> elements_;
};
