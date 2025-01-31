#include "DataSetNode.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

#include "MaterialNode.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(DataSetNode, Node);

//----------------------------------------------------------------------------
DataSetNode::DataSetNode() = default;

//----------------------------------------------------------------------------
DataSetNode::~DataSetNode() = default;

//----------------------------------------------------------------------------
const DataSet& DataSetNode::getDataset() const
{
    return dataset_;
}

//----------------------------------------------------------------------------
void DataSetNode::setDataset(const DataSet& dataset)
{
    dataset_ = dataset;
}

void DataSetNode::setDataset(const String& data)
{
    DataSet dataset;
    std::istringstream iss(data.c_str()); // 전체 데이터 스트림
    std::string segment;

    while (std::getline(iss, segment)) {
        std::istringstream segment_stream(segment); // 각 세그먼트 스트림 처리
        double x, y;
        char seperator;

        // 데이터가 유효한지 확인
        if (segment_stream >> x >> seperator >> y) {
            dataset.emplace_back(x, y); // x, y를 데이터셋에 추가
        }
        else {
            std::cerr << "Invalid segment: " << segment << std::endl; // 잘못된 세그먼트 출력
        }
    }

    dataset_ = dataset; // 결과를 멤버 변수에 저장
}

//----------------------------------------------------------------------------
DataPair DataSetNode::getData(size_t index) const
{
    return dataset_.at(index); // 범위를 벗어나면 std::out_of_range 예외 발생
}

//----------------------------------------------------------------------------
std::vector<float64> DataSetNode::extractXComponents() const
{
    std::vector<float64> x_components;
    x_components.reserve(dataset_.size()); // 메모리 예약

    std::transform(dataset_.begin(), dataset_.end(), std::back_inserter(x_components),
        [](const DataPair& pair) { return pair.x_; });

    return x_components;
}

//----------------------------------------------------------------------------
std::vector<float64> DataSetNode::extractYComponents() const
{
    std::vector<float64> y_components;
    y_components.reserve(dataset_.size()); // 메모리 예약

    std::transform(dataset_.begin(), dataset_.end(), std::back_inserter(y_components),
        [](const DataPair& pair) { return pair.y_; });

    return y_components;
}

//----------------------------------------------------------------------------
void DataSetNode::setData(size_t index, const DataPair& data)
{
    dataset_.at(index) = data; // 범위를 벗어나면 std::out_of_range 예외 발생
}

//----------------------------------------------------------------------------
void DataSetNode::addData(const DataPair& data)
{
    dataset_.push_back(data);
}

//----------------------------------------------------------------------------
void DataSetNode::clearDataset()
{
    dataset_.clear();
}

//----------------------------------------------------------------------------
size_t DataSetNode::size() const 
{
    return dataset_.size();
}

//----------------------------------------------------------------------------
bool DataSetNode::update()
{
    return true;
}

//----------------------------------------------------------------------------
void DataSetNode::onAttachTo(Node* parent)
{
    MaterialNode* material_node = dynamic_cast<MaterialNode*>(parent);
    if (material_node) {
        material_node->setDataSetNode(this);
    }
}

//----------------------------------------------------------------------------
void DataSetNode::onDetachFrom(Node* parent)
{
    MaterialNode* material_node = dynamic_cast<MaterialNode*>(parent);
    if (material_node) {
        material_node->setDataSetNode(nullptr);
    }
}

//----------------------------------------------------------------------------
void DataSetNode::bindProperty()
{
    BIND_PROPERTY(const DataSet&, Dataset, &setDataset, &getDataset);
}
