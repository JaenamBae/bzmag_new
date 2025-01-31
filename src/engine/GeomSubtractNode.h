#ifndef BZMAG_ENGINE_GEOMSUBTRACTNODE_H
#define BZMAG_ENGINE_GEOMSUBTRACTNODE_H

/*
Description : Subtract Node for Boolean Operation
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBooleanNode.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API GeomSubtractNode : public GeomBooleanNode
        {
        public:
            GeomSubtractNode();
            virtual ~GeomSubtractNode();
            DECLARE_CLASS(GeomSubtractNode, GeomBooleanNode);

        public:
            virtual String description() const override;

        protected:
            virtual void boolean_operation(Polygon_set_2& polyset) override;

        public:
            static void bindMethod();
            static void bindProperty();
        };

#include "geomsubtractnode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMSUBTRACTNODE_H