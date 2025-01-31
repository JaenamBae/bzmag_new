#include "GeomCurveNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;


IMPLEMENT_CLASS(GeomCurveNode, GeomPrimitiveNode);

//----------------------------------------------------------------------------
GeomCurveNode::GeomCurveNode() : sx_(nullptr), sy_(nullptr), ex_(nullptr), ey_(nullptr),
    cx_(nullptr), cy_(nullptr), radius_(nullptr),
    sstart_("0,0"), send_("0,0"), scenter_("0,0"), sradius_("0")
{
    uint32 key = getID();

    sx_ = new Expression();
    sy_ = new Expression();
    ex_ = new Expression();
    ey_ = new Expression();
    cx_ = new Expression();
    cy_ = new Expression();
    radius_ = new Expression();

    sx_->setKey("sx_" + std::to_string(key));
    sy_->setKey("sy_" + std::to_string(key));
    ex_->setKey("ex_" + std::to_string(key));
    ey_->setKey("ey_" + std::to_string(key));
    cx_->setKey("cx_" + std::to_string(key));
    cy_->setKey("cy_" + std::to_string(key));
    radius_->setKey("radius_" + std::to_string(key));
}

//----------------------------------------------------------------------------
GeomCurveNode::~GeomCurveNode()
{

}

//----------------------------------------------------------------------------
bool GeomCurveNode::setParameters(const String& start, const String& end, const String& center, const String& radius)
{
    if (!sx_.valid() ||
        !sy_.valid() ||
        !ex_.valid() ||
        !ey_.valid() ||
        !cx_.valid() ||
        !cy_.valid() ||
        !radius_.valid()) return false;

    // point 를 ','로 분리해 x,y 값을 얻는다
    auto token_start = Expression::splitByTopLevelComma(start);
    if (token_start.size() != 2) return false;

    auto token_end = Expression::splitByTopLevelComma(end);
    if (token_end.size() != 2) return false;

    auto token_center = Expression::splitByTopLevelComma(center);
    if (token_center.size() != 2) return false;

    // 시작점
    // sx, sy의 이전값 임시 저장
    const String& psx = sx_->getExpression();
    const String& psy = sy_->getExpression();
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
    if (!ex_->setExpression(token_end[0])
        || !ey_->setExpression(token_end[1]))
    {
        // 실패시 이전값 복원
        ex_->setExpression(pex);
        ey_->setExpression(pey);

        return false;
    }

    // 중심점..
    // mx, my의 이전값 임시 저장
    const String& pmx = cx_->getExpression();
    const String& pmy = cy_->getExpression();
    if (!cx_->setExpression(token_center[0])
        || !cy_->setExpression(token_center[1]))
    {
        // 실패시 이전값 복원
        cx_->setExpression(pmx);
        cy_->setExpression(pmy);

        return false;
    }

    const String& pradius = radius_->getExpression();
    if (!radius_->setExpression(radius))
    {
        // 실패시 이전값 복원
        radius_->setExpression(pradius);

        return false;
    }

    sstart_ = start;
    send_ = end;
    scenter_ = center;
    sradius_ = radius;

    return update();
}

//----------------------------------------------------------------------------
void GeomCurveNode::clearBelongings()
{
    sx_->setExpression("0");
    sy_->setExpression("0");
    ex_->setExpression("0");
    ey_->setExpression("0");
    cx_->setExpression("0");
    cy_->setExpression("0");
    radius_->setExpression("0");

    sx_ = nullptr;
    sy_ = nullptr;
    ex_ = nullptr;
    ey_ = nullptr;
    cx_ = nullptr;
    cy_ = nullptr;
    radius_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomCurveNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    if (!sx_.valid() ||
        !sy_.valid() ||
        !ex_.valid() ||
        !ey_.valid() ||
        !cx_.valid() ||
        !cy_.valid() ||
        !radius_.valid()) return false;

    float64 sx = sx_->eval();
    float64 sy = sy_->eval();

    float64 ex = ex_->eval();
    float64 ey = ey_->eval();

    float64 cx = cx_->eval();
    float64 cy = cy_->eval();

    float64 radii = radius_->eval();

    if (sx == ex && ex == cx && sy == ey && ey == cy) {
        return true;
    }

    Point_2 ss = transform(Point_2(sx, sy));
    Point_2 ee = transform(Point_2(ex, ey));
    Point_2 cc = transform(Point_2(cx, cy));

    Traits_2 traits;
    Traits_2::Point_2 source, target;
    bool set_source = false;
    bool set_target = false;
    Circle_2 circle(cc, radii * radii);
    {
        Arrangement arr;
        CGAL::insert(arr, circle);

        auto vec2 = (ss - cc) * 1.01;
        Point_2 ss2(vec2.x() + cc.x(), vec2.y() + cc.y());
        X_monotone_curve_2 line(cc, ss2);
        CGAL::insert(arr, line);
        float64 error = 0;
        for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
            if (vit->degree() > 1) { // Intersection points have degree > 1
                set_source = true;
                float64 dx = CGAL::to_double(ss.x() - vit->point().x());
                float64 dy = CGAL::to_double(ss.y() - vit->point().y());
                float64 err = std::sqrt(dx * dx + dy * dy);
                if (error == 0 || (error != 0 && err < error)) {
                    source = vit->point();
                    error = err;
                }
            }
        }
    }
    {
        Arrangement arr;
        CGAL::insert(arr, circle);

        auto vec2 = (ee - cc) * 1.01;
        Point_2 ss2(vec2.x() + cc.x(), vec2.y() + cc.y());
        X_monotone_curve_2 line(cc, ss2);
        CGAL::insert(arr, line);
        float64 error = 0;
        for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
            if (vit->degree() > 1) { // Intersection points have degree > 1
                set_target = true;
                float64 dx = CGAL::to_double(ee.x() - vit->point().x());
                float64 dy = CGAL::to_double(ee.y() - vit->point().y());
                float64 err = std::sqrt(dx * dx + dy * dy);
                if (error == 0 || (error != 0 && err < error)) {
                    target = vit->point();
                    error = err;
                }
            }
        }
    }

    Curve_2 curve;
    if (set_source && set_target) {
        curve = Curve_2(circle, source, target);
    }

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
void GeomCurveNode::bindProperty()
{
    BIND_PROPERTY(const String&, StartPoint, &setStartPoint, &getStartPoint);
    BIND_PROPERTY(const String&, EndPoint, &setEndPoint, &getEndPoint);
    BIND_PROPERTY(const String&, CenterPoint, &setCenterPoint, &getCenterPoint);
    BIND_PROPERTY(const String&, Radius, &setRadius, &getRadius);
}
