#include "GeomToPath.h"
#include "GeomHeadNode.h"
#include "BCNode.h"

using namespace bzmag;
using namespace bzmag::engine;


//----------------------------------------------------------------------------
GeomToPath::GeomToPath(GeomHeadNode* render_node) :
    num_segments_(360),
    render_node_(render_node), bc_node_(nullptr)
{

}

//----------------------------------------------------------------------------
GeomToPath::GeomToPath(BCNode* bc_node) :
    num_segments_(360),
    render_node_(nullptr), bc_node_(bc_node)
{

}

//----------------------------------------------------------------------------
GeomToPath::~GeomToPath()
{

}

//----------------------------------------------------------------------------
void GeomToPath::setNormalDeviation(uint32 num)
{
    num_segments_ = num;
}

//----------------------------------------------------------------------------
uint32 GeomToPath::getNormalDeviation() const
{
    return num_segments_;
}

//----------------------------------------------------------------------------
bool GeomToPath::makePath(VertexList& path)
{
    // initialize
    path.clear();

    if (render_node_) {
        // Object
        if (render_node_->isCovered())
        {
            const Polygon_set_2& geometry = render_node_->getPolyset();
            std::list<Polygon_with_holes_2> res;
            geometry.polygons_with_holes(std::back_inserter(res));

            std::list<Polygon_with_holes_2>::iterator it;

            for (it = res.begin(); it != res.end(); ++it)
            {
                Polygon_with_holes_2 pwh = *it;
                Polygon_2 poly_outer = pwh.outer_boundary();
                makeBoundary(poly_outer, path);

                Polygon_with_holes_2::Hole_iterator hi;
                for (hi = pwh.holes_begin(); hi != pwh.holes_end(); ++hi)
                {
                    Polygon_2 poly_hole = *(hi);
                    makeBoundary(poly_hole, path);
                }
            }

            return true;
        }

        // Curve
        else
        {
            const GeomHeadNode::Curves& curve = render_node_->getCurves();
            GeomHeadNode::Curves::const_iterator i;
            for (i = curve.begin(); i != curve.end(); ++i) {
                const X_monotone_curve_2& edge = *i;
                makeBoundary(edge, path, i == curve.begin());
            }

            return false;
        }
    }
    else if (bc_node_) {
        const BCNode::Curves& curve = bc_node_->getCurves();
        BCNode::Curves::const_iterator i;
        for (i = curve.begin(); i != curve.end(); ++i) {
            const X_monotone_curve_2& edge = *i;
            makeBoundary(edge, path, i == curve.begin());
        }

        return true;
    }


    return false;
}

//----------------------------------------------------------------------------
bool GeomToPath::makeEdgePath(uint32 edgeID, VertexList& path)
{
    const GeomHeadNode::Curves& curves = render_node_->getCurves();
    if (edgeID > curves.size()) return false;

    const X_monotone_curve_2& edge = curves[edgeID];
    makeBoundary(edge, path, true);

    return true;
}

