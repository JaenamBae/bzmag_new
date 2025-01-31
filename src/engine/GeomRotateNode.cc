#include "GeomRotateNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/vector2.h"
#include "core/kernel.h"
#include "core/simplepropertybinder.h"
#include "core/enumpropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomRotateNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomRotateNode::GeomRotateNode() : angle_(nullptr), sangle_("0")
{
    uint32 key = getID();

    angle_ = new Expression();
    angle_->setKey("angle_" + std::to_string(key));
}

//----------------------------------------------------------------------------
GeomRotateNode::~GeomRotateNode()
{

}

//----------------------------------------------------------------------------
bool GeomRotateNode::setParameters(const String& angle)
{
    if (!angle_.valid()) return false;

    if (!angle_->setExpression(angle)) {
        angle_->setExpression(sangle_);
        return false;
    }

    sangle_ = angle;

    return update();
}

//----------------------------------------------------------------------------
float64 GeomRotateNode::evalAngle() const
{
    if (angle_.valid())
        return angle_->eval();
    return 0;
}

//----------------------------------------------------------------------------
void GeomRotateNode::setReferedCS(CSNode* cs)
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
CSNode* GeomRotateNode::getReferedCS() const
{
    return cs_;
}

//----------------------------------------------------------------------------
void GeomRotateNode::clearBelongings()
{
    angle_->setExpression("0");
    angle_ = nullptr;

    cs_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomRotateNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    return true;
}

//----------------------------------------------------------------------------
Transformation GeomRotateNode::getMyTransform()
{
    // 주어진 회전각
    float64 angle = angle_->eval();

    // 실제 Rotate 명령을 위한 변환식
    NT diry = std::sin(angle) * 256 * 256 * 256;
    NT dirx = std::cos(angle) * 256 * 256 * 256;
    NT sin_alpha;
    NT cos_alpha;
    NT w;
    CGAL::rational_rotation_approximation(dirx, diry,
        sin_alpha, cos_alpha, w,
        NT(1), NT(1000000));

    Transformation rotate(CGAL::ROTATION, sin_alpha, cos_alpha, w);

    // 참조 좌표계를 고려하기 위한 변환식
    Transformation trans;
    if (cs_.valid()) trans = cs_->transformation();

    // 절대좌표계상의 기준 좌표계의 원점 계산
    Point_2 org = trans(Point_2(0, 0));
    Transformation translate1(CGAL::TRANSLATION, Vector_2(org.x(), org.y()));
    Transformation translate2(CGAL::TRANSLATION, Vector_2(-org.x(), -org.y()));

    //Vector_2 v1(cs_->getGlobalOriginX(), cs_->getGlobalOriginY());
    //Vector_2 v2(-cs_->getGlobalOriginX(), -cs_->getGlobalOriginY());
    //Transformation translate1(CGAL::TRANSLATION, v1);
    //Transformation translate2(CGAL::TRANSLATION, v2);

    // 아래 좌표변환식은 한번더 고민해 봐야 함
    Transformation transform = translate1 * (rotate * translate2);
    return transform;
}

//----------------------------------------------------------------------------
void GeomRotateNode::bindProperty()
{
    BIND_PROPERTY(const String&, Angle, &setAngle, &getAngle);
    BIND_PROPERTY(CSNode*, CoordinateSystem, &setReferedCS, &getReferedCS);
}
