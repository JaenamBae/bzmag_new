#include "GeomBaseNode.h"
#include "GeomHeadNode.h"
#include "GeomBooleanNode.h"
#include "CSNode.h"
#include "GeomSplitNode.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_ABSTRACTCLASS(GeomBaseNode, Node);

//----------------------------------------------------------------------------
GeomBaseNode::GeomBaseNode()
    : head_(nullptr), be_covered_(false), be_geometry_(false), be_detaching_(false)
{

}

//----------------------------------------------------------------------------
GeomBaseNode::~GeomBaseNode()
{
    geometry_.clear();
    curves_.clear();
    vertices_.clear();
}

//----------------------------------------------------------------------------
GeomHeadNode* GeomBaseNode::getHeadNode()
{
    return head_;
}

//----------------------------------------------------------------------------
void GeomBaseNode::clearBelongings()
{

}

//----------------------------------------------------------------------------
bool GeomBaseNode::update()
{
    std::lock_guard<std::mutex> lock(mtx_);

    // 업데이트가 발생하면 makeGeometry()를 새로 호출해야 한다
    be_geometry_ = false;

    // 해드, 변환 메트릭스, 링크 노드 초기화 : 부모로부터 가져오기; 기본 초기화
    GeomBaseNode* parent = dynamic_cast<GeomBaseNode*>(getParent());
    if (parent) {
        // 해드노드 셋업 
        setupHead(parent);

        // 1. 커버드 초기화
        be_covered_ = parent->be_covered_;

        // 2. 최종변환 메트릭스 초기화
        last_trans_ = parent->last_trans_;

        // 3. 링크노드 초기화
        linked_heads_ = parent->linked_heads_;
    }
    else {
        head_ = nullptr;
        last_trans_ = Transformation();
        linked_heads_.clear();
        be_covered_ = false;
    }

    // 1. 커버드 업데이트
    updateCovered();

    // 2. 최종변환 메트릭스 업데이트
    updateTransform();

    // 3. 링크노드 업데이트
    updateLinkedNode();


    // 자식노드들을 업데이트 하는데 GeomBaseNode를 상속받은 class에 한한다
    size_t num_child = 0;
    for (NodeIterator n = firstChildNode(); n != lastChildNode(); ++n)
    {
        Node* node = *n;
        GeomBaseNode* child = dynamic_cast<GeomBaseNode*>(node);
        if (child && !child->be_detaching_) {
            // 자식노드를 업데이트 함
            child->update();

            // GeomBaseNode 상속 클래스 중 GeomHeadNode를 제외하고 자식수 카운팅
            if (!child->isKindOf("GeomHeadNode")) num_child++;
        }
    }

    // 업데이트 할 자식노드가 0이면 즉, 없다면 자신이 최종 노드이다.
    // 이 경우 나의 해드노드의 최종노드로 자신을 설정한다
    if ((0 == num_child) && head_) {
        // 해드노드에 최종노드가 자신임을 통지함
        head_->setLastNode(this);
    }

    return true;
}

//----------------------------------------------------------------------------
void GeomBaseNode::setupHead(GeomBaseNode* parent)
{
    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(parent);

    // 부모노드가 GeomHeadNode이면 head_값 설정
    if (head) {
        head_ = head;
    }

    // 그렇지 않으면 부모노의 해드노드를 현재노드의 해드노드로 설정
    else if (parent) {
        head_ = parent->getHeadNode();
    }
}

//----------------------------------------------------------------------------
void GeomBaseNode::updateTransform()
{
    Transformation trans = getMyTransform();
    last_trans_ = trans * last_trans_;
}

//----------------------------------------------------------------------------
void GeomBaseNode::updateLinkedNode()
{
    Transformation trans = getMyTransform();

    LinkedHeads::iterator it;
    for (it = linked_heads_.begin(); it != linked_heads_.end(); ++it)
    {
        Transformation prev_trans = (*it).second;
        (*it).second = trans * prev_trans;
    }
}

//----------------------------------------------------------------------------
void GeomBaseNode::updateCovered()
{

}


//----------------------------------------------------------------------------
bool GeomBaseNode::makeHistory(std::list<GeomBaseNode*>& history, GeomHeadNode* head)
{
    history.push_back(this);
    if (this == head) return true;
    else {
        GeomBaseNode* parent = dynamic_cast<GeomBaseNode*>(getParent());
        if (parent)
            return parent->makeHistory(history, head);
        else
            return false;
    }
}

//----------------------------------------------------------------------------
const Polygon_set_2& GeomBaseNode::getPolyset()
{
    if (!be_geometry_)
        makeGeometry();
    return geometry_;
}

//----------------------------------------------------------------------------
const GeomBaseNode::Curves& GeomBaseNode::getCurves()
{
    if (!be_geometry_)
        makeGeometry();
    return curves_;
}

//----------------------------------------------------------------------------
const GeomBaseNode::Vertices& GeomBaseNode::getVertices()
{
    if (!be_geometry_)
        makeGeometry();
    return vertices_;
}

