#include "MaterialNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void MaterialNode_n_getDataSetNode_v(MaterialNode* self, Parameter* param)
{
    param->out()->get<DataSetNode*>(0) = self->getDataSetNode();
}

//----------------------------------------------------------------------------
static void MaterialNode_v_setPermeability_s(MaterialNode* self, Parameter* param)
{
    String data = param->in()->get<String>(0);
    self->setPermeability(data);
}

//----------------------------------------------------------------------------
static void MaterialNode_v_setConductivity_s(MaterialNode* self, Parameter* param)
{
    String data = param->in()->get<String>(0);
    self->setConductivity(data);
}

//----------------------------------------------------------------------------
static void MaterialNode_v_setMagnetization_s(MaterialNode* self, Parameter* param)
{
    String data = param->in()->get<String>(0);
    self->setMagnetization(data);
}

//----------------------------------------------------------------------------
static void MaterialNode_v_setDirectionOfMagnetization_s(MaterialNode* self, Parameter* param)
{
    String data = param->in()->get<String>(0);
    self->setDirectionOfMagnetization(data);
}

//----------------------------------------------------------------------------
void MaterialNode::bindMethod()
{
    BIND_METHOD(n_getDataSetNode_v, MaterialNode_n_getDataSetNode_v);

    BIND_METHOD(v_setPermeability_s, MaterialNode_v_setPermeability_s);
    BIND_METHOD(v_setConductivity_s, MaterialNode_v_setConductivity_s);
    BIND_METHOD(v_setMagnetization_s, MaterialNode_v_setMagnetization_s);
    BIND_METHOD(v_setDirectionOfMagnetization_s, MaterialNode_v_setDirectionOfMagnetization_s);
}
