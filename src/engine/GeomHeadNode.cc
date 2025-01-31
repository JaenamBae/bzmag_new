#include "GeomHeadNode.h"
#include "GeomBooleanNode.h"
#include "MaterialNode.h"
#include "CSNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomHeadNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomHeadNode::GeomHeadNode() :
    standalone_(true),
    model_node_(true),
    hide_(false),
    color_(230, 230, 230, 230),
    num_elements_(0),
    last_node_(nullptr),
    material_(nullptr)
{

}

//----------------------------------------------------------------------------
GeomHeadNode::~GeomHeadNode()
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
        cs_ = nullptr;
    }
}

//----------------------------------------------------------------------------
bool GeomHeadNode::contain(GeomHeadNode* node)
{
    const Polygon_set_2& geometry = getPolyset();
    const Polygon_set_2& geom = node->getPolyset();

    Polygon_set_2 op(geom);
    op.difference(geometry);
    if (op.is_empty())
        return true;

    return false;
}

//----------------------------------------------------------------------------
void GeomHeadNode::setLastNode(GeomBaseNode* last)
{
    last_node_ = last;
    be_geometry_ = false;

    GeomBaseNode* parent = dynamic_cast<GeomBaseNode*>(getParent());
    if (parent) parent->update();
}

//----------------------------------------------------------------------------
GeomBaseNode* GeomHeadNode::getLastNode() const
{
    return last_node_;
}

//----------------------------------------------------------------------------
void GeomHeadNode::setMaterialNode(MaterialNode* material)
{
    material_ = material;
}

//----------------------------------------------------------------------------
MaterialNode* GeomHeadNode::getMaterialNode() const
{
    return material_;
}

//----------------------------------------------------------------------------
void GeomHeadNode::setReferedCS(CSNode* cs)
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
    }
    cs_ = cs;
    if (cs_.valid()) {
        cs_->insertReferenceNode(this);
    }

    update();
}

//----------------------------------------------------------------------------
CSNode* GeomHeadNode::getReferedCS() const
{
    return cs_;
}

//----------------------------------------------------------------------------
void GeomHeadNode::forcedUpdate()
{
    //std::lock_guard<std::mutex> lock(mtx_);

    for (NodeIterator n = firstChildNode(); n != lastChildNode(); ++n)
    {
        Node* node = *n;
        GeomBaseNode* child = dynamic_cast<GeomBaseNode*>(node);
        if (child) {
            // 자식노드를 업데이트 함
            child->update();
        }
    }
}

//----------------------------------------------------------------------------
bool GeomHeadNode::isCovered() const
{
    if (last_node_) return last_node_->isCovered();
    return false;
}

//----------------------------------------------------------------------------
bool GeomHeadNode::makeGeometry(Transformation trans/* = Transformation()*/)
{
    std::lock_guard<std::mutex> lock(mtx_);

    GeomBaseNode* last = getLastNode();

    if (last) {
        bool result = last->makeGeometry(trans);
        geometry_ = last->geometry_;
        curves_ = last->curves_;
        vertices_ = last->vertices_;

        be_geometry_ = result;
        return result;
    }
    return false;
}

//----------------------------------------------------------------------------
bool GeomHeadNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    return true;
}

//----------------------------------------------------------------------------
void GeomHeadNode::updateCovered()
{
    be_covered_ = false;
}

//----------------------------------------------------------------------------
void GeomHeadNode::updateTransform()
{
    last_trans_ = Transformation();
}

//----------------------------------------------------------------------------
void GeomHeadNode::updateLinkedNode()
{
    linked_heads_.clear();
}

//----------------------------------------------------------------------------
bool GeomHeadNode::update()
{
    std::lock_guard<std::mutex> lock(mtx_);

    // 업데이트가 발생하면 makeGeometry()를 새로 호출해야 한다
    be_geometry_ = false;

    // 해드노드 설정 
    GeomBaseNode* parent = dynamic_cast<GeomBaseNode*>(getParent());
    if (parent) setupHead(parent);
    else head_ = this;

    // 1. 커버드 업데이트
    updateCovered();

    // 2. 최종변환 메트릭스 업데이트
    updateTransform();

    // 3. 링크노드 업데이트
    updateLinkedNode();

    return true;
}

//----------------------------------------------------------------------------
void GeomHeadNode::onAttachTo(Node* parent)
{
    // 부모가 Boolean 노드라면 부모 업데이트
    GeomBooleanNode* node = dynamic_cast<GeomBooleanNode*>(parent);
    if (node) {
        node->update();
        setStandAlone(false);
    }

    update();
}

//----------------------------------------------------------------------------
void GeomHeadNode::onDetachFrom(Node* parent)
{
    // 부모가 Boolean 노드라면 부모 업데이트
    GeomBooleanNode* node = dynamic_cast<GeomBooleanNode*>(parent);
    if (node) {
        node->update();
        setStandAlone(true);
    }

    update();
}

//----------------------------------------------------------------------------
void GeomHeadNode::clearBelongings()
{
    material_ = nullptr;
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
        cs_ = nullptr;
    }
}

//----------------------------------------------------------------------------
void GeomHeadNode::bindProperty()
{
    BIND_PROPERTY(const Color&, Color, &setColor, &getColor);
    BIND_PROPERTY(bool, IsHide, &setHideStatus, &isHide);
    BIND_PROPERTY(bool, IsModel, &setModelNode, &isModelNode);
    BIND_PROPERTY(bool, IsStandAlone, 0, &isStandAlone);
    BIND_PROPERTY(MaterialNode*, Material, &setMaterialNode, &getMaterialNode);
    BIND_PROPERTY(CSNode*, CoordinateSystem, &setReferedCS, &getReferedCS);
    BIND_PROPERTY(int32, RequiredNumberOfElements, &setNumberOfElements, &getNumberOfElements);
}