//----------------------------------------------------------------------------
void GeomToPath::makeBoundary(Polygon_2& p, VertexList& path)
{
    //const float64 CGAL_PI = atan(1) * 4;

    VertexInfo p1, p2, p3;
    Polygon_2::Curve_const_iterator i;
    for (i = p.curves_begin(); i != p.curves_end(); ++i) {
        X_monotone_curve_2 edge = *i;
        Traits_2::Point_2 source = edge.source();
        Traits_2::Point_2 target = edge.target();

        // 시작점
        p1.x = CGAL::to_double(source.x());
        p1.y = CGAL::to_double(source.y());
        p1.cmd = 1;     // Move To ; 시작점만 Move To 명령어임

        // 끝점
        p2.x = CGAL::to_double(target.x());
        p2.y = CGAL::to_double(target.y());
        p2.cmd = 2;     // Line To

        // 세그멘테이션 점
        p3.cmd = 2;     // Line To

        // 커브가 첫 시작이면 시작점 추가 (이어지는 커브일 수도 있음)
        if (i == p.curves_begin()) path.emplace_back(p1);

        // 커브가 곡선이면 세그멘테이션 점 추가
        if (!edge.is_linear())
        {
            Circle_2 circle = edge.supporting_circle();
            Point_2 center = circle.center();
            VertexInfo cc;
            cc.x = CGAL::to_double(center.x());
            cc.y = CGAL::to_double(center.y());
            float64 radius = sqrt(CGAL::to_double(circle.squared_radius()));

            float64 start_angle = atan2(p1.y - cc.y, p1.x - cc.x);
            float64 end_angle = atan2(p2.y - cc.y, p2.x - cc.x);

            if (start_angle < 0) start_angle += 2 * CGAL_PI;
            if (end_angle < 0)   end_angle += 2 * CGAL_PI;

            int segnum = 0;
            float64 delang = 0;
            if (edge.orientation() == CGAL::COUNTERCLOCKWISE) {
                if (start_angle > end_angle) start_angle -= 2 * CGAL_PI;
            }
            else {
                if (start_angle < end_angle) end_angle -= 2 * CGAL_PI;
            }

            segnum = int(abs(num_segments_ * ((start_angle - end_angle) / (2.0 * CGAL_PI))));
            delang = (end_angle - start_angle) / segnum;

            for (int sn = 1; sn < segnum; ++sn)
            {
                float64 ang = start_angle + sn * delang;
                p3.x = radius * cos(ang) + cc.x;
                p3.y = radius * sin(ang) + cc.y;
                path.emplace_back(p3);
            }
        }

        // 끝점 추가
        path.emplace_back(p2);
    }
}

//----------------------------------------------------------------------------
void GeomToPath::makeBoundary(const X_monotone_curve_2& edge, VertexList& path, bool first/*=true*/)
{
    //const float64 CGAL_PI = atan(1) * 4;

    VertexInfo p1, p2, p3;

    Traits_2::Point_2 source = edge.source();
    Traits_2::Point_2 target = edge.target();

    p1.x = CGAL::to_double(source.x());
    p1.y = CGAL::to_double(source.y());
    p1.cmd = 1;     // Move To ; 시작점만 Move To 명령어임

    p2.x = CGAL::to_double(target.x());
    p2.y = CGAL::to_double(target.y());
    p2.cmd = 2;     // Line To

    p3.cmd = 2;     // Line To

    if (first) path.emplace_back(p1);//  ps.move_to(p1.x, p1.y);
    if (!edge.is_linear())
    {
        Circle_2 circle = edge.supporting_circle();
        Point_2 center = circle.center();
        VertexInfo cc;
        cc.x = CGAL::to_double(center.x());
        cc.y = CGAL::to_double(center.y());
        float64 radius = sqrt(CGAL::to_double(circle.squared_radius()));

        float64 start_angle = atan2(p1.y - cc.y, p1.x - cc.x);
        float64 end_angle = atan2(p2.y - cc.y, p2.x - cc.x);

        if (start_angle < 0) start_angle += 2 * CGAL_PI;
        if (end_angle < 0)   end_angle += 2 * CGAL_PI;

        int segnum = 0;
        float64 delang = 0;
        if (edge.orientation() == CGAL::COUNTERCLOCKWISE) {
            if (start_angle > end_angle) start_angle -= 2 * CGAL_PI;
        }
        else {
            if (start_angle < end_angle) end_angle -= 2 * CGAL_PI;
        }

        segnum = int(abs(num_segments_ * ((start_angle - end_angle) / (2.0 * CGAL_PI))));
        delang = (end_angle - start_angle) / segnum;

        for (int sn = 1; sn < segnum; ++sn)
        {
            float64 ang = start_angle + sn * delang;
            p3.x = radius * cos(ang) + cc.x;
            p3.y = radius * sin(ang) + cc.y;
            path.emplace_back(p3);
            //ps.line_to(p3.x, p3.y);
        }
    }
    path.emplace_back(p2);
    //ps.line_to(p2.x, p2.y);
}

