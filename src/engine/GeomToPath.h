#ifndef BZMAG_ENGINE_GEOMTOPATH_H
#define BZMAG_ENGINE_GEOMTOPATH_H

/*
Description : Geometry Node to vertex list Converter (helper class)
Last Update : 2018.11.07
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "core/primitivetype.h"
#include "core/vector2.h"

namespace bzmag
{
    namespace engine
    {
        class GeomHeadNode;
        class BCNode;
        class ENGINELIBRARY_API GeomToPath
        {
        public:
            struct VertexInfo { float64 x; float64 y; uint32 cmd; };
            typedef std::list<VertexInfo> VertexList;

            GeomToPath(GeomHeadNode* render_node);
            GeomToPath(BCNode* bc_node);
            virtual ~GeomToPath();

        public:
            void setNormalDeviation(uint32 num);
            uint32 getNormalDeviation() const;
            bool makePath(VertexList& path);
            bool makeEdgePath(uint32 edgeID, VertexList& path);

        protected:
            void makeBoundary(Polygon_2& p, VertexList& path);
            void makeBoundary(const X_monotone_curve_2& c, VertexList& path, bool first = true);

        protected:
            GeomHeadNode* render_node_;
            BCNode* bc_node_;
            //VertexList path_;

            // 360도 아크를 표현할 세그먼트 갯수
            uint32 num_segments_;
        };
    }
}

#endif //BZMAG_ENGINE_GEOMTOPATH_H
