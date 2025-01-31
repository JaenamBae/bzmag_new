#include "TriangleLoader.h"


// 파일에서 poly 데이터를 읽어오는 함수
bool TriangleLoader::readPolyFile(const std::string& filename, bool zero_index)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        spdlog::error("Failed to open poly file: {}", filename);
        return false;
    }

    spdlog::info("Reading poly file: {}", filename);

    // 노드, 세그먼트, 폴리곤, 홀을 순서대로 읽기
    if (!readNodes(file)) return false;
    if (!readSegments(file, zero_index)) return false;
    if (!readHoles(file)) return false;

    spdlog::info("Finished reading poly file.");
    return true;
}

// .node 파일을 읽어오는 함수 (경계 마커 포함)
bool TriangleLoader::readNodeFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        spdlog::error("Failed to open node file: {}", filename);
        return false;
    }

    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);

    int num_nodes, dimensions, num_attributes, boundary_marker;
    iss >> num_nodes >> dimensions >> num_attributes >> boundary_marker;

    nodes_.clear();
    while (std::getline(file, line)) {
        std::istringstream nodeStream(line);
        Vertex node;
        int node_id;
        nodeStream >> node_id >> node.x >> node.y;
        if (boundary_marker) nodeStream >> node.boundary_marker;  // 경계 마커 읽기
        nodes_.push_back(node);
    }

    file.close();
    return true;
}

// .ele 파일을 읽어오는 함수 (속성 포함)
bool TriangleLoader::readElementFile(const std::string& filename, bool zero_index) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        spdlog::error("Failed to open element file: {}", filename);
        return false;
    }

    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);

    int num_triangles, nodes_per_element, num_attributes;
    iss >> num_triangles >> nodes_per_element >> num_attributes;

    triangles_.clear();
    while (std::getline(file, line)) {
        std::istringstream triangleStream(line);
        Triangle triangle;
        int triangle_id;
        triangleStream >> triangle_id;

        for (int i = 0; i < nodes_per_element; ++i) {
            triangleStream >> triangle.node_ids[i];
            if (!zero_index) triangle.node_ids[i]--;
        }

        if (num_attributes > 0) triangleStream >> triangle.attribute;  // 속성 읽기
        triangles_.push_back(triangle);
    }

    file.close();
    return true;
}

// 노드 정보를 읽어오는 함수
bool TriangleLoader::readNodes(std::ifstream& file) {
    std::string line;
    std::getline(file, line);  // 첫 번째 줄(노드 헤더) 읽기
    std::istringstream iss(line);

    int num_nodes, dimensions, num_attributes, boundary_marker;
    iss >> num_nodes >> dimensions >> num_attributes >> boundary_marker;

    spdlog::info("Poly file contains {} nodes", num_nodes);

    // 노드 정보 읽기
    nodes_.clear();
    while (std::getline(file, line)) {
        std::istringstream nodeStream(line);
        Vertex node;
        int node_id;
        nodeStream >> node_id >> node.x >> node.y;
        if (boundary_marker) nodeStream >> node.boundary_marker;  // 경계 마커 읽기
        nodes_.push_back(node);
    }

    return true;
}

// 세그먼트 정보를 읽어오는 함수
bool TriangleLoader::readSegments(std::ifstream& file, bool zero_index) {
    std::string line;
    std::getline(file, line);  // 세그먼트 헤더 읽기
    std::istringstream iss(line);

    int num_segments, boundary_marker;
    iss >> num_segments >> boundary_marker;

    spdlog::info("Poly file contains {} segments", num_segments);

    // 세그먼트 정보 읽기
    segments_.clear();
    for (int i = 0; i < num_segments; ++i) {
        Segment segment;
        std::getline(file, line);
        std::istringstream segmentStream(line);
        int segment_id;
        segmentStream >> segment_id >> segment[0] >> segment[1];

        if (!zero_index) {
            segment[0]--;
            segment[1]--;
        }
        segments_.push_back(segment);
        //spdlog::info("Read segment {}: ({} -> {})", segment_id, segment[0], segment[1]);
    }

    return true;
}

// 구멍 정보를 읽어오는 함수
bool TriangleLoader::readHoles(std::ifstream& file) {
    std::string line;
    std::getline(file, line);  // 구멍 헤더 읽기
    std::istringstream iss(line);

    int num_holes;
    iss >> num_holes;

    spdlog::info("Poly file contains {} holes", num_holes);

    // 구멍 정보 읽기
    holes_.clear();
    for (int i = 0; i < num_holes; ++i) {
        Hole hole;
        std::getline(file, line);
        std::istringstream holeStream(line);
        int hole_id;
        holeStream >> hole_id >> hole[0] >> hole[1];

        holes_.push_back(hole);
        spdlog::info("Read hole {}: ({}, {})", hole_id, hole[0], hole[1]);
    }

    return true;
}
