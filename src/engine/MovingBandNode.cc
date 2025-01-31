#include "MovingBandNode.h"
#include "GeomHeadRefNode.h"
#include "GeomHeadNode.h"
#include "GeomToTriangle.h"
#include "Expression.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(MovingBandNode, BCNode);


//----------------------------------------------------------------------------
MovingBandNode::MovingBandNode() : master_(nullptr), slave_(nullptr),
    outer_line_(nullptr), inner_line_(nullptr), outer_(nullptr), inner_(nullptr), speed_(nullptr)
{
    // 무빙밴드가 만들어지려면 밴드기준 좌우로 공극영역이 만들어져야하고
    // 밴드라인이 만들어져야 함
    // 각 영역은 고유 ID가 필요하며 이를 위해 BCNode의 ID를 활용할 것임
    outer_line_ = new BCNode();
    inner_line_ = new BCNode();
    outer_ = new Object();
    inner_ = new Object();

    outer_line_->setName("OuterLine");
    inner_line_->setName("InnerLine");

    uint32 key = getID();

    speed_ = new Expression();
    speed_->setKey("speed_" + std::to_string(key));
    speed_->setExpression("0");

    initial_pos_ = new Expression();
    initial_pos_->setKey("init_pos_" + std::to_string(key));
    initial_pos_->setExpression("0");
}

//----------------------------------------------------------------------------
MovingBandNode::~MovingBandNode()
{

}

bool MovingBandNode::checkValid()
{
    // 참조하는 헤더노드는 1개만 존재해야 함
    if (heades_.size() != 1) {
        return false;
    }

    master_ = nullptr;
    slave_ = nullptr;

    std::map<float64, std::list<X_monotone_curve_2>> curves;

    // MovingBand가 가진 커브들 중에 경계에 포함되는 커브가 있는지 판단
    Node* parent = getParent();
    for (auto& curve : curves_) {
        bool is_bc_curve = false;
        for (auto it = parent->firstChildNode(); it != parent->lastChildNode(); ++it) {
            Node* node = *it;
            if (node->getType()->getName() == "MasterPeriodicBCNode") {
                MasterPeriodicBCNode* master = dynamic_cast<MasterPeriodicBCNode*>(node);
                if (master && master->testCurve(curve) != 0) {
                    master_ = master;
                    is_bc_curve = true;
                    //master_->addCurve(curve);
                    continue;
                }
            }
            else if (node->getType()->getName() == "SlavePeriodicBCNode") {
                SlavePeriodicBCNode* slave = dynamic_cast<SlavePeriodicBCNode*>(node);
                if (slave && slave->testCurve(curve) != 0) {
                    slave_ = slave;
                    if (!slave->checkValid()) {
                        return false;
                    }
                    is_bc_curve = true;
                    //slave_->addCurve(curve);
                    continue;
                }
            }
        }
        if (is_bc_curve) continue;

        if (curves.size() == 0) {
            is_circular_ = curve.is_circular();
        }
        else if (curve.is_circular() != is_circular_) {
            return false;
        }

        if (is_circular_) {
            auto circle = curve.supporting_circle();
            if (circle.center().x() != 0 || circle.center().y() != 0) {
                return false;
            }
            float64 radii = std::sqrt(CGAL::to_double(circle.squared_radius()));
            curves[radii].push_back(curve);
        }
        else {
            if (!curve.supporting_line().is_horizontal()) {
                return false;
            }
            float64 length = CGAL::to_double(curve.source().y());
            curves[length].push_back(curve);
        }
    }

    // 반지름 혹은 y축 좌표가 다른 곡선이 단 두개만 존재해야 함
    if (curves.size() != 2) {
        return false;
    }

    // 주기조건이 존재한다면 마스터/슬레이브 짝이 맞아야 함
    if (slave_ != nullptr) {
        if (master_ == nullptr) {
            return false;
        }
        if (slave_->getPair() != master_) {
            return false;
        }
    }

    // 경계조건에 커브를 추가함
    float64 val1 = curves.begin()->first;
    float64 val2 = curves.rbegin()->first;

    float64 min_length, max_length;
    if (val1 > val2) {
        min_length = val2;
        max_length = val1;
    }
    else {
        min_length = val1;
        max_length = val2;
    }
    airgap_length_ = (max_length - min_length);


    auto ref_head = heades_.front();
    GeomHeadNode* head = ref_head->getHeadNode();
    if (!head) {
        return false;
    }

    // Inner, Outer 영역 및 경계라인 생성
    // update를 통해 outer_line_과 inner_line_에 포함된 커브를 초기화 한다
    outer_line_->update();
    inner_line_->update();

    outer_obj_ = inner_obj_ = head->getPolyset();
    if (is_circular_) {
        // Airgap 영역을 생성함
        float64 delta = (max_length - min_length) / 3.0;
        float64 radii1 = max_length - delta;
        float64 radii2 = min_length + delta;

        // radii1, radii2 원을 생성해서 band - circle1 은 outer
        // band 와 circle2 의 교집합은 inner으로 설정하면 됨
        Point_2 center(0, 0);
        Circle_2 circle1(center, radii1 * radii1);
        Polygon_2 poly1;
        Polygon_set_2 polyset1;
        if (!circle1.is_degenerate()) {
            //*
            // for CGAL 5.x
            // Subdivide the circle into two x-monotone arcs.
            Traits_2 traits;
            Curve_2 curve(circle1);
            std::list<CGAL::Object>  objects;
            traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
            CGAL_assertion(objects.size() == 2);

            // Insert arcs to the curves_
            X_monotone_curve_2 arc;
            std::list<CGAL::Object>::iterator iter;

            for (iter = objects.begin(); iter != objects.end(); ++iter) {
                CGAL::assign(arc, *iter);
                poly1.push_back(arc);
            }
            polyset1.insert(poly1);
        }

        Circle_2 circle2(center, radii2 * radii2);
        Polygon_2 poly2;
        Polygon_set_2 polyset2;
        if (!circle2.is_degenerate()) {
            //*
            // for CGAL 5.x
            // Subdivide the circle into two x-monotone arcs.
            Traits_2 traits;
            Curve_2 curve(circle2);
            std::list<CGAL::Object>  objects;
            traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
            CGAL_assertion(objects.size() == 2);

            // Insert arcs to the curves_
            X_monotone_curve_2 arc;
            std::list<CGAL::Object>::iterator iter;

            for (iter = objects.begin(); iter != objects.end(); ++iter) {
                CGAL::assign(arc, *iter);
                poly2.push_back(arc);
            }
            polyset2.insert(poly2);
        }
        moving_area_ = polyset2;

        // 영역 생성
        outer_obj_.difference(polyset1);
        inner_obj_.intersection(polyset2);

        // outer_line_경계 커브 추가
        {
            std::list<Polygon_with_holes_2> res;
            outer_obj_.polygons_with_holes(std::back_inserter(res));
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                const auto& outer = (*it).outer_boundary();
                for (auto curve = outer.curves_begin(); curve != outer.curves_end(); ++curve) {
                    if (curve->is_circular()) {
                        auto radii = std::sqrt(CGAL::to_double(curve->supporting_circle().squared_radius()));
                        if (radii < max_length) {
                            X_monotone_curve_2 cc = *curve;
                            X_monotone_curve_2 r_cc(cc.supporting_circle(), cc.target(), cc.source(), CGAL::COUNTERCLOCKWISE);
                            outer_line_->addCurve(r_cc);
                        }
                    }
                }
            }
        }

        // inner_line_경계 커브 추가
        {
            std::list<Polygon_with_holes_2> res;
            inner_obj_.polygons_with_holes(std::back_inserter(res));
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                const auto& outer = (*it).outer_boundary();
                for (auto curve = outer.curves_begin(); curve != outer.curves_end(); ++curve) {
                    if (curve->is_circular()) {
                        auto radii = std::sqrt(CGAL::to_double(curve->supporting_circle().squared_radius()));
                        if (radii > min_length) {
                            X_monotone_curve_2 cc = *curve;
                            inner_line_->addCurve(cc);
                        }
                    }
                }
            }
        }
    }
    else {

    }

    return true;
}

