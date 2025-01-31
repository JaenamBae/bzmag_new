#include "Transient.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void Transient_v_setStopTime_s(Transient* self, Parameter* param)
{
    self->setStopTime(param->in()->get<String>(0));
}

//----------------------------------------------------------------------------
static void Transient_v_setTimeStep_s(Transient* self, Parameter* param)
{
    self->setTimeStep(param->in()->get<String>(0));
}

//----------------------------------------------------------------------------
void Transient::bindMethod()
{
    BIND_METHOD(v_setStopTime_s, Transient_v_setStopTime_s);
    BIND_METHOD(v_setTimeStep_s, Transient_v_setTimeStep_s);
}
