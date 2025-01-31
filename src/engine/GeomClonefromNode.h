#ifndef BZMAG_ENGINE_GEOMCLONEFROMNODE_H
#define BZMAG_ENGINE_GEOMCLONEFROMNODE_H

/*
Description : CloneFrom Node
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomPrimitiveNode.h"

namespace bzmag
{
    namespace engine
    {
        class GeomCloneToNode;
        class ENGINELIBRARY_API GeomCloneFromNode : public GeomPrimitiveNode
        {
        public:
            GeomCloneFromNode();
            virtual ~GeomCloneFromNode();
            DECLARE_CLASS(GeomCloneFromNode, GeomPrimitiveNode);

        public:
            void setReferenceNode(GeomCloneToNode* node);
            GeomCloneToNode* getReferenceNode() const;

        public:
            // 이하 재정의 되어야 함
            virtual String description() const override;

        protected:
            virtual void clearBelongings() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;
            virtual void updateCovered();

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            Ref<GeomCloneToNode> from_; // modified : 2020.08.18
        };

#include "geomclonefromnode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMCLONEFROMNODE_H