#ifndef BZMAG_ENGINE_WINDINGNODE_H
#define BZMAG_ENGINE_WINDINGNODE_H

/*
Description : Winding Node for Handling a Windings
Last Update : 2020.11.04
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "CoilNode.h"
#include "Expression.h"
#include "core/node.h"
#include "core/vector2.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API WindingNode : public Node
        {
        public:
            friend class CoilNode;

        public:
            WindingNode();
            virtual ~WindingNode();
            DECLARE_CLASS(WindingNode, Node);

        public:
            void setCurrent(const String& I);
            const String& getCurrent() const;

            void setNumberOfParallelBranches(const String& a);
            const String& getNumberOfParallelBranches() const;
            //void clear();

            float64 evaluateCurrent() const;
            float64 evaluateParallelBranches() const;


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
            Ref<Expression> a_;     // 병렬회로수
            Ref<Expression> I_;     // 전류

            String sa_;
            String sI_;
        };

#include "windingnode.inl"

    }
}

#endif //BZMAG_ENGINE_WINDINGNODE_H