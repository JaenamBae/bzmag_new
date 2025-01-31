#ifndef BZMAG_CORE_OBJECT_NODEEVENTPUBLISHER_H
#define BZMAG_CORE_OBJECT_NODEEVENTPUBLISHER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::NodeEventPublisher
    @brief
*/

#include <list>
#include "platform.h"
#include "nodeeventsubscriber.h"
#include "singleton.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API NodeEventPublisher : public Singleton<NodeEventPublisher>
    {
    public:
        virtual ~NodeEventPublisher() {};
        void add(NodeEventSubscriber* s);
        void remove(NodeEventSubscriber* s);

        void onAttachTo(Node* parent, Node* child);
        void onDetachFrom(Node* parent, Node* child);

    private:
        typedef std::list<NodeEventSubscriber*> NodeEventSubscribers;

    private:
        NodeEventSubscribers subs_;
    };
}

#endif // BZMAG_CORE_OBJECT_NODEEVENTPUBLISHER_H
