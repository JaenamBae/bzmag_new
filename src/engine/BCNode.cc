#include "BCnode.h"
#include "GeomHeadNode.h"
#include "GeomHeadRefNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"
#include <CGAL/Polygon_2.h>
#include <algorithm>
#include <vector>

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(BCNode, Node);

// 동일점 인정 오차
float64 BCNode::tol_ = 1e-3;

//----------------------------------------------------------------------------
BCNode::BCNode()
{

}

//----------------------------------------------------------------------------
BCNode::~BCNode()
{

}

//----------------------------------------------------------------------------
void BCNode::addBoundary(GeomHeadRefNode* ref_head)
{
    heades_.push_back(ref_head);
    update();
}

void BCNode::removeBoundary(GeomHeadRefNode* ref_head)
{
    heades_.erase(std::remove(heades_.begin(), heades_.end(), ref_head), heades_.end());
    update();
}

void BCNode::addCurve(X_monotone_curve_2& curve)
{
    curves_.push_back(curve);
}

//----------------------------------------------------------------------------
// General Polygon을 이루는 커브에 대한 테스트
int BCNode::testCurve(const X_monotone_curve_2& test_curve) const
{
    Traits_2::Point_2 source = test_curve.source();
    Traits_2::Point_2 target = test_curve.target();

    Vert ss{ CGAL::to_double(source.x()), CGAL::to_double(source.y()) };
    Vert tt{ CGAL::to_double(target.x()), CGAL::to_double(target.y()) };

    Curves::const_iterator it;
    for (it = curves_.cbegin(); it != curves_.cend(); ++it) {
        const X_monotone_curve_2& curve = *it;
        if (testVertex(ss, curve) && testVertex(tt, curve))
        {
            // 둘다 직선이면 경계라인 상에 test_curve가 존재하는 것임
            if (curve.is_linear() && test_curve.is_linear()) {

                Vert ss2{ CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y())};
                Vert tt2{ CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()) };

                Vert v1 = tt - ss;
                Vert v2 = tt2 - ss2;

                // Calculate dot product
                double dot_product = CGAL::to_double(v1.x() * v2.x() + v1.y() * v2.y());

                if (dot_product > 0) {
                    return 1;
                }
                else {
                    return -1;
                }
            }

            // 둘다 곡선이라면 원점 비교까지 필요함
            if (curve.is_circular() && test_curve.is_circular()) {
                Point_2 c_curve = curve.supporting_circle().center();
                Point_2 t_curve = test_curve.supporting_circle().center();

                Vert cc_curve(CGAL::to_double(c_curve.x()), CGAL::to_double(c_curve.y()));
                Vert tt_curve(CGAL::to_double(t_curve.x()), CGAL::to_double(t_curve.y()));
                float64 dx = cc_curve[0] - tt_curve[0];
                float64 dy = cc_curve[1] - tt_curve[1];

                float64 delta = std::sqrt(dx*dx + dy*dy);
                if (delta < tol_) {
                    if (test_curve.orientation() == curve.orientation()) {
                        return 1;
                    }
                    else {
                        return -1;
                    }
                }
            }

        }
    }
    return 0;
}

