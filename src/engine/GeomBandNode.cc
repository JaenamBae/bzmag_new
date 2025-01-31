#include "GeomBandNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomBandNode, GeomPrimitiveNode);

//----------------------------------------------------------------------------
GeomBandNode::GeomBandNode() : cx_(nullptr), cy_(nullptr), radii_(nullptr), width_(nullptr), segs_(nullptr),
    scenter_("0,0"), sradii_("0"), swidth_("0")
{
    uint32 key = getID();

    cx_ = new Expression();
    cy_ = new Expression();
    radii_ = new Expression();
    width_ = new Expression();
    segs_ = new Expression();

    cx_->setKey("cx_" + std::to_string(key));
    cy_->setKey("cy_" + std::to_string(key));
    radii_->setKey("radii_" + std::to_string(key));
    width_->setKey("width_" + std::to_string(key));
    segs_->setKey("segs_" + std::to_string(key));
}

//----------------------------------------------------------------------------
GeomBandNode::~GeomBandNode()
{

}

//----------------------------------------------------------------------------
bool GeomBandNode::setParameters(const String& center, const String& radius, const String& width, const String& segs)
{
    if (!cx_.valid() ||
        !cy_.valid() ||
        !radii_.valid() ||
        !width_.valid() ||
        !segs_.valid()) return false;

    // 이전값 임시 저장
    const String& pcx = cx_->getExpression();
    const String& pcy = cy_->getExpression();
    const String& pradii = radii_->getExpression();
    const String& pwidth = width_->getExpression();
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
        || !width_->setExpression(width)
        || !segs_->setExpression(segs))
    {
        cx_->setExpression(pcx);
        cy_->setExpression(pcy);
        radii_->setExpression(pradii);
        width_->setExpression(pwidth);
        segs_->setExpression(psegs);

        return false;
    }

    // 맴버 새로운 값으로 업데이트
    scenter_ = center;
    sradii_ = radius;
    swidth_ = width;
    ssegs_ = segs;

    return update();
}

//----------------------------------------------------------------------------
void bzmag::engine::GeomBandNode::clearBelongings()
{
    cx_->setExpression("0");
    cy_->setExpression("0");
    radii_->setExpression("0");
    width_->setExpression("0");
    segs_->setExpression("0");

    cx_ = nullptr;
    cy_ = nullptr;
    radii_ = nullptr;
    width_ = nullptr;
    segs_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomBandNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    if (!cx_.valid() ||
        !cy_.valid() ||
        !radii_.valid() ||
        !width_.valid() ||
        !segs_.valid()) return false;

    float64 cx = cx_->eval();
    float64 cy = cy_->eval();
    float64 radii = abs(radii_->eval());
    float64 width = width_->eval();

    if (width <= 0 || radii == 0) {
        return true;
    }

    int32 segs = (int32)(segs_->eval());

    float64 oradii = radii + width * 0.5;
    float64 iradii = radii - width * 0.5;


    // 세그먼트 수가 3보다 작으면 완전원으로 표현
    Polygon_2 poly_o;
    Polygon_2 poly_i;
    if (segs < 3) {
        Point_2 center = transform(Point_2(cx, cy));
        Circle_2 circle_o(center, oradii * oradii);
        Circle_2 circle_i(center, iradii * iradii);

        if (!circle_o.is_degenerate() && !circle_i.is_degenerate()) {
            /*
            // for CGAL 6.x
            // 바같 원
            // Subdivide the circle into two x-monotone arcs and construct the polygon
            Traits_2 traits;
            Curve_2 curve_o(circle_o);
            Polygon_2 poly_o;

            traits.make_x_monotone_2_object() (curve_o,
                CGAL::dispatch_or_drop_output<X_monotone_curve_2>(std::back_inserter(poly_o)));
            assert(poly_o.size() == 2);

            // 안쪽 원
            Curve_2 curve_i(circle_i);
            Polygon_2 poly_i;

            traits.make_x_monotone_2_object() (curve_i,
                CGAL::dispatch_or_drop_output<X_monotone_curve_2>(std::back_inserter(poly_i)));
            assert(poly_i.size() == 2);
            /*/
            //for CGAL 5.x
            X_monotone_curve_2 arc;
            std::list<CGAL::Object>::iterator iter;
            Traits_2 traits;

            // outer circle
            Curve_2 curve_o(circle_o);
            std::list<CGAL::Object>  objects;
            traits.make_x_monotone_2_object() (curve_o, std::back_inserter(objects));
            CGAL_assertion(objects.size() == 2);
            
            for (iter = objects.begin(); iter != objects.end(); ++iter) {
                CGAL::assign(arc, *iter);
                poly_o.push_back(arc);
            }

            // inner circle
            objects.clear();
            Curve_2 curve_i(circle_i);
            traits.make_x_monotone_2_object() (curve_i, std::back_inserter(objects));
            CGAL_assertion(objects.size() == 2);
            // Insert arcs to the curves
            for (iter = objects.begin(); iter != objects.end(); ++iter) {
                CGAL::assign(arc, *iter);
                poly_i.push_back(arc);
            }
            //*/

            // 바깥 원에서 안쪽 원 빼기
            polyset.insert(poly_o);
            polyset.difference(poly_i);
        }
    }
    else {
        // outside
        Point_2 p1(oradii, 0);
        Point_2 p2;
        for (int32 i = 1; i < segs; ++i) {
            p2 = Point_2(oradii * cos(2 * CGAL_PI * (i / (float64)segs)), oradii * sin(2 * CGAL_PI * (i / (float64)segs)));

            Point_2 p1_trans = transform(p1);
            Point_2 p2_trans = transform(p2);

            X_monotone_curve_2 so(p1_trans, p2_trans);
            poly_o.push_back(so);
            p1 = p2;
        }
        p1 = Point_2(oradii, 0);
        Point_2 p1_trans = transform(p1);
        Point_2 p2_trans = transform(p2);

        X_monotone_curve_2 so(p2, p1);
        poly_o.push_back(so);

        polyset.insert(poly_o);

        // inside
        p1 = Point_2(iradii, 0);
        for (int32 i = 1; i < segs; ++i) {
            p2 = Point_2(iradii * cos(2 * CGAL_PI * (i / (float64)segs)), iradii * sin(2 * CGAL_PI * (i / (float64)segs)));

            Point_2 p1_trans = transform(p1);
            Point_2 p2_trans = transform(p2);

            X_monotone_curve_2 si(p1_trans, p2_trans);
            poly_i.push_back(si);
            p1 = p2;
        }
        p1 = Point_2(iradii, 0);
        p1_trans = transform(p1);
        p2_trans = transform(p2);

        X_monotone_curve_2 si(p2_trans, p1_trans);
        poly_i.push_back(si);

        // 바깥 원에서 안쪽 원 빼기
        polyset.difference(poly_i);
    }

    for (auto i = poly_o.curves_begin(); i != poly_o.curves_end(); ++i) {
        X_monotone_curve_2 arc = *i;
        curves.push_back(arc);

        Traits_2::Point_2 source = arc.source();
        Traits_2::Point_2 target = arc.target();

        vertices.push_back(source);
        vertices.push_back(target);
    }

    for (auto i = poly_i.curves_begin(); i != poly_i.curves_end(); ++i) {
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
void GeomBandNode::bindProperty()
{
    BIND_PROPERTY(const String&, Center, &setCenter, &getCenter);
    BIND_PROPERTY(const String&, Radius, &setRadius, &getRadius);
    BIND_PROPERTY(const String&, Width, &setWidth, &getWidth);
    BIND_PROPERTY(const String&, Segments, &setSegments, &getSegments);
}
