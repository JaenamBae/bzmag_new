#include "GeomRectNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"
#include "Expression.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomRectNode, GeomPrimitiveNode);

//----------------------------------------------------------------------------
GeomRectNode::GeomRectNode() : px_(nullptr), py_(nullptr), width_(nullptr), height_(nullptr),
    spoint_("0,0"), swidth_("0"), sheight_("0")
{
    uint32 key = getID();

    px_ = new Expression();
    py_ = new Expression();
    width_ = new Expression();
    height_ = new Expression();

    px_->setKey("px_" + std::to_string(key));
    py_->setKey("py_" + std::to_string(key));
    width_->setKey("width_" + std::to_string(key));
    height_->setKey("height_" + std::to_string(key));
}

//----------------------------------------------------------------------------
GeomRectNode::~GeomRectNode()
{

}

//----------------------------------------------------------------------------
bool GeomRectNode::setParameters(const String& point,
    const String& dx,
    const String& dy)
{
    if (!px_.valid() ||
        !py_.valid() ||
        !width_.valid() ||
        !height_.valid()) return false;

    // 이전값 임시 저장
    const String& ppx = px_->getExpression();
    const String& ppy = py_->getExpression();
    const String& pdx = width_->getExpression();
    const String& pdy = height_->getExpression();

    // point 를 ','로 분리해 x,y 값을 얻는다
    auto token = Expression::splitByTopLevelComma(point);

    // (x,y) 로 분리되지 않으면 실패
    if (token.size() != 2) return false;

    // 분리된 경우 x,y 값(스트링) 저장
    String px = token[0];
    String py = token[1];

    // 일단 Expression으로 변환 시도
    if (!px_->setExpression(px) ||
        !py_->setExpression(py) ||
        !width_->setExpression(dx) ||
        !height_->setExpression(dy))
    {
        // 실패시 이전값 복원
        px_->setExpression(ppx);
        py_->setExpression(ppy);
        width_->setExpression(pdx);
        height_->setExpression(pdy);

        return false;
    }

    // 맴버 새로운 값으로 업데이트
    spoint_ = point;
    swidth_ = dx;
    sheight_ = dy;

    return update();
}

//----------------------------------------------------------------------------
void bzmag::engine::GeomRectNode::clearBelongings()
{
    px_->setExpression("0");
    py_->setExpression("0");
    width_->setExpression("0");
    height_->setExpression("0");

    px_ = nullptr;
    py_ = nullptr;
    width_ = nullptr;
    height_ = nullptr;
}

//----------------------------------------------------------------------------
bool GeomRectNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    if (!px_.valid() ||
        !py_.valid() ||
        !width_.valid() ||
        !height_.valid()) return false;

    // Construct the polygon.
    float64 px = px_->eval();
    float64 py = py_->eval();
    float64 dx = width_->eval();
    float64 dy = height_->eval();

    if (dx == 0 || dy == 0) {
        return true;
    }

    // 반시계방향으로 포인트 생성
    Point_2 p1(px, py);
    Point_2 p2(px + dx, py);
    Point_2 p3(px + dx, py + dy);
    Point_2 p4(px, py + dy);
    if ((dx < 0 && dy > 0) || (dx > 0 && dy < 0)) {
        std::swap(p2, p4);
    }

    Polygon_2 poly;
    Rectangle_2 rect(p1, p3);
    if (!rect.is_degenerate()) {
        p1 = transform(p1);
        p2 = transform(p2);
        p3 = transform(p3);
        p4 = transform(p4);

        X_monotone_curve_2 s1(p1, p2);    poly.push_back(s1);
        X_monotone_curve_2 s2(p2, p3);    poly.push_back(s2);
        X_monotone_curve_2 s3(p3, p4);    poly.push_back(s3);
        X_monotone_curve_2 s4(p4, p1);    poly.push_back(s4);

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
void GeomRectNode::bindProperty()
{
    BIND_PROPERTY(const String&, Point, &setPoint, &getPoint);
    BIND_PROPERTY(const String&, Width, &setWidth, &getWidth);
    BIND_PROPERTY(const String&, Height, &setHeight, &getHeight);
}
