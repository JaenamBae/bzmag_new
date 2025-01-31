#ifndef BZMAG_ENGINE_SLAVEPERIODICBCNODE_H
#define BZMAG_ENGINE_SLAVEPERIODICBCNODE_H

/*
Description : Slave Boundary condition Node
Last Update : 2024.12.05
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "MasterPeriodicBCNode.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API SlavePeriodicBCNode : public MasterPeriodicBCNode
        {
        public:
            SlavePeriodicBCNode();
            virtual ~SlavePeriodicBCNode();
            DECLARE_CLASS(SlavePeriodicBCNode, MasterPeriodicBCNode);

        public:
            MasterPeriodicBCNode* getPair() const;
            void setPair(MasterPeriodicBCNode* pair);

            bool isEven() const;
            void setEven(bool even);
            
            bool checkValid();
            bool isCircular() const;
            float64 getCircularCoefficient() const;
            Vector2 getLinearCoefficient() const;

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        public:
            static void bindProperty();

        protected:
            MasterPeriodicBCNode* pair_;
            bool even_ = true;

            bool is_circular_ = true;
            float64 periodic_angle_ = 0;
            Vector2 periodic_vector_;
        };
    }
}

#endif //BZMAG_ENGINE_SLAVEPERIODICBCNODE_H