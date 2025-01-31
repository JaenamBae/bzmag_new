#ifndef BZMAG_ENGINE_GEOMINTERSECTIONNODE_H
#define BZMAG_ENGINE_GEOMINTERSECTIONNODE_H

/*
Description : Intersection Node for Boolean Operation
Last Update : 2020.08.21
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBooleanNode.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API GeomIntersectionNode : public GeomBooleanNode
        {
        public:
            GeomIntersectionNode();
            virtual ~GeomIntersectionNode();
            DECLARE_CLASS(GeomIntersectionNode, GeomBooleanNode);

        public:
            virtual String description() const override;

        protected:
            virtual void boolean_operation(Polygon_set_2& polyset) override;

        public:
            static void bindMethod();
            static void bindProperty();
        };

#include "geomintersectionnode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMINTERSECTIONNODE_H