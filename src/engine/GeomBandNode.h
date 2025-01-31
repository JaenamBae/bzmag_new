#ifndef BZMAG_ENGINE_GEOMBANDNODE_H
#define BZMAG_ENGINE_GEOMBANDNODE_H

/*
Description : Band Node
Last Update : 2020.11.11
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomPrimitiveNode.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API GeomBandNode : public GeomPrimitiveNode
        {
        public:
            GeomBandNode();
            virtual ~GeomBandNode();
            DECLARE_CLASS(GeomBandNode, GeomPrimitiveNode);

        public:
            bool setParameters(const String& center, const String& radius, const String& width, const String& segs);

            void setCenter(const String& c);
            void setRadius(const String& radius);
            void setWidth(const String& width);
            void setSegments(const String& segs);

            const String& getCenter() const;
            const String& getRadius() const;
            const String& getWidth() const;
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
            Ref<Expression> width_;
            Ref<Expression> segs_;

            String scenter_;
            String sradii_;
            String swidth_;
            String ssegs_;
        };

#include "GeomBandNode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMBANDNODE_H