#pragma once

#include "core/node.h"
#include "DrawingObject.h"

class PostNode :public bzmag::Node 
{
public:
    PostNode();
    virtual ~PostNode();
    DECLARE_CLASS(PostNode, bzmag::Node);

public:
    void setDrawingObject(DrawingObject* object);
    DrawingObject* getDrawingObject();

    void setDrawingFlag(bool flag);
    bool getDrawingFlag() const;

public:
    static void bindProperty();

private:
    DrawingObject* obj_ = nullptr;
    bool draw_flag_ = true;
};