#ifndef BZMAG_ENGINE_GEOMRECTNODE_H
#define BZMAG_ENGINE_GEOMRECTNODE_H

/*
Description : Rectangular Node
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomPrimitiveNode.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API GeomRectNode : public GeomPrimitiveNode
        {
        public:
            GeomRectNode();
            virtual ~GeomRectNode();
            DECLARE_CLASS(GeomRectNode, GeomPrimitiveNode);

        public:
            bool setParameters(const String& point,
                const String& dx,
                const String& dy);

            void setPoint(const String& point);
            void setWidth(const String& width);
            void setHeight(const String& height);

            const String& getPoint() const;
            const String& getWidth() const;
            const String& getHeight() const;

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
            Ref<Expression> px_;
            Ref<Expression> py_;
            Ref<Expression> width_;
            Ref<Expression> height_;

            String spoint_;
            String swidth_;
            String sheight_;
        };

#include "GeomRectNode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMRECTNODE_H