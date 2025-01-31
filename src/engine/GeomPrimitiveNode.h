#ifndef BZMAG_ENGINE_GEOMPRIMITIVENODE_H
#define BZMAG_ENGINE_GEOMPRIMITIVENODE_H

/*
Description : Abstract Node for Primitive Operation
Primitive 노드는 최초의 형상을 만들어 내는 노드들의 집합이다.
ex) 원, 사각형, (곡)선, 복제
Last Update : 2020.08.13
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"

namespace bzmag
{
    namespace engine
    {
        class CSNode;
        class ENGINELIBRARY_API GeomPrimitiveNode : public GeomBaseNode
        {
        public:
            GeomPrimitiveNode();
            virtual ~GeomPrimitiveNode();
            DECLARE_ABSTRACTCLASS(GeomPrimitiveNode, GeomBaseNode);

        public:
            // 참조하는 CS
            void setReferedCS(CSNode* cs);
            CSNode* getReferedCS() const;

            // 이하 재정의 되어야 함
            virtual Transformation getMyTransform() override;

        public:
            //static void bindMethod();
            static void bindProperty();

        protected:
            virtual void clearBelongings() override;
            virtual void updateTransform() override;
            virtual void updateLinkedNode() override;

        protected:
            // 참조 좌표계
            Ref<CSNode> cs_;
        };
    }
}

#endif //BZMAG_ENGINE_GEOMPRIMITIVENODE_H