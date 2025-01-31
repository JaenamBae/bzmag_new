#ifndef BZMAG_ENGINE_GEOMCOVERLINENODE_H
#define BZMAG_ENGINE_GEOMCOVERLINENODE_H

/*
Description : CoverLine Node
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API GeomCoverLineNode : public GeomBaseNode
        {
        public:
            GeomCoverLineNode();
            virtual ~GeomCoverLineNode();
            DECLARE_CLASS(GeomCoverLineNode, GeomBaseNode);

        public:
            // 이하 재정의 되어야 함
            virtual String description() const override;

        protected:
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;
            virtual void updateCovered() override;

        public:
            static void bindMethod();
            static void bindProperty();
        };

#include "geomcoverlinenode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMCOVERLINENODE_H