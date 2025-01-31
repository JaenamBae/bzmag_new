#include "GeomToSurfaceMesh.h"
#include "GeomHeadNode.h"
#include "BCNode.h"
#include <cmath>
#include <spdlog/spdlog.h>

namespace bzmag
{
    namespace engine
    {
        /*
        // 두 점이 허용 오차 내에서 동일한지 확인하는 함수
        bool are_points_close(const Point_2& p1, const Point_2& p2, double epsilon) {
            return (std::abs(CGAL::to_double(p1.x() - p2.x())) < epsilon) && (std::abs(CGAL::to_double(p1.y() - p2.y())) < epsilon);
        }

        // CDT에 이미 삽입된 점인지 확인하는 함수
        Vertex_handle find_existing_vertex(const CDT& cdt, const Point_2& new_point, double epsilon) {
            for (auto vit = cdt.finite_vertices_begin(); vit != cdt.finite_vertices_end(); ++vit) {
                if (are_points_close(vit->point(), new_point, epsilon)) {
                    return vit; // 이미 존재하는 점을 반환
                }
            }
            return Vertex_handle(); // 동일한 점이 없으면 null 반환
        }
        */

        GeomToSurfaceMesh::GeomToSurfaceMesh(GeomHeadNode* render_node)
        {
            CDT cdt;
            if (render_node) {
                // 면의 경우
                if (render_node->isCovered())
                {
                    const Polygon_set_2& geometry = render_node->getPolyset();

                    std::list<Polygon_with_holes_2> res;
                    geometry.polygons_with_holes(std::back_inserter(res));
                    if (res.size() == 0) return;

                    std::list<Polygon_with_holes_2>::iterator it;
                    for (it = res.begin(); it != res.end(); ++it)
                    {
                        Polygon_with_holes_2 polygon_with_holes = *it;
                        //if (polygon_with_holes.is_empty()) return;

                        prepairTriangulation(cdt, polygon_with_holes);
                    }

                    // 내부와 외부를 마킹 (요소생성을 위해)
                    CGAL::mark_domain_in_triangulation(cdt);

                    makePolygonData(cdt, true);
                }

                // 선의 경우
                else {
                    const GeomBaseNode::Curves& curves = render_node->getCurves();
                    Polygon_2 poly(curves.begin(), curves.end());
                    Polygon_with_holes_2 poly_holes(poly);
                    prepairTriangulation(cdt, poly_holes);
                    makePolygonData(cdt, false);
                }
            }
        }

        GeomToSurfaceMesh::GeomToSurfaceMesh(Polygon_set_2& geometry)
        {
            CDT cdt;

            std::list<Polygon_with_holes_2> res;
            geometry.polygons_with_holes(std::back_inserter(res));
            if (res.size() == 0) return;

            std::list<Polygon_with_holes_2>::iterator it;
            for (it = res.begin(); it != res.end(); ++it)
            {
                Polygon_with_holes_2 polygon_with_holes = *it;
                //if (polygon_with_holes.is_empty()) return;

                prepairTriangulation(cdt, polygon_with_holes);
            }

            // 내부와 외부를 마킹 (요소생성을 위해)
            CGAL::mark_domain_in_triangulation(cdt);

            makePolygonData(cdt, true);
        }

        GeomToSurfaceMesh::~GeomToSurfaceMesh()
        {

        }

        void GeomToSurfaceMesh::clear()
        {
            nodes_.clear();
            segments_.clear();
            elements_.clear();
        }

