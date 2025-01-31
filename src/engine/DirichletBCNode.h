#ifndef BZMAG_ENGINE_DIRICHLETBCNODE_H
#define BZMAG_ENGINE_DIRICHLETBCNODE_H

/*
Description : Dirichlet Boundary condition Node
Last Update : 2024.12.05
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "BCNode.h"
#include "core/ref.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API DirichletBCNode : public BCNode
        {
        public:
            DirichletBCNode();
            virtual ~DirichletBCNode();
            DECLARE_CLASS(DirichletBCNode, BCNode);

        public:
            void setBCValue(const String& value);
            const String& getBCValue() const;
            float64 evaluateBCValue() const;

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;

        public:
            static void bindProperty();

        protected:
            Ref<Expression> value_;
        };
    }
}

#endif //BZMAG_ENGINE_DIRICHLETBCNODE_H