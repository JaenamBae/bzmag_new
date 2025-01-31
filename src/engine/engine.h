#ifndef BZMAG_ENGINE_H
#define BZMAG_ENGINE_H

#include "GeomHeadNode.h"

#include "GeomPrimitiveNode.h"
#include "GeomLineNode.h"
#include "GeomCurveNode.h"
#include "GeomCircleNode.h"
#include "GeomRectNode.h"
#include "GeomBandNode.h"
#include "GeomClonefromNode.h"

#include "GeomClonetoNode.h"
#include "GeomCoverlineNode.h"
#include "GeomMoveNode.h"
#include "GeomRotateNode.h"
#include "GeomSplitNode.h"

#include "GeomSubtractNode.h"
#include "GeomUniteNode.h"
#include "GeomIntersectionNode.h"

#include "CSNode.h"
#include "BCNode.h"
#include "MasterPeriodicBCNode.h"
#include "SlavePeriodicBCNode.h"
#include "DirichletBCNode.h"
//#include "MovingBandBCNode.h"
#include "MovingBandNode.h"
#include "GeomHeadRefNode.h"
#include "MaterialNode.h"
#include "DataSetNode.h"
#include "CoilNode.h"
#include "WindingNode.h"
#include "SolutionSetup.h"
#include "Transient.h"
#include "ExpressionServer.h"

#include "GeomToTriangle.h"
//#include "GeomToGmsh.h"

#include "EngineNodeStringConverter.h"

#include "core/define.h"
#include "core/module.h"
#include "core/kernel.h"
#include "core/tostring.h"

#endif //BZMAG_ENGINE_H