//----------------------------------------------------------------------------
// 세그멘테이션 후의 직선에 대한 테스트
bool BCNode::testSegment(const Vert& v1, const Vert& v2) const
{
    Curves::const_iterator it;
    for (it = curves_.cbegin(); it != curves_.cend(); ++it) {
        const X_monotone_curve_2& curve = *it;
        if (testVertex(v1, curve) && testVertex(v2, curve))
        {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
bool BCNode::testVertex(const Vert& pt, const X_monotone_curve_2& curve)
{
    Traits_2::Point_2 source = curve.source();
    Traits_2::Point_2 target = curve.target();

    Vert v_pt(pt);
    Vert v_ss(CGAL::to_double(source.x()), CGAL::to_double(source.y()));
    Vert v_tt(CGAL::to_double(target.x()), CGAL::to_double(target.y()));

    // 커브의 시작점 끝점 동일 시 실패 --> 쓰래기커브라고 판단
    float64 diff_x = (v_ss.x_ - v_tt.x_);
    float64 diff_y = (v_ss.y_ - v_tt.y_);
    float64 torr = std::sqrt(diff_x * diff_x + diff_y * diff_y);
    if (torr < tol_) {
        return false;
    }

    // 직선인 경우
    if (curve.is_linear()) {
        // 커브의 직선벡터
        Vert ab{ v_tt.x_ - v_ss.x_, v_tt.y_ - v_ss.y_ };

        // 시작점에서 주어진 점으로 가는 벡터
        Vert ac{ v_pt.x_ - v_ss.x_, v_pt.y_ - v_ss.y_ };

        // 정규화
        float64 norm_ab = std::sqrt(ab.x_ * ab.x_ + ab.y_ * ab.y_);
        float64 norm_ac = std::sqrt(ac.x_ * ac.x_ + ac.y_ * ac.y_);
        Vert normalized_ab{ ab.x_ / norm_ab, ab.y_ / norm_ab };
        Vert normalized_ac{ ac.x_ / norm_ac, ac.y_ / norm_ac };

        // 외적을 구해서 0이면 동일선상에 존재하는 것임
        double cross_product = normalized_ab.x_ * normalized_ac.y_ - normalized_ab.y_ * normalized_ac.x_;
        
        // 일단 동일선상에 있고
        if (std::abs(cross_product) > tol_) return false;
        
        
        // 시작점과 끝점 사이에 존재하는지 판단
        return
            (std::min(v_ss.x_, v_tt.x_) <= v_pt.x_) &&
            (v_pt.x_ <= std::max(v_ss.x_, v_tt.x_)) &&
            (std::min(v_ss.y_, v_tt.y_) <= v_pt.y_) &&
            (v_pt.y_ <= std::max(v_ss.y_, v_tt.y_));
    }
    // 커브인 경우
    else {
        // 커브의 Supporting Circle 추출
        Circle_2 circle = curve.supporting_circle();

        // Supporting Circle의 반지름, 중심점 추출
        float64 squared_radii = CGAL::to_double(circle.squared_radius());
        float64 radii = std::sqrt(squared_radii);
        Point_2 org = circle.center();
        Vert center{ CGAL::to_double(org.x()),CGAL::to_double(org.y()) };

        // 원의 방정식: (x - center.x)^2 + (y - center.y)^2 = radius^2
        double quared_distance = std::pow(v_pt.x_ - center.x_, 2) + std::pow(v_pt.y_ - center.y_, 2);

        // 거리 제곱과 반지름 제곱이 거의 같은지 확인
        if (std::abs(quared_distance - squared_radii) > tol_) return false;

        // 점이 원 위에 있으면 각도를 계산
        double angle       = std::atan2(v_pt.y_ - center.y_, v_pt.x_ - center.x_);
        double start_angle = std::atan2(v_ss.y_ - center.y_, v_ss.x_ - center.x_);
        double end_angle   = std::atan2(v_tt.y_ - center.y_, v_tt.x_ - center.x_);

        // 시작 각도와 끝 각도를 정규화(0~2pi)
        if (angle < 0)       angle       += (2 * CGAL_PI);
        if (start_angle < 0) start_angle += (2 * CGAL_PI);
        if (end_angle < 0)   end_angle   += (2 * CGAL_PI);

        // 각도가 원호 범위 안에 있는지 확인
        if (curve.orientation() == CGAL::COUNTERCLOCKWISE) {
            if (angle >= start_angle && angle <= end_angle) return true;
            else return false;
        }
        else {
            if (angle >= end_angle && angle <= start_angle) return true;
            else return false;
        }
    }
}

//----------------------------------------------------------------------------
const BCNode::Curves& BCNode::getCurves()
{
    return curves_;
}

//----------------------------------------------------------------------------
bool BCNode::update()
{
    curves_.clear();
    for (auto it = heades_.begin(); it != heades_.end(); ++it) {
        GeomHeadRefNode* ref_head = *it;
        if (ref_head) {
            GeomHeadNode* head = ref_head->getHeadNode();
            if (head) {
                const GeomHeadNode::Curves& curves = head->getCurves();
                curves_.insert(curves_.end(), curves.begin(), curves.end());
            }
        }
    }
    return true;
}

//----------------------------------------------------------------------------
void BCNode::onAttachTo(Node* parent)
{

}


//----------------------------------------------------------------------------
void BCNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void BCNode::bindProperty()
{

}

