#pragma once

#include "core/node.h"
#include "core/dataset.h"
#include "DrawingObject.h"
#include <map>

class PostPlotNode :public bzmag::Node 
{
public:
    PostPlotNode();
    virtual ~PostPlotNode();
    DECLARE_CLASS(PostPlotNode, bzmag::Node);

    void setPlottingData(const std::string& name, bzmag::DataSet& dataset);
    const std::map<std::string, bzmag::DataSet>& getPlottingData() const;

public:
    static void bindProperty();

private:
    std::map<std::string, bzmag::DataSet> plots_;

};