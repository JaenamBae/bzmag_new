#ifndef BZMAG_ENGINE_MOVINGBANDNODE_H
#define BZMAG_ENGINE_MOVINGBANDNODE_H

/*
Description : Movingband Node
Last Update : 2024.12.26
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "BCNode.h"
#include "MasterPeriodicBCNode.h"
#include "SlavePeriodicBCNode.h"

namespace bzmag
{
    namespace engine
    {
        class GeomHeadNode;
        class Expression;
        class ENGINELIBRARY_API MovingBandNode : public BCNode
        {
        public:
            MovingBandNode();
            virtual ~MovingBandNode();
            DECLARE_CLASS(MovingBandNode, BCNode);

        public:
            bool checkValid();
            const Polygon_set_2& getInner() const;
            const Polygon_set_2& getOuter() const;
            const Polygon_set_2& getMovingArea() const;

            int32 getInnerAirgapID() const;
            int32 getOuterAirgapID() const;
            BCNode* getOuterBCNode();
            BCNode* getInnerBCNode();

            bool isCircular() const;
            bool isEven() const;

            float64 getCircularCoefficient() const;
            Vector2 getLinearCoefficient() const;

            float64 getAirgapLength() const;
            const String& getSpeed() const;
            void setSpeed(const String& speed);
            float64 evalSpeed();

            const String& getInitialPosition() const;
            void setInitialPosition(const String& speed);
            float64 evalInitialPosition();

            GeomHeadNode* getReferedHead();

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;

        public:
            static void bindProperty();

        protected:
            bool is_circular_ = true;
            MasterPeriodicBCNode* master_;
            SlavePeriodicBCNode* slave_;
            BCNode* outer_line_;
            BCNode* inner_line_;
            Object* outer_;
            Object* inner_;

            Polygon_set_2 inner_obj_;
            Polygon_set_2 outer_obj_;
            Polygon_set_2 moving_area_;

            float64 airgap_length_ = 0;

            // 속도
            Ref<Expression> speed_;

            // 초기위치
            Ref<Expression> initial_pos_;

        };
    }
}

#endif //BZMAG_ENGINE_MOVINGBANDNODE_H