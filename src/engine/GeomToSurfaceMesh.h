#ifndef BZMAG_ENGINE_GEOMTOSURFACEMESH_H
#define BZMAG_ENGINE_GEOMTOSURFACEMESH_H

/*
Description : Geometry Node to Mesh data (helper class)
Last Update : 2018.11.07
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "core/primitivetype.h"

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/mark_domain_in_triangulation.h>

#include <vector>
#include <array>

namespace bzmag
{
    namespace engine
    {
        using Vertex   = std::array<float64, 2>;    // 2D 좌표를 가지는 절점
        using Segment  = std::array<int32, 2>;      // 세그먼트는 두 절점로 정의
        using Triangle = std::array<int32, 3>;      // 요소는 세 절점로 정의

        typedef CGAL::Triangulation_vertex_base_2<L> Vb;
        typedef CGAL::Delaunay_mesh_face_base_2<L> Fb;
        typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
        typedef CGAL::Constrained_Delaunay_triangulation_2<L, Tds> CDT;
        typedef CDT::Vertex_handle Vertex_handle;

        class GeomHeadNode;
        class ENGINELIBRARY_API GeomToSurfaceMesh
        {
        public:

            GeomToSurfaceMesh(GeomHeadNode* render_node);
            GeomToSurfaceMesh(Polygon_set_2& geometry);
            virtual ~GeomToSurfaceMesh();

        protected:
            void prepairTriangulation(CDT& cdt, const Polygon_with_holes_2& polygon_with_holes);
            void makePolygonData(CDT& cdt, bool flag_elements);
            void segmentCurve(Arrangement& arr, const X_monotone_curve_2& curve);
            //void rebuildPolyHoles(Arrangement& arr, CDT& cdt);
            //void insertConstraint(CDT& cdt, const X_monotone_curve_2& curve);
            void clear();

        public:
            std::vector<Vertex> nodes_;
            std::vector<Segment> segments_;
            std::vector<Triangle> elements_;

        protected:
            int num_segments_ = 90;
            //double epsilon_ = 1e-9; // 동일점 인정 오차, 현재 사용 안함
        };
    }
}

#endif //BZMAG_ENGINE_GEOMTOSURFACEMESH_H
