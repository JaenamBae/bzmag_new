#ifndef BZMAG_ENGINE_GEOMUNITENODE_H
#define BZMAG_ENGINE_GEOMUNITENODE_H

/*
Description : Abstract Node for Boolean Operation
Last Update : 2020.08.21
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBooleanNode.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API GeomUniteNode : public GeomBooleanNode
        {
        public:
            GeomUniteNode();
            virtual ~GeomUniteNode();
            DECLARE_CLASS(GeomUniteNode, GeomBooleanNode);

        public:
            virtual String description() const override;

        protected:
            virtual void boolean_operation(Polygon_set_2& polyset) override;

        public:
            static void bindMethod();
            static void bindProperty();
        };

#include "geomunitenode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMUNITENODE_H