#ifndef BZMAG_ENGINE_GEOMCURVENODE_H
#define BZMAG_ENGINE_GEOMCURVENODE_H

/*
Description : Curve Node
Last Update : 2019.05.11
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomPrimitiveNode.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API GeomCurveNode : public GeomPrimitiveNode
        {
        public:
            GeomCurveNode();
            virtual ~GeomCurveNode();
            DECLARE_CLASS(GeomCurveNode, GeomPrimitiveNode);

        public:
            bool setParameters(const String& start, const String& end, const String& center, const String& radius);

            void setStartPoint(const String& start);
            void setEndPoint(const String& end);
            void setCenterPoint(const String& center);
            void setRadius(const String& radius);

            const String& getStartPoint() const;
            const String& getEndPoint() const;
            const String& getCenterPoint() const;
            const String& getRadius() const;

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
            Ref<Expression> cx_;
            Ref<Expression> cy_;
            Ref<Expression> radius_;

            // 시작점
            String sstart_;

            // 끝점
            String send_;

            // 중앙점
            String scenter_;

            // 반지름
            String sradius_;

            //private:
            //    static float64 torr_;
        };

#include "geomcurvenode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMCURVENODE_H