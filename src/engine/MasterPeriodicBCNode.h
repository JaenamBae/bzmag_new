#ifndef BZMAG_ENGINE_MASTERPERIODICBCNODE_H
#define BZMAG_ENGINE_MASTERPERIODICBCNODE_H

/*
Description : Master Periodic Boundary condition Node
Last Update : 2024.12.05
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "BCNode.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API MasterPeriodicBCNode : public BCNode
        {
        public:
            MasterPeriodicBCNode();
            virtual ~MasterPeriodicBCNode();
            DECLARE_CLASS(MasterPeriodicBCNode, BCNode);

        public:
            void setDirection(bool direction);
            bool getDirection() const;

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        public:
            static void bindProperty();

        protected:
            bool dirction_ = true;
        };
    }
}

#endif //BZMAG_ENGINE_MASTERPERIODICBCNODE_H