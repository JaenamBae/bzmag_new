#include "DataSetNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void DataSetNode_n_setDataSet_s(DataSetNode* self, Parameter* param)
{
    String data = param->in()->get<String>(0);
    self->setDataset(data);
}

//----------------------------------------------------------------------------
void DataSetNode::bindMethod()
{
    BIND_METHOD(n_setDataSet_s, DataSetNode_n_setDataSet_s);
}
