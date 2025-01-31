#include "SlavePeriodicBCNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(SlavePeriodicBCNode, MasterPeriodicBCNode);


//----------------------------------------------------------------------------
SlavePeriodicBCNode::SlavePeriodicBCNode() : pair_(nullptr)
{

}

//----------------------------------------------------------------------------
SlavePeriodicBCNode::~SlavePeriodicBCNode()
{

}

//----------------------------------------------------------------------------
MasterPeriodicBCNode* SlavePeriodicBCNode::getPair() const
{
    return pair_;
}

//----------------------------------------------------------------------------
void SlavePeriodicBCNode::setPair(MasterPeriodicBCNode* pair)
{
    pair_ = pair;
}

//----------------------------------------------------------------------------
bool SlavePeriodicBCNode::isEven() const
{
    return even_;
}

//----------------------------------------------------------------------------
void SlavePeriodicBCNode::setEven(bool even)
{
    even_ = even;
}

bool SlavePeriodicBCNode::checkValid()
{
    // 페어가 없으면 실패
    if (pair_ == nullptr) return false;

    // 마스터 슬레이브가 각각 1개의 직선 커브를 가지는지 판단
    const Curves& m_curves = pair_->getCurves();
    if (m_curves.size() != 1 || curves_.size() != 1) {
        return false;
    }
    const X_monotone_curve_2& master = m_curves.front();
    const X_monotone_curve_2& slave = curves_.front();
    if (master.is_circular() || slave.is_circular()) {
        return false;
    }

    // 길이가 같은지 판단
    Vert l1(CGAL::to_double(master.source().x() - master.target().x()),
        CGAL::to_double(master.source().y() - master.target().y()));

    Vert l2(CGAL::to_double(slave.source().x() - slave.target().x()),
        CGAL::to_double(slave.source().y() - slave.target().y()));

    if (abs(l1.length() - l2.length()) > 2 * tol_) return false;

    // 1) 두 곡선의 시작점 사이 거리
    auto m_start = master.source();
    auto s_start = slave.source();
    periodic_vector_.x_ = CGAL::to_double(m_start.x() - s_start.x());
    periodic_vector_.y_ = CGAL::to_double(m_start.y() - s_start.y());

    // 2) 벡터로 만들기
    auto m_end = master.target();
    auto s_end = slave.target();

    // master vector
    auto vx_m = m_end.x() - m_start.x();
    auto vy_m = m_end.y() - m_start.y();

    // slave vector
    auto vx_s = s_end.x() - s_start.x();
    auto vy_s = s_end.y() - s_start.y();

    // 내적
    double dot = CGAL::to_double(vx_m * vx_s + vy_m * vy_s);

    // 마스터/슬레이브 벡터의 외적; 외적이 0이면 선형이동 그렇지 않으면 회전이동이다
    float64 cross = CGAL::to_double(vx_m * vy_s - vy_m * vx_s);
    (cross < tol_) ? is_circular_ = false : is_circular_ = true;

    // 회전기의 경우 마스터/슬레이브의 사이각을 구한다
    if (is_circular_) {
        // 길이(노름)
        double norm_m = std::sqrt(CGAL::to_double(vx_m * vx_m + vy_m * vy_m));
        double norm_s = std::sqrt(CGAL::to_double(vx_s * vx_s + vy_s * vy_s));

        // arccos를 이용해 각도 계산 (라디안)
        periodic_angle_ = std::acos(dot / (norm_m * norm_s));
    }

    // 선형기인 경우 마스터/슬레이브의 이동 벡터를 구한다
    else {
        periodic_vector_ = Vector2(
            CGAL::to_double(master.source().x() - slave.target().x()),
            CGAL::to_double(master.source().y() - slave.target().y()));
    }

    return true;
}

//----------------------------------------------------------------------------
bool SlavePeriodicBCNode::isCircular() const
{
    return is_circular_;
}

//----------------------------------------------------------------------------
float64 SlavePeriodicBCNode::getCircularCoefficient() const
{
    return periodic_angle_;
}

Vector2 SlavePeriodicBCNode::getLinearCoefficient() const
{
    return periodic_vector_;
}

//----------------------------------------------------------------------------
bool SlavePeriodicBCNode::update()
{
    return BCNode::update();
}

//----------------------------------------------------------------------------
void SlavePeriodicBCNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void SlavePeriodicBCNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void SlavePeriodicBCNode::bindProperty()
{
    BIND_PROPERTY(MasterPeriodicBCNode*, Master, &setPair, &getPair);
    BIND_PROPERTY(bool, EvenPeriodic, &setEven, &isEven);
}

