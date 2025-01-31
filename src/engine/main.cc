#include "engine.h"

using namespace bzmag;
using namespace bzmag::engine;

//-----------------------------------------------------------------------------
void initialize_Engine(Module* module)
{
    REGISTER_TYPE(module, GeomBaseNode);
    REGISTER_TYPE(module, GeomHeadNode);

    REGISTER_TYPE(module, GeomPrimitiveNode);
    REGISTER_TYPE(module, GeomLineNode);
    REGISTER_TYPE(module, GeomCurveNode);
    REGISTER_TYPE(module, GeomCircleNode);
    REGISTER_TYPE(module, GeomRectNode);
    REGISTER_TYPE(module, GeomCloneFromNode);
    REGISTER_TYPE(module, GeomBandNode);

    REGISTER_TYPE(module, GeomCoverLineNode);
    REGISTER_TYPE(module, GeomCloneToNode);
    REGISTER_TYPE(module, GeomMoveNode);
    REGISTER_TYPE(module, GeomRotateNode);
    REGISTER_TYPE(module, GeomSplitNode);

    REGISTER_TYPE(module, GeomBooleanNode);
    REGISTER_TYPE(module, GeomSubtractNode);
    REGISTER_TYPE(module, GeomUniteNode);
    REGISTER_TYPE(module, GeomIntersectionNode);

    REGISTER_TYPE(module, CSNode);
    REGISTER_TYPE(module, MaterialNode);
    REGISTER_TYPE(module, DataSetNode);
    REGISTER_TYPE(module, CoilNode);
    REGISTER_TYPE(module, WindingNode);
    REGISTER_TYPE(module, BCNode);
    REGISTER_TYPE(module, MasterPeriodicBCNode);
    REGISTER_TYPE(module, SlavePeriodicBCNode);
    REGISTER_TYPE(module, DirichletBCNode);
    //REGISTER_TYPE(module, MovingBandBCNode);
    REGISTER_TYPE(module, MovingBandNode);
    REGISTER_TYPE(module, GeomHeadRefNode);
    REGISTER_TYPE(module, SolutionSetup);
    REGISTER_TYPE(module, Transient);

    REGISTER_TYPE(module, Expression);
    REGISTER_TYPE(module, ExpressionServer);

    REGISTER_TYPE(module, GeomToTriangle);
    //REGISTER_TYPE(module, GeomToGmsh);

    GeomToTriangle::setSingletonPath("/sys/triangle");
    //GeomToGmsh::setSingletonPath("/sys/gmsh");
    ExpressionServer::setSingletonPath("/sys/expressions");

    static CSNodeStringConverter cs_sc;
    ToString::instance()->addConverter(&cs_sc);

    static MaterialNodeStringConverter materail_sc;
    ToString::instance()->addConverter(&materail_sc);

    static GeomHeadNodeStringConverter head_sc;
    ToString::instance()->addConverter(&head_sc);

    static GeomCloneToNodeStringConverter clone_sc;
    ToString::instance()->addConverter(&clone_sc);

    static MasterPeriodicBCNodeStringConverter master_sc;
    ToString::instance()->addConverter(&master_sc);
}

//-----------------------------------------------------------------------------
void finalize_Engine(Module* module)
{

}

//-----------------------------------------------------------------------------
DECLARE_MODULE(ENGINELIBRARY_API, Engine);