const Polygon_set_2& MovingBandNode::getInner() const
{
    return inner_obj_;
}

const Polygon_set_2& MovingBandNode::getOuter() const
{
    return outer_obj_;
}

const Polygon_set_2& MovingBandNode::getMovingArea() const
{
    return moving_area_;
}

int32 MovingBandNode::getInnerAirgapID() const
{
    return inner_->getID();
}

int32 MovingBandNode::getOuterAirgapID() const
{
    return outer_->getID();
}

BCNode* MovingBandNode::getOuterBCNode()
{
    return outer_line_;
}
BCNode* MovingBandNode::getInnerBCNode()
{
    return inner_line_;
}

bool MovingBandNode::isCircular() const
{
    return is_circular_;
}

bool MovingBandNode::isEven() const
{
    if (slave_) {
        return slave_->isEven();
    }
    return false;
}

float64 MovingBandNode::getCircularCoefficient() const
{
    if (slave_) {
        return slave_->getCircularCoefficient();
    }
    return 0;
}

Vector2 MovingBandNode::getLinearCoefficient() const
{
    if (slave_) {
        return slave_->getLinearCoefficient();
    }
    return Vector2();
}

float64 MovingBandNode::getAirgapLength() const
{
    return airgap_length_;
}

const String& MovingBandNode::getSpeed() const
{
    return speed_->getExpression();
}

void MovingBandNode::setSpeed(const String& speed)
{
    // 일단 Expression으로 변환 시도
    const String& prev = speed_->getExpression();
    if (!speed_->setExpression(speed)) {
        // 실패하면 원상복귀
        speed_->setExpression(prev);
        return;
    }
}

float64 MovingBandNode::evalSpeed()
{
    return speed_->eval();
}

const String& MovingBandNode::getInitialPosition() const
{
    return initial_pos_->getExpression();
}

void MovingBandNode::setInitialPosition(const String& speed)
{
    // 일단 Expression으로 변환 시도
    const String& prev = initial_pos_->getExpression();
    if (!initial_pos_->setExpression(speed)) {
        // 실패하면 원상복귀
        initial_pos_->setExpression(prev);
        return;
    }
}

float64 MovingBandNode::evalInitialPosition()
{
    return initial_pos_->eval();
}

GeomHeadNode* MovingBandNode::getReferedHead()
{
    if (heades_.size() > 0) {
        return heades_.front()->getHeadNode();
    }

    return nullptr;
}

//----------------------------------------------------------------------------
bool MovingBandNode::update()
{
    return BCNode::update();
}

//----------------------------------------------------------------------------
void MovingBandNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void MovingBandNode::onDetachFrom(Node* parent)
{

}

void MovingBandNode::clearBelongings()
{
    speed_->setExpression("0");
    speed_ = nullptr;
}

//----------------------------------------------------------------------------
void MovingBandNode::bindProperty()
{
    BIND_PROPERTY(const String&, Speed, &setSpeed, &getSpeed);
    BIND_PROPERTY(const String&, InitialPosition, &setInitialPosition, &getInitialPosition);
}

