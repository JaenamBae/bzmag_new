#include "GeomCircleNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomCircleNode, GeomPrimitiveNode);

//----------------------------------------------------------------------------
GeomCircleNode::GeomCircleNode() : cx_(nullptr), cy_(nullptr), radii_(nullptr), segs_(nullptr),
    scenter_("0,0"), sradii_("0"), ssegs_("0")
{
    uint32 key = getID();

    cx_ = new Expression();
    cy_ = new Expression();
    radii_ = new Expression();
    segs_ = new Expression();

    cx_->setKey("cx_" + std::to_string(key));
    cy_->setKey("cy_" + std::to_string(key));
    radii_->setKey("radii_" + std::to_string(key));
    segs_->setKey("segs_" + std::to_string(key));
}

//----------------------------------------------------------------------------
GeomCircleNode::~GeomCircleNode()
{

}

//----------------------------------------------------------------------------
bool GeomCircleNode::setParameters(const String& center, const String& radius, const String& segs)
{
    if (!cx_.valid() || 
        !cy_.valid() || 
        !radii_.valid() || 
        !segs_.valid()) return false;

    // 이전값 임시 저장
    const String& pcx = cx_->getExpression();
    const String& pcy = cy_->getExpression();
    const String& pradii = radii_->getExpression();
    const String& psegs = segs_->getExpression();

    // center 를 ','로 분리해 x,y 값을 얻는다
    auto token = Expression::splitByTopLevelComma(center);

    // (x,y) 로 분리되지 않으면 실패
    if (token.size() != 2) return false;

    // 분리된 경우 x,y 값(스트링) 저장
    String cx = token[0];
    String cy = token[1];

    // 일단 Expression으로 변환 시도
    if (!cx_->setExpression(cx)
        || !cy_->setExpression(cy)
        || !radii_->setExpression(radius)
        || !segs_->setExpression(segs))
    {
        cx_->setExpression(pcx);
        cy_->setExpression(pcy);
        radii_->setExpression(pradii);
        segs_->setExpression(psegs);

        return false;
    }

    // 맴버 새로운 값으로 업데이트
    scenter_ = center;
    sradii_ = radius;
    ssegs_ = segs;

    return update();
}

//----------------------------------------------------------------------------
void GeomCircleNode::clearBelongings()
{
    cx_->setExpression("0");
    cy_->setExpression("0");
    radii_->setExpression("0");
    segs_->setExpression("0");

    cx_ = nullptr;
    cy_ = nullptr;
    radii_ = nullptr;
    segs_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomCircleNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    if (!cx_.valid() ||
        !cy_.valid() ||
        !radii_.valid() ||
        !segs_.valid()) return false;

    float64 cx = cx_->eval();
    float64 cy = cy_->eval();
    float64 radii = radii_->eval();
    int32 segs = (int32)(segs_->eval());

    if (radii == 0) {
        return true;
    }

    Polygon_2 poly;
    // 세그먼트 수가 3보다 작으면 완전원으로 표현
    if (segs < 3) {
        Point_2 center = transform(Point_2(cx, cy));
        Circle_2 circle(center, radii * radii);
        if (!circle.is_degenerate()) {
            //*
            // for CGAL 5.x
            // Subdivide the circle into two x-monotone arcs.
            Traits_2 traits;
            Curve_2 curve(circle);
            std::list<CGAL::Object>  objects;
            traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
            CGAL_assertion(objects.size() == 2);

            // Insert arcs to the curves_
            X_monotone_curve_2 arc;
            std::list<CGAL::Object>::iterator iter;

            
            for (iter = objects.begin(); iter != objects.end(); ++iter) {
                CGAL::assign(arc, *iter);
                poly.push_back(arc);
            }
            polyset.insert(poly);
            /*/

            // for CGAL 6.0
            // Subdivide the circle into two x-monotone arcs and construct the polygon
            Traits_2 traits;
            Curve_2 curve(circle);
            Polygon_2 poly;

            traits.make_x_monotone_2_object() (curve,
                CGAL::dispatch_or_drop_output<X_monotone_curve_2>(std::back_inserter(poly)));
            assert(poly.size() == 2);
            polyset.insert(poly);
            //*/
        }
    }
    else {
        Point_2 p0(radii + cx, cy);
        Point_2 p1 = p0;
        Point_2 p2;
        for (int32 i = 1; i < segs; ++i) {
            p2 = Point_2(radii * cos(2.0 * CGAL_PI * (i / (float64)segs)) + cx,
                radii * sin(2.0 * CGAL_PI * (i / (float64)segs)) + cy);

            Point_2 p1_trans = transform(p1);
            Point_2 p2_trans = transform(p2);

            X_monotone_curve_2 s(p1_trans, p2_trans);
            poly.push_back(s);
            p1 = p2;
        }
        Point_2 p1_trans = transform(p0);
        Point_2 p2_trans = transform(p2);

        X_monotone_curve_2 s(p2_trans, p1_trans);
        poly.push_back(s);

        polyset.insert(poly);
    }

    for (auto i = poly.curves_begin(); i != poly.curves_end(); ++i) {
        X_monotone_curve_2 arc = *i;
        curves.push_back(arc);

        Traits_2::Point_2 source = arc.source();
        Traits_2::Point_2 target = arc.target();

        vertices.push_back(source);
        vertices.push_back(target);
    }

    return true;
}

//----------------------------------------------------------------------------
void GeomCircleNode::bindProperty()
{
    BIND_PROPERTY(const String&, Center, &setCenter, &getCenter);
    BIND_PROPERTY(const String&, Radius, &setRadius, &getRadius);
    BIND_PROPERTY(const String&, Segments, &setSegments, &getSegments);
}
