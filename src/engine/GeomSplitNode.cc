#include "GeomSplitNode.h"
#include "CSNode.h"
//#include "Expression.h"
#include "core/simplepropertybinder.h"
#include "core/enumpropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomSplitNode, GeomBaseNode);

//----------------------------------------------------------------------------
float64 GeomSplitNode::INF_ = 1e15;

//----------------------------------------------------------------------------
GeomSplitNode::GeomSplitNode() : plane_(SPLIT_ZX), selectd_plane_(true)
{

}

//----------------------------------------------------------------------------
GeomSplitNode::~GeomSplitNode()
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
        cs_ = nullptr;
    }
}

//----------------------------------------------------------------------------
void GeomSplitNode::setReferedCS(CSNode* cs)
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
CSNode* GeomSplitNode::getReferedCS() const
{
    return cs_;
}

//----------------------------------------------------------------------------
void GeomSplitNode::clearBelongings()
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
        cs_ = nullptr;
    }
}

//----------------------------------------------------------------------------
bool GeomSplitNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    // 부모노드가 없으면 연산을 수행할 수 없음
    GeomBaseNode* parent = dynamic_cast<GeomBaseNode*>(getParent());
    if (!parent) {
        return false;
    }

    // Construct Half plane
    // 모델의 최대 크기는 double이 가질수 있는 최대 값의 1/2로 가정하였음
    float64 px = 0;
    float64 py = 0;
    float64 dx = 0;
    float64 dy = 0;
    if (plane_ == SPLIT_ZX)
    {
        px = -INF_ * 0.25;
        dx = INF_ * 0.5;
        if (selectd_plane_) {
            py = 0;
            dy = -INF_ * 0.25;
        }
        else {
            py = 0;
            dy = INF_ * 0.25;
        }
    }
    else if (plane_ == SPLIT_YZ) {
        py = -INF_ * 0.25;
        dy = INF_ * 0.5;
        if (selectd_plane_) {
            px = 0;
            dx = -INF_ * 0.25;
        }
        else {
            px = 0;
            dx = INF_ * 0.25;
        }
    }
    else {
        std::cout << "Fail to spilt Object : Undefined Plane" << plane_ << std::endl;
        return false;
    }

    // 반시계방향으로 포인트 생성
    Point_2 p1(px, py);
    Point_2 p2(px + dx, py);
    Point_2 p3(px + dx, py + dy);
    Point_2 p4(px, py + dy);
    if ((dx < 0 && dy > 0) || (dx > 0 && dy < 0))
    {
        std::swap(p2, p4);
    }

    Rectangle_2 rect(p1, p3);
    if (!rect.is_degenerate()) {
        Polygon_2 poly;
        Transformation cs_trans = transform;
        if (cs_.valid()) cs_trans = cs_trans * cs_->transformation();
        p1 = cs_trans(p1);
        p2 = cs_trans(p2);
        p3 = cs_trans(p3);
        p4 = cs_trans(p4);

        X_monotone_curve_2 s1(p1, p2);    poly.push_back(s1);
        X_monotone_curve_2 s2(p2, p3);    poly.push_back(s2);
        X_monotone_curve_2 s3(p3, p4);    poly.push_back(s3);
        X_monotone_curve_2 s4(p4, p1);    poly.push_back(s4);

        polyset.difference(poly);
    }
    else
    {
        std::cout << "Fail to spilt Object : Degenerated Plane" << std::endl;
    }

    return true;
}

//----------------------------------------------------------------------------
void GeomSplitNode::bindProperty()
{
    // enum
    BIND_ENUM_PROPERTY(SPLIT_PLANE, Plane, &setPlane, &getPlane);
        ENUM_PROPERTY_ADD(SPLIT_PLANE, SPLIT_ZX);
        ENUM_PROPERTY_ADD(SPLIT_PLANE, SPLIT_YZ);

    BIND_PROPERTY(bool, KeepPositive, &setOrientation, &getOrientation);
    BIND_PROPERTY(CSNode*, CoordinateSystem, &setReferedCS, &getReferedCS);
}
