#ifndef BZMAG_ENGINE_COILNODE_H
#define BZMAG_ENGINE_COILNODE_H

/*
Description : Coil Node for Handling a Windings
Last Update : 2020.11.04
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "Expression.h"
#include "core/node.h"
#include "core/vector2.h"
#include "core/ref.h"

namespace bzmag
{
    namespace engine
    {
        class GeomHeadNode;
        class ENGINELIBRARY_API CoilNode : public Node
        {
        public:
            CoilNode();
            virtual ~CoilNode();
            DECLARE_CLASS(CoilNode, Node);

        public:
            double getCurrent();
            void setDirection(bool dir);
            bool getDirection() const;
            void setNumberOfTurns(const String& turns);
            const String& getNumberOfTurns() const;

            void setReferenceNode(GeomHeadNode* head);
            GeomHeadNode* getReferenceNode() const;
            //void clear();

            float64 evaluateNumberOfTurns() const;

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;

        public:
            static void bindMethod();
            static void bindProperty();

        protected:
            Ref<GeomHeadNode> ref_node_;
            bool direction_;

            Ref<Expression> turns_;
            String sturns_;
        };

#include "coilnode.inl"

    }
}

#endif //BZMAG_ENGINE_COILNODE_H