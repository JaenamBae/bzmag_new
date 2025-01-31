#ifndef BZMAG_ENGINE_GEOMTOTRIANGLE_H
#define BZMAG_ENGINE_GEOMTOTRIANGLE_H

#include "platform.h"
#include "core/node.h"
#include "core/singleton3.h"
#include "core/tuple2.h"
#include "GeometricEntity.h"
#include "GMshDataStructure.h"
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <spdlog/spdlog.h>  // spdlog 추가
#include <utility>
#include <thread>
#include <atomic>

namespace bzmag
{
    namespace engine
    {
        class BCNode;
        class MovingBandNode;
        class GeomHeadNode;
        class ENGINELIBRARY_API GeomToTriangle : public Node, public Singleton3<GeomToTriangle>
        {
        public:
            // ----------------------------------------------
            // 연산에 필요한 CGAL 요소
            using Point = Traits_2::Point_2;
            using Vb = CGAL::Triangulation_vertex_base_2<L>;
            using Fb = CGAL::Delaunay_mesh_face_base_2<L>;
            using Tds = CGAL::Triangulation_data_structure_2<Vb, Fb>;
            using CDT = CGAL::Constrained_Delaunay_triangulation_2<L, Tds>;
            using Vertex_handle = CDT::Vertex_handle;
            using Bbox_2 = CGAL::Bbox_2;

            // ----------------------------------------------
            // Triangle 입력 데이터 형식을 위한 데이터 구조
            using Vert     = std::array<float64, 2>;
            using Seg      = std::array<int32, 2>;
            struct Region {
                Vert point = {0,0};
                int32 attribute = -1;    // should be region_id
                float64 max_area = 0;
            };
            using Vertices = std::vector<Vert>;
            using Segments = std::vector<Seg> ;
            using Regions  = std::map<int32, Region>; // id, {Vert, attribute, maximum area}
            using Holes    = std::vector<Vert>;

            // ----------------------------------------------
            // 도메인 정보; GeomHeadNode 들에 대한 정보
            struct DomainInfo {
                String name;
                float64 area;
                int32   required_number_of_elements;
            };

            // ----------------------------------------------
            // 단일 Polyholes
            struct UniquePolyHoles {
                int32 region_id = -1;       // region id (GeomHeadNode의 id)
                Polygon_with_holes_2 polyholes;             // 원본 폴리홀
                Polygon_set_2        domain;                // 원본에서 내부에 포함된 폴리홀 모두 뺀 실질적인 도메인
                Polygon_with_holes_2 unique_polyholes;      // 완전독립 폴리홀
                Polygon_with_holes_2 refined_polyholes;     // 절점 추가된 폴리홀
                Polygon_with_holes_2 segmented_polyholes;   // 최종 요소 고려한 세그멘트 된 폴리홀
                std::vector<int32>   included_ids;          // 이 polyhole 에 포함된 UniquePolyHole 의 unique_id

                int32 required_number_of_elements = 0;
                float64 area = 0;
                float64 based_length = 0;
                float64 based_angle = 0;
            };

            // ----------------------------------------------
            // 도메인을 구성하는 커브(refined_polyholes 기반)
            struct DomainCurve {
                X_monotone_curve_2 curve;
                std::vector<int32> unique_polyholes_ids;  // UniquePolyHoles 의 id
                float64 based_length = 0;
                float64 based_angle = 0;
                std::list<Vert> segmented_curve;
            };

        public:
            GeomToTriangle();
            virtual ~GeomToTriangle();
            DECLARE_CLASS(GeomToTriangle, Node);

        public:
            int getProgress() const { return progress_; }
            void stop() { stop_flag_ = true; }
            bool isComplete() const { return done_configure_; }

            bool setPath(const String& geom_path, const String& bc_path);
            bool generateGmshStructures(const String& geom_path, const String& bc_path, std::function<void(int, int)> progressCallback = 0);
            Vertices::iterator firstVertex() { return vertices_.begin(); }
            Vertices::iterator lastVertex()  { return vertices_.end(); }

            std::vector<int32>::iterator firstVertexMarker() { return vertices_maker_.begin(); }
            std::vector<int32>::iterator lastVertexMarker() { return vertices_maker_.end(); }

            Segments::iterator firstSegment(){ return segments_.begin(); }
            Segments::iterator lastSegment() { return segments_.end(); }

            std::vector<int32>::iterator firstSegmentMarker() { return segment_makers_.begin(); }
            std::vector<int32>::iterator lastSegmentMarker()  { return segment_makers_.end(); }

            Regions::iterator  firstRegion() { return regions_.begin(); }
            Regions::iterator  lastRegion()  { return regions_.end(); }

            Holes::iterator    firstHole()   { return holes_.begin(); }
            Holes::iterator    lastHole()    { return holes_.end(); }

            std::map<int32, UniquePolyHoles>::iterator firstUinquePolyHole() { return polyholes_.begin(); }
            std::map<int32, UniquePolyHoles>::iterator lastUinquePolyHole() { return polyholes_.end(); }

            int32 getTotalRequiredNumberOfElements() const { return based_total_elements_num_; }
            void setTotalRequiredNumberOfElements(int32 elements_num) { based_total_elements_num_ = elements_num; }

            float64 getBasedAngleForSegmentation() const { return based_angle_; }
            void setBasedAngleForSegmentation(float64 angle) {  based_angle_ = angle; }
            float64 getDomainArea(int32 region_id) { return domain_info_[region_id].area; }

            void writePolyFile(const String& filename) const;
            bool writeGeoFile(const String& filename);

        public:
            static void bindMethod();
            static void bindProperty();

