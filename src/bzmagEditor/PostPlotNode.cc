#include "PostPlotNode.h"
#include "core/simplepropertybinder.h"
#include "engine/ExpressionServer.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(PostPlotNode, Node);

//----------------------------------------------------------------------------
PostPlotNode::PostPlotNode() 
{

}

PostPlotNode::~PostPlotNode()
{
    
}

void PostPlotNode::setPlottingData(const std::string& name, bzmag::DataSet& dataset)
{
    plots_[name] = dataset;
}

const std::map<std::string, bzmag::DataSet>& PostPlotNode::getPlottingData() const
{
    return plots_;
}

void PostPlotNode::bindProperty()
{
    //BIND_PROPERTY(bool, Show, &setDrawingFlag, &getDrawingFlag);
}
