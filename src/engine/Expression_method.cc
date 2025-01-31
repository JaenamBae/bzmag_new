#include "Expression.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void Expression_b_setKey_s(Expression* self, Parameter* param)
{
    bool res = self->setKey(param->in()->get<String>(0));
    param->out()->get<bool>(0) = res;
}

//----------------------------------------------------------------------------
static void Expression_b_setExpression_s(Expression* self, Parameter* param)
{
    bool res = self->setExpression(param->in()->get<String>(0));
    param->out()->get<bool>(0) = res;
}

//----------------------------------------------------------------------------
static void Expression_v_setComment_s(Expression* self, Parameter* param)
{
    self->setComment(param->in()->get<String>(0));
}

//----------------------------------------------------------------------------
static void Expression_d_eval_v(Expression* self, Parameter* param)
{
    param->out()->get<float64>(0) = self->eval();
}

//----------------------------------------------------------------------------
void Expression::bindMethod()
{
    BIND_METHOD(b_setKey_s, Expression_b_setKey_s);
    BIND_METHOD(b_setExpression_s, Expression_b_setExpression_s);
    BIND_METHOD(v_setComment_s, Expression_v_setComment_s);
    BIND_METHOD(d_eval_v, Expression_d_eval_v);
}
