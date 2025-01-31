#include "Transient.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"
#include "core/methodbinder.h"
#include "Expression.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(Transient, Node);

//----------------------------------------------------------------------------
Transient::Transient() : stop_time_(nullptr), time_step_(nullptr)
{
    uint32 key = getID();

    stop_time_ = new Expression();
    time_step_ = new Expression();
    stop_time_->setKey("stop_time_" + std::to_string(key));
    time_step_->setKey("time_step_" + std::to_string(key));

    stop_time_->setExpression("0");
    time_step_->setExpression("0");
}

//----------------------------------------------------------------------------
Transient::~Transient()
{

}

const String& Transient::getTimeStep() const
{
    return time_step_->getExpression();
}

void Transient::setTimeStep(const String& time_step)
{
    // 일단 Expression으로 변환 시도
    const String& prev = time_step_->getExpression();
    if (!time_step_->setExpression(time_step)) {
        // 실패하면 원상복귀
        time_step_->setExpression(prev);
        return;
    }
}

const String& Transient::getStopTime() const
{
    return stop_time_->getExpression();
}

void Transient::setStopTime(const String& stop_time)
{
    // 일단 Expression으로 변환 시도
    const String& prev = stop_time_->getExpression();
    if (!stop_time_->setExpression(stop_time)) {
        // 실패하면 원상복귀
        stop_time_->setExpression(prev);
        return;
    }
}

float64 Transient::evalTimeStep()
{
    return time_step_->eval();
}

float64 Transient::evalStopTime()
{
    return stop_time_->eval();
}

bool Transient::update()
{
    return false;
}

void Transient::onAttachTo(Node* parent)
{
}

void Transient::onDetachFrom(Node* parent)
{
}

void Transient::clearBelongings()
{
    time_step_->setExpression("0");
    stop_time_->setExpression("0");

    time_step_ = nullptr;
    stop_time_ = nullptr;
}

void Transient::bindProperty()
{
    BIND_PROPERTY(const String&, TimeStep, &setTimeStep, &getTimeStep);
    BIND_PROPERTY(const String&, StopTime, &setStopTime, &getStopTime);
}
