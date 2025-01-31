#ifndef BZMAG_ENGINE_GEOMBOOLEANNODE_H
#define BZMAG_ENGINE_GEOMBOOLEANNODE_H

/*
Description : Abstract Node for Boolean Operation
Last Update : 2017.09.29
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"
#include <list>

namespace bzmag
{
    namespace engine
    {
        class GeomHeadNode;
        class ENGINELIBRARY_API GeomBooleanNode : public GeomBaseNode
        {
        public:
            typedef std::list<GeomHeadNode*> ToolNodes;
            typedef ToolNodes::iterator ToolIter;

        public:
            GeomBooleanNode();
            virtual ~GeomBooleanNode();
            DECLARE_ABSTRACTCLASS(GeomBooleanNode, GeomBaseNode);

            ToolIter firstToolNode()
            {
                return toolnodes_.begin();
            }

            ToolIter lastToolNode()
            {
                return toolnodes_.end();
            }

            void updateToolNodes();

        public:
            // 이하 재정의 되어야 함
            virtual String description() const = 0;

        protected:
            //virtual void updateTransform();
            virtual void updateLinkedNode() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;

            // 이하 재정의 필요
            virtual void boolean_operation(Polygon_set_2& polyset) = 0;

        protected:
            ToolNodes toolnodes_;
        };
    }
}

#endif //BZMAG_ENGINE_GEOMBOOLEANNODE_H