        protected:
            int calculateProgress(int total_step, int current_step);

            // for Step 1
            bool registerPolygonWithHoles(int32 region_id, const Polygon_with_holes_2& polyholes);

            // Step 2
            bool isPointOnCurveWithTolerence(const X_monotone_curve_2& curve, const Traits_2::Point_2& point, Traits_2::Point_2& intersection_point);
            bool isPointWithinBoundingBox(const X_monotone_curve_2& curve, const Traits_2::Point_2 & point);

            // for Step 2
            void extractCurvesFromPolyholes(const Polygon_with_holes_2& polyholes,
                std::list<X_monotone_curve_2>& outer_curves,
                std::list<X_monotone_curve_2>& hole_curves);
            void generateBasedPointsFromPolyholes(const Polygon_with_holes_2& polyholes);

            // for Step 3
            Polygon_with_holes_2 refinePolyholesWithBasedPoints(const Polygon_with_holes_2& polyholes);
            void rebuildPolyHoles(Arrangement& arr, std::vector<Polygon_with_holes_2>& polygons_with_holes);
            bool rebuildPolygons(std::list<X_monotone_curve_2>& curves, std::vector<Polygon_2>& outer_boundary, std::vector<Polygon_2>& holes);

            // for Step 4, 5
            Polygon_with_holes_2 segmentPolyholes(const Polygon_with_holes_2& polyholes, int32 unique_polyholes_id,
                float64 based_length, float64 based_angle);
            Polygon_2 segmentPolygon(const Polygon_2& polygon, int32 unique_polyholes_id,
                float64 based_length, float64 based_angle);
            bool segmentCurve(const X_monotone_curve_2& edge, std::list<Vert>& result, 
                float64 based_length, float64 based_angle);
            void filterPolygonBoundary(std::list<X_monotone_curve_2>& curves);

            std::pair<BCNode*, bool> testCurveInBCNode(const X_monotone_curve_2& edge) const;
            float64 calculatePolyholeArea(const Polygon_with_holes_2& polyholes, int32 unique_polyholes_id);

            // for Step 5
            void generateBasedCurveFromPolyHoles(const Polygon_with_holes_2& polyholes, int32 unique_polyholes_id);
            void generateBasedCurveFromPolygon(const Polygon_2& polygon, int32 unique_polyholes_id);
            bool findBasedCurve(const X_monotone_curve_2& curve, int32& curve_id, bool& direction);

            // For Step 6-1
            void generateTriangleData();
            int32 insertVertex(const Vert& vert, int32 boundary_marker);    // 동일점 인정기준 적용!
            int32 insertSegment(const Seg& seg, int32 boundary_marker);
            bool getArbitraryPointInPolyHoles(const Polygon_with_holes_2& polyholes, Vert& point);
            bool triangulatePolyholes(CDT& cdt, const Polygon_with_holes_2& polygon_with_holes);

            // For Step 6-2
            //void generateGmshData();
            void generateGmshSurface(const Polygon_2& polygon, std::vector<int>& surface_ids, int domain_id);
            int32 insertGPoint(const GPoint& point);    // 동일점 인정기준 적용!
            int32 insertGCurve(const GCurve& curve);
            int32 insertGLoop(const GLoop& loop);
            void removeRedundancyGCurves(std::vector<int>& curve_ids);
            void calculateGmshSizeAtPoints();
            void adjustPointsOnPeriodicBC();
            void clear();

        private:
            String geom_path_;
            String bc_path_;

            //--------------------------------------- 
            // INPUT VARIABLES
            // 주어진 폴리셋트 (ID, Polygon_set_2의 조합)
            static int32 unique_id_;
            std::map<int32, UniquePolyHoles> polyholes_;
            std::map<int32, Polygon_set_2>   domains_;
            std::map<int32, DomainInfo>      domain_info_;

            // 기저절점
            std::list<Point> based_points_;
            std::list<Point> extra_points_;

            // 도메인을 구성하는 모든 커브들 집함
            std::vector<DomainCurve> based_curves_;

            // makePolyStructures() 가 수행되었는지?
            bool done_configure_ = false;

            //---------------------------------------
            // OUTPUT VARIABLES
            // Triangle Input 데이터
            Vertices vertices_;
            std::vector<int32> vertices_maker_;

            Segments segments_;
            std::vector<int32> segment_makers_;

            Regions  regions_;
            Holes    holes_;

            //---------------------------------------
            // OUTPUT VARIABLES
            // gmsh 데이터
            static int32 gmsh_entity_id_;
            std::map<int, GPoint> gmsh_points_;

            // GPoint와 연관된(GPoint를 포함하는) UniquePolyHoles의 id(key)
            std::map<int, std::vector<int>> gmsh_related_domains_;
            std::map<int, GCurve> gmsh_curves_;
            std::map<int, GLoop> gmsh_loops_;

            // key: region_id, value: GSurfaces
            std::map<int32, std::vector<GSurface>> gmsh_surfaces_;

            // key: bcnode, value: curve_ids
            std::map<BCNode*, std::vector<int32>> gmsh_bc_curves_;
            std::map<BCNode*, std::vector<int32>> gmsh_bc_points_;


            // 진행률 관련
            std::atomic<int> progress_;
            std::atomic<bool> stop_flag_;

            //---------------------------------------
            // REFERENCES
            // 참조하는 경계조건 노드
            std::list<BCNode*> bc_nodes_;
            MovingBandNode* mb_;

            static int32 based_total_elements_num_;
            static float64 based_angle_;
            static float64 tol_;
        };
    }
}

#endif //BZMAG_ENGINE_GEOMTOTRIANGLE_H