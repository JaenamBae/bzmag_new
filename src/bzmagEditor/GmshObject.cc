#include "GmshObject.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

GmshObject::GmshObject(Flat3DShader* shader) : 
    DrawingObject(shader), total_num_nodes_(0), total_num_elements_(0)
{
    setColor(QColor(128, 128, 128));
}

GmshObject::~GmshObject()
{

}

void GmshObject::setColor(const QColor& color)
{
    color_ = color;
}

bool GmshObject::loadMesh(const QString& file_path)
{
    bzmag::String temp(file_path.toStdWString().c_str());
    std::string name(temp.c_str());
    std::ifstream file(name);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("$Nodes") != std::string::npos) {
            parseNodes(file);
        }
        else if (line.find("$Elements") != std::string::npos) {
            parseElements(file);
        }
    }

    file.close();

    if (nodes_.size() != total_num_nodes_) {
        nodes_.clear();
        elements_.clear();
        segments_.clear();
        return false;
    }
    /*if (elements_.size() + segments_.size() != total_num_elements_) {
        nodes_.clear();
        elements_.clear();
        segments_.clear();
        return false;
    }*/

    return true;
}

void GmshObject::parseNodes(std::ifstream& file)
{
    std::string line;
    std::getline(file, line); // Read block info
    std::istringstream iss(line);

    unsigned int num_entity_blocks, min_node_tag, max_node_tag;
    iss >> num_entity_blocks >> total_num_nodes_ >> min_node_tag >> max_node_tag;

    nodes_.clear();
    nodes_.reserve(total_num_nodes_); // 사전 메모리 예약

    std::vector<unsigned int> node_tags; // 노드 태그를 저장할 벡터

    for (unsigned int i = 0; i < num_entity_blocks; ++i) {
        // Read entity block header
        std::getline(file, line);
        iss.clear();
        iss.str(line);

        unsigned int entity_dim, entity_tag, parametric, num_nodes_in_block;
        iss >> entity_dim >> entity_tag >> parametric >> num_nodes_in_block;

        // Read node tags
        for (unsigned int j = 0; j < num_nodes_in_block; ++j) {
            std::getline(file, line);
            iss.clear();
            iss.str(line);

            unsigned int node_tag;
            iss >> node_tag;
            node_tags.push_back(node_tag);
        }

        // Read node coordinates
        for (unsigned int i = 0; i < num_nodes_in_block; ++i) {
            std::getline(file, line);
            iss.clear();
            iss.str(line);

            float x, y, z;
            iss >> x >> y >> z;

            nodes_.emplace_back(Vertex{ x, y, z });
        }
    }
}

void GmshObject::parseElements(std::ifstream& file)
{
    std::string line;
    std::getline(file, line); // Read block info
    std::istringstream iss(line);

    unsigned int num_entity_blocks;
    iss >> num_entity_blocks >> total_num_elements_;

    elements_.clear();
    segments_.clear();

    for (unsigned int i = 0; i < num_entity_blocks; ++i) {
        std::getline(file, line);
        iss.clear();
        iss.str(line);

        unsigned int entity_dim, entity_tag, element_type, num_elements_in_block;
        iss >> entity_dim >> entity_tag >> element_type >> num_elements_in_block;

        for (unsigned int j = 0; j < num_elements_in_block; ++j) {
            std::getline(file, line);
            iss.clear();
            iss.str(line);

            unsigned int element_tag;
            std::array<unsigned int, 3> nodes;
            if (element_type == 2) { // 삼각형 요소
                iss >> element_tag >> nodes[0] >> nodes[1] >> nodes[2];
                elements_.emplace_back(Triangle{ nodes[0] - 1, nodes[1] - 1, nodes[2] - 1 }); // 0-based 인덱스
            }
            else if (element_type == 1) { // 선 요소
                std::array<unsigned int, 2> segment_nodes;
                iss >> element_tag >> segment_nodes[0] >> segment_nodes[1];
                segments_.emplace_back(Segment{ segment_nodes[0] - 1, segment_nodes[1] - 1 }); // 0-based 인덱스
            }
        }
    }
}

void GmshObject::drawMesh(QMatrix4x4& transform_matrix)
{
    QMatrix4x4 trans;
    trans.scale(1000);

    shader_->use();
    shader_->setColor(color_);
    shader_->setTransformation(transform_matrix* trans);
    shader_->draw(nodes_, elements_, true);
}