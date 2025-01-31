#ifndef BZMAG_ENGINE_GEOMROTATENODE_H
#define BZMAG_ENGINE_GEOMROTATENODE_H

/*
Description : Rotate Nodes
Last Update : 2016.04.20
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
        class ENGINELIBRARY_API GeomRotateNode : public GeomBaseNode
        {
        public:
            GeomRotateNode();
            virtual ~GeomRotateNode();
            DECLARE_CLASS(GeomRotateNode, GeomBaseNode);

        public:
            bool setParameters(const String& angle);
            void setAngle(const String& angle);

            float64 evalAngle() const;
            const String& getAngle() const;

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
            Ref<Expression> angle_;
            String sangle_;

            // 참조 좌표계
            Ref<CSNode> cs_;
        };

#include "GeomRotateNode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMROTATENODE_H