#ifndef BZMAG_ENGINE_GEOMLINENODE_H
#define BZMAG_ENGINE_GEOMLINENODE_H

/*
Description : Line Node
Last Update : 2024.12.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomPrimitiveNode.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API GeomLineNode : public GeomPrimitiveNode
        {
        public:
            GeomLineNode();
            virtual ~GeomLineNode();
            DECLARE_CLASS(GeomLineNode, GeomPrimitiveNode);

        public:
            bool setParameters(const String& start, const String& end);

            void setStartPoint(const String& start);
            void setEndPoint(const String& end);

            const String& getStartPoint() const;
            const String& getEndPoint() const;

        public:
            // 이하 재정의 되어야 함
            virtual String description() const override;

        protected:
            virtual void clearBelongings() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            Ref<Expression> sx_;
            Ref<Expression> sy_;
            Ref<Expression> ex_;
            Ref<Expression> ey_;

            // 시작점
            String sstart_;

            // 끝점
            String send_;
        };

#include "GeomLineNode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMLINENODE_H