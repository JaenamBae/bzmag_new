#ifndef BZMAG_ENGINE_GEOMCLONETONODE_H
#define BZMAG_ENGINE_GEOMCLONETONODE_H

/*
Description : CloneTo Node
Last Update : 2016.04.23
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"

namespace bzmag
{
    namespace engine
    {
        class GeomCloneFromNode;
        class ENGINELIBRARY_API GeomCloneToNode : public GeomBaseNode
        {
            friend class GeomCloneFromNode;

        public:
            // how to change the list to set? (duplicate problem)
            typedef std::list<GeomCloneFromNode*> FromNodes;
            typedef FromNodes::iterator FromIterator;

            GeomCloneToNode();
            virtual ~GeomCloneToNode();
            DECLARE_CLASS(GeomCloneToNode, GeomBaseNode);

        public:
            FromIterator firstClonedNode();
            FromIterator lastClonedNode();

        public:
            // 이하 재정의 되어야 함
            virtual String description() const override;
            virtual bool update() override;

        protected:
            virtual void clearBelongings() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;

        public:
            static void bindMethod();
            static void bindProperty();

        protected:
            // GeomCloneFrom 노드에서만 설정할 수 있게 바꾸었음
            FromNodes clones_;
        };

#include "geomclonetonode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMCLONETONODE_H