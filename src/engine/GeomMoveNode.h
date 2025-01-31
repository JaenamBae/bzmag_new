#ifndef BZMAG_ENGINE_GEOMMOVENODE_H
#define BZMAG_ENGINE_GEOMMOVENODE_H

/*
Description : Move Nodes
Comment : Move 노드는 Move를 위한 기준좌표계가 주어져야함
          기준좌표계는 GeomBaseNode에 구현된 setReferedCS()함수를 통해 설정할 수 있음
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class CSNode;
        class ENGINELIBRARY_API GeomMoveNode : public GeomBaseNode
        {
        public:
            GeomMoveNode();
            virtual ~GeomMoveNode();
            DECLARE_CLASS(GeomMoveNode, GeomBaseNode);

        public:
            bool setParameters(const String& dx, const String& dy);
            void set_dx(const String& dx);
            void set_dy(const String& dx);

            float64 eval_dx() const;
            float64 eval_dy() const;
            const String& get_dx() const;
            const String& get_dy() const;

            // 참조하는 CS
            void setReferedCS(CSNode* cs);
            CSNode* getReferedCS() const;

        public:
            // 이하 재정의 되어야 함
            virtual Transformation getMyTransform() override;
            virtual String description() const override;

        protected:
            virtual void clearBelongings() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            Ref<Expression> dx_;
            Ref<Expression> dy_;

            String sdx_;
            String sdy_;

            // 참조 좌표계
            Ref<CSNode> cs_;
        };

#include "GeomMoveNode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMMOVENODE_H