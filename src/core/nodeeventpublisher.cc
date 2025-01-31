#include "nodeeventpublisher.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
void NodeEventPublisher::add(NodeEventSubscriber* s)
{
    addRefSingleton();
    subs_.push_back(s);
}


//-----------------------------------------------------------------------------
void NodeEventPublisher::remove(NodeEventSubscriber* s)
{
    subs_.remove(s);
    releaseSingleton();
}


//-----------------------------------------------------------------------------
void NodeEventPublisher::onAttachTo(Node* parent, Node* child)
{
    for (NodeEventSubscribers::iterator i = subs_.begin();
         i != subs_.end(); ++i)
         (*i)->onAttachTo(parent, child);
}


//-----------------------------------------------------------------------------
void NodeEventPublisher::onDetachFrom(Node* parent, Node* child)
{
    for (NodeEventSubscribers::iterator i = subs_.begin();
         i != subs_.end(); ++i)
         (*i)->onDetachFrom(parent, child);
}
