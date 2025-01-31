#include "Modeler.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void Modeler_n_createLine_sss(Modeler* self, Parameter* param)
{
    GeomLineNode* res = self->createLine(param->in()->get<String>(0), 
        param->in()->get<String>(1), 
        param->in()->get<String>(2));
    param->out()->get<GeomLineNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createCurve_sssss(Modeler* self, Parameter* param)
{
    GeomCurveNode* res = self->createCurve(param->in()->get<String>(0), 
        param->in()->get<String>(1), 
        param->in()->get<String>(2),
        param->in()->get<String>(3),
        param->in()->get<String>(4));
    param->out()->get<GeomCurveNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createCircle_ssss(Modeler* self, Parameter* param)
{
    GeomCoverLineNode* res = self->createCircle(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2),
        param->in()->get<String>(3));
    param->out()->get<GeomCoverLineNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createRectangle_ssss(Modeler* self, Parameter* param)
{
    GeomCoverLineNode* res = self->createRectangle(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2),
        param->in()->get<String>(3));
    param->out()->get<GeomCoverLineNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createBand_sssss(Modeler* self, Parameter* param)
{
    GeomCoverLineNode* res = self->createBand(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2),
        param->in()->get<String>(3),
        param->in()->get<String>(4));
    param->out()->get<GeomCoverLineNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_clone_ns(Modeler* self, Parameter* param)
{
    GeomCloneFromNode* res = self->clone(param->in()->get<GeomBaseNode*>(0),
        param->in()->get<String>(1));
    param->out()->get<GeomCloneFromNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_move_nss(Modeler* self, Parameter* param)
{
    GeomMoveNode* res = self->move(param->in()->get<GeomBaseNode*>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2));
    param->out()->get<GeomMoveNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_rotate_ns(Modeler* self, Parameter* param)
{
    GeomRotateNode* res = self->rotate(param->in()->get<GeomBaseNode*>(0),
        param->in()->get<String>(1));
    param->out()->get<GeomRotateNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_subtract_nn(Modeler* self, Parameter* param)
{
    auto lhs = param->in()->get<GeomBaseNode*>(0).data_;
    auto rhs = param->in()->get<GeomBaseNode*>(1).data_;

    GeomSubtractNode* res = self->booleanSubtract(lhs, { rhs });
    param->out()->get<GeomSubtractNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_unite_nn(Modeler* self, Parameter* param)
{
    auto lhs = param->in()->get<GeomBaseNode*>(0).data_;
    auto rhs = param->in()->get<GeomBaseNode*>(1).data_;

    GeomUniteNode* res = self->booleanUnite(lhs, { rhs });
    param->out()->get<GeomUniteNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_intersect_nn(Modeler* self, Parameter* param)
{
    auto lhs = param->in()->get<GeomBaseNode*>(0).data_;
    auto rhs = param->in()->get<GeomBaseNode*>(1).data_;

    GeomIntersectionNode* res = self->booleanIntersect(lhs, { rhs });
    param->out()->get<GeomIntersectionNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_split_nibn(Modeler* self, Parameter* param)
{
    GeomSplitNode* res = self->split(param->in()->get<GeomBaseNode*>(0),
        param->in()->get<GeomSplitNode::SPLIT_PLANE>(1),
        param->in()->get<bool>(2),
        param->in()->get<CSNode*>(3));
    param->out()->get<GeomSplitNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createCS_sssn(Modeler* self, Parameter* param)
{
    CSNode* res = self->createCS(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2),
        param->in()->get<CSNode*>(3));
    param->out()->get<CSNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createBCObject_nsn(Modeler* self, Parameter* param)
{
    GeomHeadRefNode* res = self->createBCObject(param->in()->get<GeomHeadNode*>(0),
        param->in()->get<String>(1),
        param->in()->get<BCNode*>(2));

    param->out()->get<GeomHeadRefNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createDirichletBC_ss(Modeler* self, Parameter* param)
{
    DirichletBCNode* res = self->createDirichletBC(param->in()->get<String>(0),
        param->in()->get<String>(1));

    param->out()->get<DirichletBCNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createMasterBC_bs(Modeler* self, Parameter* param)
{
    MasterPeriodicBCNode* res = self->createMasterBC(param->in()->get<bool>(0),
        param->in()->get<String>(1));

    param->out()->get<MasterPeriodicBCNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createSlaveBC_nbbs(Modeler* self, Parameter* param)
{
    MasterPeriodicBCNode* master = param->in()->get<MasterPeriodicBCNode*>(0);

    if (master) {
        SlavePeriodicBCNode* res = self->createSlaveBC(master,
            param->in()->get<bool>(1),
            param->in()->get<bool>(2),
            param->in()->get<String>(3));

        param->out()->get<SlavePeriodicBCNode*>(0) = res;
    }
    else {
        param->out()->get<SlavePeriodicBCNode*>(0) = nullptr;
    }
}

//----------------------------------------------------------------------------
static void Modeler_n_createMovingBand_sss(Modeler* self, Parameter* param)
{
    MovingBandNode* res = self->createMovingBand(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2));

    param->out()->get<MovingBandNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createMaterial_sb(Modeler* self, Parameter* param)
{
    MaterialNode* res = self->createMaterial(param->in()->get<String>(0),
        param->in()->get<bool>(1));
    param->out()->get<MaterialNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createWinding_sss(Modeler* self, Parameter* param)
{
    WindingNode* res = self->createWinding(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2));
    param->out()->get<WindingNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createCoil_sbsn(Modeler* self, Parameter* param)
{
    CoilNode* res = self->createCoil(param->in()->get<String>(0),
        param->in()->get<bool>(1),
        param->in()->get<String>(2),
        param->in()->get<WindingNode*>(3));
    param->out()->get<CoilNode*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createExpression_sss(Modeler* self, Parameter* param)
{
    Expression* res = self->createExpression(param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2));
    param->out()->get<Expression*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_createTransientSetup_sn(Modeler* self, Parameter* param)
{
    Transient* res = self->createTransientSetup(param->in()->get<String>(0),
        param->in()->get<SolutionSetup*>(1));
    param->out()->get<Transient*>(0) = res;
}

//----------------------------------------------------------------------------
static void Modeler_n_getCurrentCS_v(Modeler* self, Parameter* param)
{
    CSNode* cs = self->getCurrentCSNode();
    param->out()->get<CSNode*>(0) = cs;
}

//----------------------------------------------------------------------------
static void Modeler_n_getDefaultCS_v(Modeler* self, Parameter* param)
{
    CSNode* cs = self->getWorkingDefaultCSNode();
    param->out()->get<CSNode*>(0) = cs;
}

//----------------------------------------------------------------------------
static void Modeler_n_getDefaultSetup_v(Modeler* self, Parameter* param)
{
    SolutionSetup* setup = self->getDefaultSetupNode();
    param->out()->get<SolutionSetup*>(0) = setup;
}

//----------------------------------------------------------------------------
void Modeler::bindMethod()
{
    BIND_METHOD(n_createLine_sss, Modeler_n_createLine_sss);
    BIND_METHOD(n_createCurve_sssss, Modeler_n_createCurve_sssss);

    BIND_METHOD(n_createCircle_ssss, Modeler_n_createCircle_ssss);
    BIND_METHOD(n_createRectangle_ssss, Modeler_n_createRectangle_ssss);
    BIND_METHOD(n_createBand_sssss, Modeler_n_createBand_sssss);
    BIND_METHOD(n_clone_ns, Modeler_n_clone_ns);

    BIND_METHOD(n_move_nss, Modeler_n_move_nss);
    BIND_METHOD(n_rotate_ns, Modeler_n_rotate_ns);

    BIND_METHOD(n_subtract_nn, Modeler_n_subtract_nn);
    BIND_METHOD(n_unite_nn, Modeler_n_unite_nn);
    BIND_METHOD(n_intersect_nn, Modeler_n_intersect_nn);
    BIND_METHOD(n_split_nibn, Modeler_n_split_nibn);

    BIND_METHOD(n_createCS_sssn, Modeler_n_createCS_sssn);

    BIND_METHOD(n_createBCObject_nsn, Modeler_n_createBCObject_nsn);
    BIND_METHOD(n_createDirichletBC_ss, Modeler_n_createDirichletBC_ss);
    BIND_METHOD(n_createMasterBC_bs, Modeler_n_createMasterBC_bs);
    BIND_METHOD(n_createSlaveBC_nbbs, Modeler_n_createSlaveBC_nbbs);
    BIND_METHOD(n_createMovingBand_sss, Modeler_n_createMovingBand_sss);

    BIND_METHOD(n_createMaterial_sb, Modeler_n_createMaterial_sb);
    BIND_METHOD(n_createWinding_sss, Modeler_n_createWinding_sss);
    BIND_METHOD(n_createCoil_sbsn, Modeler_n_createCoil_sbsn);
    BIND_METHOD(n_createTransientSetup_sn, Modeler_n_createTransientSetup_sn);

    BIND_METHOD(n_createExpression_sss, Modeler_n_createExpression_sss);

    BIND_METHOD(n_getCurrentCS_v, Modeler_n_getCurrentCS_v);
    BIND_METHOD(n_getDefaultCS_v, Modeler_n_getDefaultCS_v);

    BIND_METHOD(n_getDefaultSetup_v, Modeler_n_getDefaultSetup_v);
}
