#include "GeomMoveNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/vector2.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomMoveNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomMoveNode::GeomMoveNode() : dx_(nullptr), dy_(nullptr), sdx_("0"), sdy_("0")
{
    uint32 key = getID();

    dx_ = new Expression();
    dy_ = new Expression();

    dx_->setKey("dx_" + std::to_string(key));
    dy_->setKey("dy_" + std::to_string(key));
}

//----------------------------------------------------------------------------
GeomMoveNode::~GeomMoveNode()
{

}

//----------------------------------------------------------------------------
bool GeomMoveNode::setParameters(const String& dx, const String& dy)
{
    if (!dx_.valid() || !dy_.valid()) return false;

    if (!dx_->setExpression(dx) || !dy_->setExpression(dy)) {
        dx_->setExpression(sdx_);
        dy_->setExpression(sdy_);

        return false;
    }

    sdx_ = dx;
    sdy_ = dy;

    return update();
}

//----------------------------------------------------------------------------
float64 GeomMoveNode::eval_dx() const
{
    if (dx_.valid())
        return dx_->eval();
    else
        return 0;
}

//----------------------------------------------------------------------------
float64 GeomMoveNode::eval_dy() const
{
    if (dy_.valid())
        return dy_->eval();
    else
        return 0;
}

//----------------------------------------------------------------------------
void GeomMoveNode::setReferedCS(CSNode* cs)
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
CSNode* GeomMoveNode::getReferedCS() const
{
    return cs_;
}

//----------------------------------------------------------------------------
void GeomMoveNode::clearBelongings()
{
    dx_->setExpression("0");
    dy_->setExpression("0");

    dx_ = nullptr;
    dy_ = nullptr;

    cs_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomMoveNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    return true;
}

//----------------------------------------------------------------------------
Transformation GeomMoveNode::getMyTransform()
{
    float64 dx = dx_->eval();
    float64 dy = dy_->eval();

    // 참조좌표계가 설정되어 있다면, 
    Transformation rotate;
    if (cs_.valid()) {
        float64 angle = cs_->getGlobalAngle();
        if ((std::isnan(angle) || std::isinf(angle))) {
            return Transformation();
        }

        NT diry = std::sin(angle) * 256 * 256 * 256;
        NT dirx = std::cos(angle) * 256 * 256 * 256;
        NT sin_alpha;
        NT cos_alpha;
        NT w;
        CGAL::rational_rotation_approximation(dirx, diry,
            sin_alpha, cos_alpha, w,
            NT(1), NT(1000000));

        rotate = Transformation(CGAL::ROTATION, sin_alpha, cos_alpha, w);
    }

    // 실제 이동 될 변위
    Vector_2 disp = rotate(Vector_2(dx, dy));
    Transformation transform(CGAL::TRANSLATION, disp);
    return transform;
}

//----------------------------------------------------------------------------
void GeomMoveNode::bindProperty()
{
    BIND_PROPERTY(const String&, dx, &set_dx, &get_dx);
    BIND_PROPERTY(const String&, dy, &set_dy, &get_dy);
    BIND_PROPERTY(CSNode*, CoordinateSystem, &setReferedCS, &getReferedCS);
}
