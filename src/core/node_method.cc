#include "node.h"

#include "methodbinder.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
static void Node_v_setName_s(Node* self, Parameter* param)
{
    self->setName(param->in()->get<String>(0));
}


//-----------------------------------------------------------------------------
static void Node_s_getName_v(Node* self, Parameter* param)
{
    param->out()->get<String>(0) = self->getName();
}


//-----------------------------------------------------------------------------
static void Node_s_getAbsolutePath_v(Node* self, Parameter* param)
{
    param->out()->get<String>(0) = self->getAbsolutePath();
}


//-----------------------------------------------------------------------------
static void Node_n_getParent_v(Node* self, Parameter* param)
{
    param->out()->get<Node*>(0) = self->getParent();
}


//-----------------------------------------------------------------------------
static void Node_v_attach_n(Node* self, Parameter* param)
{
    self->attach(param->in()->get<Node*>(0));
}


//-----------------------------------------------------------------------------
static void Node_v_detach_v(Node* self, Parameter* param)
{
    self->detach();
}


//-----------------------------------------------------------------------------
static void Node_v_clearChildren_v(Node* self, Parameter* param)
{
    self->clearChildren<Node>();
}


//-----------------------------------------------------------------------------
static void Node_i_getNumChildren_v(Node* self, Parameter* param)
{
    param->out()->get<uint64>(0) = self->getNumChildren();
}


//-----------------------------------------------------------------------------
static void Node_n_findChild_s(Node* self, Parameter* param)
{
    param->out()->get<Node*>(0) = self->findChild(param->in()->get<String>(0));
}


//-----------------------------------------------------------------------------
static void Node_n_relativeNode_s(Node* self, Parameter* param)
{
    param->out()->get<Node*>(0) =
        self->relativeNode(param->in()->get<Path>(0));
}


//-----------------------------------------------------------------------------
static void Node_z_getChildren_v(Node* self, Parameter* param)
{
    param->out()->clear();
    for (Node::NodeIterator i = self->firstChildNode();
         i != self->lastChildNode(); ++i)
        param->out()->add<Node*>(*i);
}

//-----------------------------------------------------------------------------
static void Node_b_update_v(Node* self, Parameter* param)
{
    param->out()->get<bool>(0) = self->update();
}

//-----------------------------------------------------------------------------
void Node::bindMethod()
{
    BIND_METHOD(v_setName_s, Node_v_setName_s);
    BIND_METHOD(s_getName_v, Node_s_getName_v);
    BIND_METHOD(s_getAbsolutePath_v, Node_s_getAbsolutePath_v);
    BIND_METHOD(n_getParent_v, Node_n_getParent_v);
    BIND_METHOD(v_attach_n, Node_v_attach_n);
    BIND_METHOD(v_detach_v, Node_v_detach_v);
    BIND_METHOD(v_clearChildren_v, Node_v_clearChildren_v);
    BIND_METHOD(i_getNumChildren_v, Node_i_getNumChildren_v);
    BIND_METHOD(n_findChild_s, Node_n_findChild_s);
    BIND_METHOD(n_relativeNode_s, Node_n_relativeNode_s);
    BIND_METHOD(z_getChildren_v, Node_z_getChildren_v);
    BIND_METHOD(b_update_v, Node_b_update_v);
}
