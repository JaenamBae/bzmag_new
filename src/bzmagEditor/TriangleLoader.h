#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <spdlog/spdlog.h>  // spdlog 추가

class TriangleLoader {
public:
    // 노드와 삼각형에 대한 구조체 정의
    struct Vertex {
        double x, y;
        int boundary_marker = 0;  // 경계 마커
    };

    struct Triangle {
        unsigned int node_ids[3];
        int attribute = 0;  // 속성
    };

    using Segment = std::array<unsigned int, 2>;   // 세그먼트는 두 노드로 정의
    using Hole = std::array<double, 2>;            // 구멍의 위치는 2D 좌표

public:
    // .poly 파일에서 데이터를 읽음
    bool readPolyFile(const std::string& filename, bool zero_index = false);

    // .node 파일에서 노드 데이터를 읽음
    bool readNodeFile(const std::string& filename);

    // .ele 파일에서 삼각형 요소 데이터를 읽음
    bool readElementFile(const std::string& filename, bool zero_index = false);

    const std::vector<Vertex>& getNodes() const { return nodes_; }
    const std::vector<Triangle>& getElements() const { return triangles_; }
    const std::vector<Segment>& getPolygon() const { return segments_; }
    const std::vector<Hole>& getHoles() const { return holes_; }

private:
    bool readNodes(std::ifstream& file);
    bool readSegments(std::ifstream& file, bool zero_index);
    bool readHoles(std::ifstream& file);

private:
    std::vector<Vertex> nodes_;                // 노드 정보
    std::vector<Segment> segments_;            // 세그먼트 정보
    std::vector<Triangle> triangles_;          // 삼각형 정보
    std::vector<Hole> holes_;                  // 구멍 위치 정보
};
