#ifndef BZMAG_ENGINE_GEOMCIRCLENODE_H
#define BZMAG_ENGINE_GEOMCIRCLENODE_H

/*
Description : Circle Node
Last Update : 2017.09.28
- Big change of Expression
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomPrimitiveNode.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API GeomCircleNode : public GeomPrimitiveNode
        {
        public:
            GeomCircleNode();
            virtual ~GeomCircleNode();
            DECLARE_CLASS(GeomCircleNode, GeomPrimitiveNode);

        public:
            bool setParameters(const String& center, const String& radius, const String& segs = "0");
            void setCenter(const String& c);
            void setRadius(const String& radius);
            void setSegments(const String& segs);

            const String& getCenter() const;
            const String& getRadius() const;
            const String& getSegments() const;

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
            Ref<Expression> cx_;
            Ref<Expression> cy_;
            Ref<Expression> radii_;
            Ref<Expression> segs_;

            String scenter_;
            String sradii_;
            String ssegs_;
        };

#include "geomcirclenode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMCIRCLENODE_H