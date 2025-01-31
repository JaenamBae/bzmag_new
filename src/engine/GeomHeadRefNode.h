#ifndef BZMAG_ENGINE_HEADREFNODE_H
#define BZMAG_ENGINE_HEADREFNODE_H

/*
Description : Head reference node for Boundary condition
Last Update : 2024.12.04
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "core/node.h"

namespace bzmag
{
    namespace engine
    {
        class GeomHeadNode;
        class ENGINELIBRARY_API GeomHeadRefNode : public Node
        {
        public:
            GeomHeadRefNode();
            virtual ~GeomHeadRefNode();
            DECLARE_CLASS(GeomHeadRefNode, Node);

            GeomHeadNode* getHeadNode() const;
            void setHeadNode(GeomHeadNode* head);

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;

        public:
            static void bindProperty();

        protected:
            // 헤더노드
            Ref<GeomHeadNode> head_ = nullptr;
        };
    }
}

#endif //BZMAG_ENGINE_HEADREFNODE_H