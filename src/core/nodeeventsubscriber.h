#ifndef BZMAG_CORE_OBJECT_NODEEVENTSUBSCRIBER_H
#define BZMAG_CORE_OBJECT_NODEEVENTSUBSCRIBER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::NodeEventSubscriber
    @brief
*/

#include "platform.h"

namespace bzmag
{
    class Node;
    class BZMAG_LIBRARY_API NodeEventSubscriber
    {
    public:
        virtual void onAttachTo(Node* parent, Node* child) {}
        virtual void onDetachFrom(Node* parent, Node* child) {}
    };
}

#endif // BZMAG_CORE_OBJECT_NODEEVENTSUBSCRIBER_H