        void GeomToSurfaceMesh::prepairTriangulation(CDT& cdt, const Polygon_with_holes_2& polygon_with_holes)
        {
            //if (polygon_with_holes.is_empty() || polygon_with_holes.is_unbounded()) return;
            if (polygon_with_holes.is_unbounded()) return;

            Arrangement arr;

            // 1. 외부 경계 처리
            const Polygon_2& outer_boundary = polygon_with_holes.outer_boundary();
            for (auto it = outer_boundary.curves_begin(); it != outer_boundary.curves_end(); ++it) {
                const X_monotone_curve_2& curve = *it;
                try {
                    // 직선 세그먼트로 CDT에 삽입
                    segmentCurve(arr, curve);
                    //insertConstraint(cdt, curve);
                }
                catch (const std::exception& e) {
                    // 예외가 발생하면 처리
                    spdlog::error("Error inserting constraint for outer boundary: {}", e.what());
                    clear();
                    return;  // 에러 발생 시 데이터 초기화 후 종료
                }
            }

            // 2. 홀 경계 처리
            for (auto hole_it = polygon_with_holes.holes_begin(); hole_it != polygon_with_holes.holes_end(); ++hole_it) {
                const Polygon_2& hole = *hole_it;
                for (auto it = hole.curves_begin(); it != hole.curves_end(); ++it) {
                    const X_monotone_curve_2& curve = *it;
                    try {
                        // 직선 세그먼트로 CDT에 삽입
                        segmentCurve(arr, curve);
                        //insertConstraint(cdt, curve);
                    }
                    catch (const std::exception& e) {
                        // 예외가 발생하면 처리
                        spdlog::error("Error inserting constraint for hole boundary: {}", e.what());
                        clear();
                        return;  // 에러 발생 시 데이터 초기화 후 종료
                    }
                }
            }

            // Arrangement로 부터 CDT의 Constraint 입력
            for (auto cit = arr.edges_begin(); cit != arr.edges_end(); ++cit) {
                const X_monotone_curve_2& curve = cit->curve();
                Point_2 ss(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
                Point_2 tt(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
                cdt.insert_constraint(ss, tt);
            }
        }

        void GeomToSurfaceMesh::makePolygonData(CDT& cdt, bool flag_elements)
        {
            // 절점 좌표 및 노드 인덱스를 추적하기 위한 맵
            std::map<Vertex_handle, int> vertex_map;
            int vertex_index = 0;

            // 절점(Vertices) 좌표 저장
            for (auto vit = cdt.finite_vertices_begin(); vit != cdt.finite_vertices_end(); ++vit) {
                vertex_map[vit] = vertex_index++;
                double x = CGAL::to_double(vit->point().x());
                double y = CGAL::to_double(vit->point().y());
                nodes_.push_back({ x, y });
            }

            // 세그먼트(Segments) 좌표 저장
            for (auto seg_it = cdt.finite_edges_begin(); seg_it != cdt.finite_edges_end(); ++seg_it) {
                if (cdt.is_constrained(*seg_it)) {  // 제약 엣지만 처리
                    Segment segment;

                    Vertex_handle v1 = seg_it->first->vertex(cdt.cw(seg_it->second));
                    Vertex_handle v2 = seg_it->first->vertex(cdt.ccw(seg_it->second));

                    segment[0] = vertex_map[v1];  // 첫 번째 절점 인덱스
                    segment[1] = vertex_map[v2];  // 두 번째 절점 인덱스
                    segments_.push_back(segment);
                }
            }

            // 삼각형 요소(노드1, 노드2, 노드3) 저장
            if (flag_elements) {
                for (auto face_it = cdt.finite_faces_begin(); face_it != cdt.finite_faces_end(); ++face_it) {
                    if (face_it->is_in_domain()) {
                        Triangle element;
                        element[0] = vertex_map[face_it->vertex(0)];
                        element[1] = vertex_map[face_it->vertex(1)];
                        element[2] = vertex_map[face_it->vertex(2)];
                        elements_.push_back(element);
                    }
                }
            }
        }

        void GeomToSurfaceMesh::segmentCurve(Arrangement& arr, const X_monotone_curve_2& curve)
        {
            Vertex source{ CGAL::to_double(curve.source().x()),
                          CGAL::to_double(curve.source().y()) };
            Vertex target{ CGAL::to_double(curve.target().x()),
                           CGAL::to_double(curve.target().y()) };

            // 커브이면 세그멘테이션 포인트 삽입
            if (curve.is_circular()) {
                Vertex center{ CGAL::to_double(curve.supporting_circle().center().x()),
                               CGAL::to_double(curve.supporting_circle().center().y()) };
                double radius = std::sqrt(CGAL::to_double(curve.supporting_circle().squared_radius()));

                double start_angle = std::atan2(source[1] - center[1], source[0] - center[0]);
                double end_angle = std::atan2(target[1] - center[1], target[0] - center[0]);
                if (start_angle < 0) start_angle += 2 * CGAL_PI;
                if (end_angle < 0)   end_angle += 2 * CGAL_PI;
                if (curve.orientation() == CGAL::COUNTERCLOCKWISE) {
                    if (start_angle > end_angle) start_angle -= 2 * CGAL_PI;
                }
                else {
                    if (start_angle < end_angle) end_angle -= 2 * CGAL_PI;
                }

                int segnum = int(abs(num_segments_ * ((start_angle - end_angle) / (2.0 * CGAL_PI))));
                double delta_angle = (end_angle - start_angle) / segnum;
                Point_2 previous_point(source[0], source[1]);
                for (int sn = 1; sn < segnum; ++sn)
                {
                    double ang = start_angle + sn * delta_angle;
                    Point_2 current_point(
                        center[0] + radius * std::cos(ang),
                        center[1] + radius * std::sin(ang)
                    );

                    // 세그멘테이션 포인트 추가
                    X_monotone_curve_2 seg_curve(previous_point, current_point);
                    CGAL::insert(arr, seg_curve);

                    previous_point = current_point;
                }
                Point_2 end_point(target[0], target[1]);
                X_monotone_curve_2 seg_curve(previous_point, end_point);
                CGAL::insert(arr, seg_curve);

            }
            else {
                Point_2 ss(source[0], source[1]);
                Point_2 tt(target[0], target[1]);
                X_monotone_curve_2 seg_curve(ss, tt);
                CGAL::insert(arr, seg_curve);
            }
        }

        /*
        void GeomToSurfaceMesh::insertConstraint(CDT& cdt, const X_monotone_curve_2& curve)
        {
            if (curve.is_circular()) {
                Vertex center{ CGAL::to_double(curve.supporting_circle().center().x()),
                               CGAL::to_double(curve.supporting_circle().center().y()) };
                double radius = std::sqrt(CGAL::to_double(curve.supporting_circle().squared_radius()));

                Vertex source{ CGAL::to_double(curve.source().x()),
                               CGAL::to_double(curve.source().y()) };
                Vertex target{ CGAL::to_double(curve.target().x()),
                               CGAL::to_double(curve.target().y()) };

                double start_angle = std::atan2(source[1] - center[1], source[0] - center[0]);
                double end_angle = std::atan2(target[1] - center[1], target[0] - center[0]);

                if (start_angle < 0) start_angle += 2 * CGAL_PI;
                if (end_angle < 0)   end_angle += 2 * CGAL_PI;

                
                if (curve.orientation() == CGAL::COUNTERCLOCKWISE) {
                    if (start_angle > end_angle) start_angle -= 2 * CGAL_PI;
                }
                else {
                    if (start_angle < end_angle) end_angle -= 2 * CGAL_PI;
                }

                Point_2 previous_point(source[0], source[1]);
                int segnum = int(abs(num_segments_ * ((start_angle - end_angle) / (2.0 * CGAL_PI))));
                double delta_angle = (end_angle - start_angle) / segnum;
                Vertex_handle existing_vertex;
                for (int sn = 1; sn < segnum; ++sn)
                {
                    double ang = start_angle + sn * delta_angle;
                    Point_2 current_point(
                        center[0] + radius * std::cos(ang),
                        center[1] + radius * std::sin(ang)
                    );

                    // 직선 세그먼트로 CDT에 삽입
                    cdt.insert_constraint(previous_point, current_point);

                    // 다음 세그먼트를 위해 현재 점을 이전 점으로 업데이트
                    previous_point = current_point;
                }

                // 끝점 추가
                Point_2 end_point(target[0], target[1]);
                existing_vertex = find_existing_vertex(cdt, previous_point, epsilon_);
                if (existing_vertex != Vertex_handle()) {
                    previous_point = existing_vertex->point();
                }
                existing_vertex = find_existing_vertex(cdt, end_point, epsilon_);
                if (existing_vertex != Vertex_handle()) {
                    end_point = existing_vertex->point();
                }
                
                cdt.insert_constraint(previous_point, end_point);

            }
            else {
                Point_2 source(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
                Point_2 target(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
                // 직선 세그먼트로 CDT에 삽입
                cdt.insert_constraint(source, target);
            }
        }
        */
    }
}