//----------------------------------------------------------------------------
bool GeomBaseNode::hitTest(float64 x, float64 y)
{
    if (!be_geometry_)
        makeGeometry();

    Polygon_with_holes_2 poly;
    if (geometry_.locate(
        Traits_2::Point_2(CoordNT(x), CoordNT(y)), poly))
        return true;

    return false;
}

//----------------------------------------------------------------------------
bool GeomBaseNode::isCovered() const
{
    return be_covered_;
}

//----------------------------------------------------------------------------
bool GeomBaseNode::makeGeometry(Transformation trans/* = Transformation()*/)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // Linked 해드를 먼저 업데이트 한다; 역순으로 업데이트 해야 함!
    for (LinkedHeads::iterator it = linked_heads_.begin(); it != linked_heads_.end(); ++it)
    {
        GeomHeadNode* linked_head = (*it).first;
        Transformation extra_trans = (*it).second;
        linked_head->makeGeometry(trans * extra_trans);
    }

    // 히스토리 따라서 형상 만들기
    std::list<GeomBaseNode*> history;

    // 하기 makeHistory는 역순으로 히스토리를 만든다
    if (makeHistory(history, getHeadNode()))
    {
        geometry_.clear();
        curves_.clear();
        vertices_.clear();

        std::list<GeomBaseNode*>::reverse_iterator ii;
        for (ii = history.rbegin(); ii != history.rend(); ++ii)
        {
            GeomBaseNode* node = *ii;
            Transformation make_trans;
            if (dynamic_cast<GeomSplitNode*>(node)) {
                for (auto it_last = ii; it_last != history.rend(); ++it_last)
                {
                    make_trans = (*it_last)->getMyTransform() * make_trans;
                }
                make_trans = trans * make_trans;
            }
            else {
                make_trans = trans * last_trans_;
            }
            if (!node->make_geometry(geometry_, curves_, vertices_, make_trans)) {
                std::cout << "Fail to make geometry! " << node->getName().c_str() << std::endl;
                return false;
            }
        }
        if (be_covered_) {
            indexingGeometry(geometry_, curves_, vertices_);
        }

        be_geometry_ = true;
    }
    else {
        be_geometry_ = false;
    }

    return be_geometry_;
}

//----------------------------------------------------------------------------
Transformation GeomBaseNode::getMyTransform()
{
    return Transformation();
}


//----------------------------------------------------------------------------
void GeomBaseNode::indexingGeometry(const Polygon_set_2& geometry, Curves& curves, Vertices& vertices) const
{
    // 최종 형상을 이루는 점/선에 대한 인덱싱

    // 기존데이터 삭제
    curves.clear();
    vertices.clear();

    // Polygon_set의 각 polygon_with_hole 에 대해서
    // 점과 선을 추출함
    std::list<Polygon_with_holes_2> res;
    std::list<Polygon_with_holes_2>::const_iterator it;
    geometry.polygons_with_holes(std::back_inserter(res));

    for (it = res.begin(); it != res.end(); ++it)
    {
        Polygon_with_holes_2 polyhole = *it;
        if (!polyhole.is_unbounded()) {
            const Polygon_2& poly_o = polyhole.outer_boundary();

            // 여기서 function call을 통해 Polygon을 이루는 점/선에 대한 인덱싱 작업을 한다
            indexingPolygon(poly_o, curves, vertices);

            Polygon_with_holes_2::Hole_iterator hit;
            for (hit = polyhole.holes_begin(); hit != polyhole.holes_end(); ++hit) {
                const Polygon_2& poly_i = *hit;
                indexingPolygon(poly_i, curves, vertices);
            }
        }
    }
}

//----------------------------------------------------------------------------
void GeomBaseNode::indexingPolygon(const Polygon_2& poly, Curves& curves, Vertices& vertices) const
{
    Polygon_2::Curve_const_iterator it;
    for (it = poly.curves_begin(); it != poly.curves_end(); ++it)
    {
        const X_monotone_curve_2& curve = *it;
        curves.emplace_back(curve);

        Traits_2::Point_2 source = curve.source();
        Traits_2::Point_2 target = curve.target();
        // 이하 코드가 맞는지..? // 2019.09.14
        if (curve.orientation() == CGAL::COUNTERCLOCKWISE)
            vertices.emplace_back(source);
        else
            vertices.emplace_back(target);
    }
}


//----------------------------------------------------------------------------
void GeomBaseNode::onAttachTo(Node* parent)
{
    be_detaching_ = false;

    // 최종형상에 적용될 Transform을 업데이트한다
    update();
}

//----------------------------------------------------------------------------
void GeomBaseNode::onDetachFrom(Node* parent)
{
    be_detaching_ = true;

    // 원래 Head노드를 업데이트 (원래 Head노드의 lastNode를 새로 설정하기 위함)
    if (head_) {
        head_->forcedUpdate();
    }

    // 최종형상에 적용될 Transform을 업데이트한다
    update();
}

//----------------------------------------------------------------------------
void GeomBaseNode::bindProperty()
{
    //BIND_PROPERTY(CSNode*, CoordinateSystem,
    //    &setReferedCS,
    //    &getReferedCS);
}
