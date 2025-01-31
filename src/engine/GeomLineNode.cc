#include "GeomLineNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;


IMPLEMENT_CLASS(GeomLineNode, GeomPrimitiveNode);

//----------------------------------------------------------------------------
GeomLineNode::GeomLineNode() : sx_(nullptr), sy_(nullptr), ex_(nullptr), ey_(nullptr)
    ,sstart_("0,0"), send_("0,0")
{
    uint32 key = getID();

    sx_ = new Expression();
    sy_ = new Expression();
    ex_ = new Expression();
    ey_ = new Expression();

    sx_->setKey("sx_" + std::to_string(key));
    sy_->setKey("sy_" + std::to_string(key));
    ex_->setKey("ex_" + std::to_string(key));
    ey_->setKey("ey_" + std::to_string(key));

}

//----------------------------------------------------------------------------
GeomLineNode::~GeomLineNode()
{

}

//----------------------------------------------------------------------------
bool GeomLineNode::setParameters(const String& start, const String& end)
{
    if (!sx_.valid() ||
        !sy_.valid() ||
        !ex_.valid() ||
        !ey_.valid()) return false;

    // point 를 ','로 분리해 x,y 값을 얻는다
    auto token_start = Expression::splitByTopLevelComma(start);
    if (token_start.size() != 2) return false;

    auto token_end = Expression::splitByTopLevelComma(end);
    if (token_end.size() != 2) return false;

    // 시작점
    // sx, sy의 이전값 임시 저장
    const String& psx = sx_->getExpression();
    const String& psy = sy_->getExpression();

    // expression에 셋팅 시도
    if (!sx_->setExpression(token_start[0])
        || !sy_->setExpression(token_start[1]))
    {
        // 실패시 이전값 복원
        sx_->setExpression(psx);
        sy_->setExpression(psy);

        return false;
    }

    // 끝점
    // ex, ey의 이전값 임시 저장
    const String& pex = ex_->getExpression();
    const String& pey = ey_->getExpression();

    // expression에 셋팅 시도
    if (!ex_->setExpression(token_end[0])
        || !ey_->setExpression(token_end[1]))
    {
        // 실패시 이전값 복원
        ex_->setExpression(pex);
        ey_->setExpression(pey);

        return false;
    }

    sstart_ = start;
    send_ = end;

    return update();
}

//----------------------------------------------------------------------------
void GeomLineNode::clearBelongings()
{
    sx_->setExpression("0");
    sy_->setExpression("0");
    ex_->setExpression("0");
    ey_->setExpression("0");

    sx_ = nullptr;
    sy_ = nullptr;
    ex_ = nullptr;
    ey_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomLineNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    if (!sx_.valid() ||
        !sy_.valid() ||
        !ex_.valid() ||
        !ey_.valid()) return false;

    float64 sx = sx_->eval();
    float64 sy = sy_->eval();

    float64 ex = ex_->eval();
    float64 ey = ey_->eval();

    if (sx == ex && sy == ey) {
        return true;
    }

    Point_2 ss = transform(Point_2(sx, sy));
    Point_2 ee = transform(Point_2(ex, ey));
    Curve_2 curve(ss, ee);
    if (curve.source() != curve.target())
    {
        /*
        // for CGAL 6.x
        Traits_2 traits;
        Polygon_2 poly;
        traits.make_x_monotone_2_object() (curve,
            CGAL::dispatch_or_drop_output<X_monotone_curve_2>(std::back_inserter(poly)));

        /*/
        // for CGAL 5.x
        Traits_2 traits;
        std::list<CGAL::Object>  objects;
        traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
        //CGAL_assertion(objects.size() == 2);

        // Insert arcs to the curves_
        X_monotone_curve_2 arc;
        std::list<CGAL::Object>::iterator iter;

        Polygon_2 poly;
        for (iter = objects.begin(); iter != objects.end(); ++iter) {
            CGAL::assign(arc, *iter);
            poly.push_back(arc);
        }
        //*/

        Polygon_2::Curve_const_iterator i;
        for (i = poly.curves_begin(); i != poly.curves_end(); ++i) {
            X_monotone_curve_2 arc = *i;
            curves.push_back(arc);

            Traits_2::Point_2 source = arc.source();
            Traits_2::Point_2 target = arc.target();
            vertices.push_back(source);
            vertices.push_back(target);
        }
    }

    return true;
}

//----------------------------------------------------------------------------
void GeomLineNode::bindProperty()
{
    BIND_PROPERTY(const String&, StartPoint, &setStartPoint, &getStartPoint);
    BIND_PROPERTY(const String&, EndPoint, &setEndPoint, &getEndPoint);
}
