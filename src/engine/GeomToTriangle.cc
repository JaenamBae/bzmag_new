#include "GeomToTriangle.h"
#include "GeomHeadNode.h"
#include "BCNode.h"
#include "MasterPeriodicBCNode.h"
#include "SlavePeriodicBCNode.h"
#include "MovingBandNode.h"
#include "core/simplepropertybinder.h"
#include "engine/print_utils.h"
#include <CGAL/Arr_simple_point_location.h>
#include <CGAL/Arr_batched_point_location.h>
#include <CGAL/Surface_sweep_2_algorithms.h>
#include <CGAL/Arr_circular_arc_traits_2.h>

// AABB Tree 관련 정의
//#include <CGAL/AABB_face_graph_triangle_primitive.h>
//#include <CGAL/AABB_traits.h>
//#include <CGAL/AABB_tree.h>

#include <spdlog/spdlog.h>

using namespace bzmag;
using namespace bzmag::engine;


IMPLEMENT_CLASS(GeomToTriangle, Node);

int32 GeomToTriangle::unique_id_ = 0;
int32 GeomToTriangle::gmsh_entity_id_ = 1;
int32 GeomToTriangle::based_total_elements_num_ = 5000;
float64 GeomToTriangle::based_angle_ = 4;   // deg단위
float64 GeomToTriangle::tol_ = 1e-3;       // 동일점 인정 오차

//-----------------------------------------------------------------------------
GeomToTriangle::GeomToTriangle()
{

}

//-----------------------------------------------------------------------------
GeomToTriangle::~GeomToTriangle()
{

}

bool GeomToTriangle::setPath(const String& geom_path, const String& bc_path)
{
    Kernel* kernel = Kernel::instance();

    // Geoemtry 생성을 위한 루트 노드를 가져온다
    Node* node = kernel->lookup(geom_path);
    if (node == 0) return false;

    geom_path_ = geom_path;
    bc_path_ = bc_path;

    return true;
}

//----------------------------------------------------------------------------
bool GeomToTriangle::generateGmshStructures(const String& geom_path, const String& bc_path, std::function<void(int, int)> progressCallback)
{
    if (geom_path.empty() || bc_path.empty())
        return false;

    geom_path_ = geom_path;
    bc_path_ = bc_path;

    clear();
    done_configure_ = false;

    // 진행률 초기화
    progress_ = 0;

    // 정지 플래그 초기화
    stop_flag_ = false;

    spdlog::info("Make gmsh input structure..."); 
    //------------------------------------------------------------------
    // Step 0. 준비작업
    Kernel* kernel = Kernel::instance();

    // Geoemtry 생성을 위한 루트 노드를 가져온다
    Node* node = kernel->lookup(geom_path_);
    if (node == 0) return false;

    // 경계조건 노드를 셋팅한다
    Node* bc_node = kernel->lookup(bc_path_);
    mb_ = nullptr;
    if (bc_node) {
        for (auto it = bc_node->firstChildNode(); it != bc_node->lastChildNode(); ++it)
        {
            Node* child = *it;

            BCNode* BC = dynamic_cast<BCNode*>(child);
            if (BC) {
                bc_nodes_.push_back(BC);
            }

            MovingBandNode* mb = dynamic_cast<MovingBandNode*>(child);
            if (mb) {
                // 이미 무빙밴드 조건이 존재했다면 실패; 무빙조건은 하나만 지원한다
                if (mb_) {
                    return false;
                }
                mb_ = mb;

                // 이하 꼭 호출해서 mb 데이터를 생성해야 함
                if (!mb_->checkValid()) {
                    spdlog::error("Moving condition is not valid");
                    return false;
                }
            }
        }
    }
    
    //------------------------------------------------------------------
    // Step 1. 모든 Polyset을 Polygon_with_holes 로 만들어 작업할 준비를 함
    spdlog::info("(1/7) check domain and generate based domains");
    int32 unique_id = 0;

    int current_count = 0;
    for (auto node_it = node->firstChildNode(); node_it != node->lastChildNode(); ++node_it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        Node* childnode = *node_it;

        // 무빙밴드가 참조하는 헤더는 따로 처리할 것임
        GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(childnode);
        if (mb_ && mb_->getReferedHead() == head) {
            continue;
        }

        if (head && head->isModelNode() && head->isStandAlone())
        {
            int32 region_id = head->getID();
            if (head->isCovered()) {
                const Polygon_set_2& polyset = head->getPolyset();

                std::list<Polygon_with_holes_2> res;
                polyset.polygons_with_holes(std::back_inserter(res));
                for (auto it = res.begin(); it != res.end(); ++it)
                {
                    if (!registerPolygonWithHoles(region_id, *it)) {
                        return false;
                    }
                }

                domain_info_[region_id].name = head->getName();
                domain_info_[region_id].area = 0;
                domain_info_[region_id].required_number_of_elements = head->getNumberOfElements();
            }
        }

        if (progressCallback) {
            progress_ = calculateProgress((int)node->getNumChildren(), ++current_count);
            progressCallback(1, progress_);
        }
    }

    // 무빙밴드영역 추가
    if (mb_) {
        GeomHeadNode* head = mb_->getReferedHead();

        {
            int32 region_id = mb_->getOuterAirgapID();
            const Polygon_set_2& polyset = mb_->getOuter();

            std::list<Polygon_with_holes_2> res;
            polyset.polygons_with_holes(std::back_inserter(res));
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                if (!registerPolygonWithHoles(region_id, *it)) {
                    return false;
                }
            }

            std::string name(mb_->getName().c_str());
            std::string mb_name = "MB_" + name + "_Outer";
            domain_info_[region_id].name = mb_name;
            domain_info_[region_id].area = 0;
            domain_info_[region_id].required_number_of_elements = (int)(head->getNumberOfElements() / 2.0);
        }

        {
            int32 region_id = mb_->getInnerAirgapID();
            const Polygon_set_2& polyset = mb_->getInner();

            std::list<Polygon_with_holes_2> res;
            polyset.polygons_with_holes(std::back_inserter(res));
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                if (!registerPolygonWithHoles(region_id, *it)) {
                    return false;
                }
            }

            std::string name(mb_->getName().c_str());
            std::string mb_name = "MB_" + name + "_Inner";
            domain_info_[region_id].name = mb_name;
            domain_info_[region_id].area = 0;
            domain_info_[region_id].required_number_of_elements = (int)(head->getNumberOfElements() / 2.0);
        }
    }

    //------------------------------------------------------------------
    // Step 2. 모든 커브와 절점을 뽑고, 동일점 오차를 적용한 커브상의 절점을 만든다
    spdlog::info("(2/7) find extra based points");
    std::list<X_monotone_curve_2> curves;
    current_count = 0;
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        std::list<X_monotone_curve_2> outer, inner;
        Polygon_with_holes_2& polyholes = it->second.polyholes;
        extractCurvesFromPolyholes(polyholes, curves, curves);
    }
    std::list<Traits_2::Point_2> points;
    for (const auto& curve : curves) {
        points.push_back(curve.source());
        points.push_back(curve.target());
    }
    points.unique();
    for (const auto& curve : curves) {
        if (stop_flag_) {
            clear();
            return false;
        }

        for (const auto& point : points) {
            if (isPointWithinBoundingBox(curve, point)) {
                Traits_2::Point_2 intersect_point;
                if (isPointOnCurveWithTolerence(curve, point, intersect_point)) {
                    extra_points_.push_back(intersect_point);
                }
            }
        }
        if (progressCallback) {
            progress_ = calculateProgress((int)curves.size(), ++current_count);
            progressCallback(2, progress_);
        }
    }

    //------------------------------------------------------------------
    // Step 3. 서로 겹치지 않는 독립적인 서브-도메인(UniquePolyHoles)을 만들어준다.
    //         동시에 기저절점을 뽑는다
    spdlog::info("(3/7) generate unique domains");
    current_count = 0;
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        Polygon_with_holes_2& polyholes = it->second.polyholes;
        Polygon_set_2 polyset(polyholes);
        for (auto id : it->second.included_ids) {
            polyset.difference(polyholes_[id].polyholes);
        }
        it->second.domain = polyset;

        // 원에서 링을 빼면 기존 폴리곤이 2개로 쪼개질수 있다
        // 따라서 이러한 경우를 대비해서 polyset을 다시 정비한다
        std::vector<Polygon_with_holes_2> res;
        polyset.polygons_with_holes(std::back_inserter(res));

        assert(res.size() != 0);

        // 마지막 요소를 원래 polyholes_ 의 unique_polyholes 로 설정한다
        it->second.unique_polyholes = res.back();
        res.pop_back();

        // 기저절점 만들기; unique_polyholes 대상
        generateBasedPointsFromPolyholes(it->second.unique_polyholes);
        
        // 기존 polyholes이 두조각 이상으로 분리되었다면, 면적과 요구 요소수를 재계산 해야 한다
        if (res.size() > 0) {
            // 나머지가 더 있다면 새로운 Polyhole을 만들어 등록한다(polyholes 에 등록)
            for (auto pp = res.begin(); pp != res.end(); ++pp) {
                UniquePolyHoles new_polyholes;
                new_polyholes.region_id = it->second.region_id;
                new_polyholes.polyholes = *pp;

                // addPolygonWithHoles()을 이용해 등록하지 않는다
                // 이미 모든 작업이 끝났기 때문이다.
                polyholes_[unique_id_] = new_polyholes;
                unique_id_++;
            }
        }

        if (progressCallback) {
            progress_ = calculateProgress((int)polyholes_.size(), ++current_count);
            progressCallback(3, progress_);
        }
    }
    
    //------------------------------------------------------------------
    // Step 4. 기저절점으로 unique_polyholes을 이루는 커브 세그먼트 나누기
    //         refined_polyholes 을 만듦
    spdlog::info("(4/7) refine the unique domains");
    current_count = 0;
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        Polygon_with_holes_2& polyholes = it->second.unique_polyholes;
        Polygon_with_holes_2 refined_polyholes = refinePolyholesWithBasedPoints(polyholes);

        int p_holes = polyholes.number_of_holes();
        int n_holes = refined_polyholes.number_of_holes();
        assert(p_holes == n_holes);

        it->second.refined_polyholes = refined_polyholes;

        if (progressCallback) {
            progress_ = calculateProgress((int)polyholes_.size(), ++current_count);
            progressCallback(4, progress_);
        }
    }

    //------------------------------------------------------------------
    // Step 5 서브 도메인별 면적 및 요소 나누는 기준인 based_length 결정
    // Step 5.1 서브 도메인 별 면적 계산
    spdlog::info("(5/7) calculate area of the unique domains");
    current_count = 0;
    for (auto it = polyholes_.begin(); it!=polyholes_.end(); ++ it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        // unique_polyholes로 작업한다
        Polygon_with_holes_2& polyholes = it->second.unique_polyholes;
        int32 region_id = it->second.region_id;
        int32 unique_polyholes_id = it->first;

        // 길이기준 1개로, 각도 기준 1도로 아크를 나눈다
        Polygon_with_holes_2 segmented_polyholes = segmentPolyholes(polyholes, unique_polyholes_id, 0, 1);
        it->second.area = calculatePolyholeArea(segmented_polyholes, unique_polyholes_id);
        
        domain_info_[region_id].area += it->second.area;

        if (progressCallback) {
            int progress = calculateProgress((int)(polyholes_.size()*2), ++current_count);
            progressCallback(5, progress_);
        }
    }
    // Step 5.2 도메인 전체 면적 구하기
    float64 total_area = 0;
    for (auto domain = domain_info_.begin(); domain != domain_info_.end(); ++domain) {
        total_area += domain->second.area;
    }
    // Step 5.3 서브도메인 별 요구 요소수 및 세그먼트 시 참고할 세그먼트 기준 세우기
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        Polygon_with_holes_2& polyholes = it->second.refined_polyholes;
        int32 region_id = it->second.region_id;

        float64 domain_area = domain_info_[region_id].area;
        float64 my_area = it->second.area;
        int32 required_number_of_elements = domain_info_[region_id].required_number_of_elements;
        if (required_number_of_elements <= 0) {
            required_number_of_elements = (int32)(based_total_elements_num_ * domain_area / total_area);
            domain_info_[region_id].required_number_of_elements = required_number_of_elements;
        }

        int32 my_num_elements = (int32)(required_number_of_elements * my_area / domain_area);
        if (my_num_elements == 0 && my_area > 0) {
            spdlog::warn("Required number of elements for the region {} is too small", region_id);
            my_num_elements = 1;
        }
        it->second.required_number_of_elements = my_num_elements;
        

        // 요소 면적을 기준으로 세그멘테이션 기준을 세운다
        // 정사각형이 두개의 직각 삼각형으로 만들어진다고 가정해서 based_length를 설정함
        it->second.based_length = std::sqrt(2 * my_area / (float64)my_num_elements);
        it->second.based_angle = based_angle_;

        if (progressCallback) {
            int progress = calculateProgress((int)(polyholes_.size() * 2), ++current_count);
            progressCallback(5, progress_);
        }
    }

    //------------------------------------------------------------------
    // Step 6. 세그멘테이션된 정보를 이용해 세그멘티드 폴리곤 만들기
    //         기저 절점이 CGAL::to_double() 에의해서 double 로 변환되었다가
    //         이를 기반으로 세그멘테이션 한 후 폴리곤을 재생성함
    //         따라서 커널에서 보장하던 Exactness 가 깨지는 시점임
    //         절점/세그먼트 정보 생성 시점임
    // Step 6.1 based_curves_ 추출
    // 도메인에 포함된 모든 커브를 추출한다; refined_polyholes대상
    spdlog::info("(6/7) segment boundaries of the hole domains");
    current_count = 0;
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        Polygon_with_holes_2& polyholes = it->second.refined_polyholes;
        int32 unique_polyholes_id = it->first;

        // 다음함수는 based_curves_를 채워준다
        // 참고) segmentPolyholes() 에서 based_curves_를 활용한다
        generateBasedCurveFromPolyHoles(polyholes, unique_polyholes_id);

        if (progressCallback) {
            int progress = calculateProgress((int)(polyholes_.size() * 2 + based_curves_.size()), ++current_count);
            progressCallback(6, progress_);
        }
    }

    // Step 6.2 based_curves_의 세그먼테이션 조건 설정
    std::list<X_monotone_curve_2> segments;
    for (auto it = based_curves_.begin(); it != based_curves_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        const X_monotone_curve_2& curve = it->curve;
        float64 based_length = 0;
        float64 based_angle = 0;
        for (auto id : it->unique_polyholes_ids) {
            based_length += polyholes_[id].based_length;
            based_angle += polyholes_[id].based_angle;
        }
        // 겹처진 커브인 경우 커브를 공유하는 polyholes들의 평균값을 이용한다
        based_length /= (float64)(it->unique_polyholes_ids.size()); //unique_polyholes_ids의 갯수는 1 혹은 2개이다
        based_angle /= (float64)(it->unique_polyholes_ids.size());
        it->based_angle = based_angle;
        it->based_length = based_length;

        // based curves를 세그멘테이션 한다; 
        segmentCurve(it->curve, it->segmented_curve, it->based_length, it->based_angle);

        if (progressCallback) {
            int progress = calculateProgress((int)(polyholes_.size() * 2 + based_curves_.size()), ++current_count);
            progressCallback(6, progress_);
        }
    }

//#ifdef _DEBUG
//    for (auto it = based_curves_.begin(); it != based_curves_.end(); ++it)
//    {
//        std::cout << "Based Curves : " << it->curve << std::endl;
//    }
//#endif

    // Step 6.3 segmented based curved를 이용한 세그멘티드 폴리곤 만들기
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        int32 unique_polyholes_id = it->first;
        Polygon_with_holes_2& polyholes = it->second.refined_polyholes;

        // based_length를 -1로 주어 based_curves_ 정보 사용하게 함
        Polygon_with_holes_2 segmented_polyholes = segmentPolyholes(polyholes, unique_polyholes_id, -1, -1);
        it->second.segmented_polyholes = segmented_polyholes;

        if (progressCallback) {
            int progress = calculateProgress((int)(polyholes_.size() * 2 + based_curves_.size()), ++current_count);
            progressCallback(6, progress_);
        }
    }
    
    //------------------------------------------------------------------
    // Step 7. Triangle 입력 데이터의 Region과 Hole 데이터 만드는 시점임
    spdlog::info("(7/7) genetate geometry data for the gmsh");
    // triangle 용 데이터 생성
    //generateTriangleData();
    /*
    current_count = 0;
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        if (stop_flag_) {
            clear();
            return false;
        }

        if (it->second.area == 0) {
            continue;
        }
        int32 region_id = it->second.region_id;
        Polygon_with_holes_2& polyholes = it->second.refined_polyholes;
        const Polygon_2& outer = polyholes.outer_boundary();

        // 내부 홀경계
        for (auto hit = polyholes.holes_begin(); hit != polyholes.holes_end(); ++hit)
        {
            std::vector<int> inner_curve_ids;
            const Polygon_2& inner = *hit;

        }
        if (progressCallback) {
            progress_ = calculateProgress((int)polyholes_.size(), ++current_count);
            progressCallback(7, progress_);
        }
    }
    */

    // gmsh용 데이터 생성
    // 서브도메인 만들기(refined_polyholes 로 작업한다)
    current_count = 0;
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it) {
        if (stop_flag_) {
            clear();
            return false;
        }

        if (it->second.area == 0) {
            continue;
        }

        int32 domain_id = it->first;
        int32 region_id = it->second.region_id;
        Polygon_with_holes_2& polyholes = it->second.refined_polyholes;

        // 외곽경계
        std::vector<int> outer_curve_ids;
        const Polygon_2& outer = polyholes.outer_boundary();
        generateGmshSurface(outer, outer_curve_ids, domain_id);

        // 쓰레기 영역에 대해서는 패쓰하되, 요소 싸이즈를 0으로 만들어 둠
        // 추후 gmsh 메쉬 사이즈 정할때 참조되기 때문임
        if (outer_curve_ids.size() < 2) {
            it->second.based_length = 0;
            continue;
        }
        if ((outer_curve_ids.size() == 2) && (outer_curve_ids[0] == -outer_curve_ids[1])) {
            it->second.based_length = 0;
            continue;
        }

        GLoop outer_loop(gmsh_entity_id_++, outer_curve_ids);
        int outer_loop_id = insertGLoop(outer_loop);

        // 내부 홀경계
        std::vector<int> inner_loop_ids;
        for (auto hit = polyholes.holes_begin(); hit != polyholes.holes_end(); ++hit)
        {
            std::vector<int> inner_curve_ids;
            const Polygon_2& inner = *hit;
            generateGmshSurface(inner, inner_curve_ids, domain_id);
            if (inner_curve_ids.size() < 2) {
                it->second.based_length = 0;
                continue;
            }
            if ((inner_curve_ids.size() == 2) && (inner_curve_ids[0] == -inner_curve_ids[1])) {
                it->second.based_length = 0;
                continue;
            }
            GLoop inner_loop(gmsh_entity_id_++, inner_curve_ids);
            int inner_loop_id = insertGLoop(inner_loop);
            inner_loop_ids.push_back(inner_loop_id);
        }

        GSurface surface(gmsh_entity_id_++, outer_loop_id, inner_loop_ids);
        gmsh_surfaces_[region_id].push_back(surface);

        if (progressCallback) {
            progress_ = calculateProgress((int)polyholes_.size(), ++current_count);
            progressCallback(7, progress_);
        }
    }

    spdlog::info("Complete to generate mesh structure");
    done_configure_ = true;
    return true;
}

//----------------------------------------------------------------------------
void GeomToTriangle::clear()
{
    unique_id_ = 0;
    gmsh_entity_id_ = 1;

    polyholes_.clear();
    domains_.clear();
    domain_info_.clear();
    based_points_.clear();
    based_curves_.clear();

    vertices_.clear();
    vertices_maker_.clear();
    segments_.clear();
    segment_makers_.clear();
    regions_.clear();
    holes_.clear();
    gmsh_related_domains_.clear();
    gmsh_points_.clear();
    gmsh_curves_.clear();
    gmsh_bc_curves_.clear();
    gmsh_bc_points_.clear();
    gmsh_loops_.clear();
    gmsh_surfaces_.clear();

    bc_nodes_.clear();

    mb_ = nullptr;
    stop_flag_ = false;
}

int GeomToTriangle::calculateProgress(int total_step, int current_step)
{
    float based_progress = float(current_step-1) / float(total_step);
    return (int)(based_progress * 100);
}

//----------------------------------------------------------------------------
bool GeomToTriangle::registerPolygonWithHoles(int32 region_id, const Polygon_with_holes_2& polyholes)
{
    //if (polyholes.is_empty()) return false;
    UniquePolyHoles new_polyholes;
    new_polyholes.region_id = region_id;
    new_polyholes.polyholes = polyholes;

    Bbox_2 B1 = polyholes.outer_boundary().bbox();
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        UniquePolyHoles& given_polygon = it->second;

        // 우선 박스로 서로 독립인지 간편 테스트
        Bbox_2 B2 = given_polygon.polyholes.outer_boundary().bbox();

        // B2에 B1이 포함되는지
        bool B2_contain_B1((B2.xmin() <= B1.xmin() && B2.xmax() >= B1.xmax() &&
            B2.ymin() <= B1.ymin() && B2.ymax() >= B1.ymax()));

        // B1에 B2이 포함되는지
        bool B1_contain_B2((B1.xmin() <= B2.xmin() && B1.xmax() >= B2.xmax() &&
            B1.ymin() <= B2.ymin() && B1.ymax() >= B2.ymax()));

        // 서로 독립이면 더 연산할 필요가 없음
        if (!B2_contain_B1 && !B1_contain_B2) {
            continue;
        }

        bool same_bounding_box = B2_contain_B1 && B1_contain_B2;

        // 간편테스트로 B1이 B2에 포한된 것이라면 더 정밀하게 연산해봄
        Polygon_set_2 A(polyholes);
        if ((B2_contain_B1 && !B1_contain_B2) || same_bounding_box) {
            // A-B = 0이면, A가 B에 포함된 것임
            A.difference(given_polygon.polyholes);
            if (A.is_empty()) {
                // polyholes을 위한 id가 현재의 unique_id_가 될 것이다 
                given_polygon.included_ids.push_back(unique_id_);
                continue;
            }
        }

        // 간편테스트로 B2이 B1에 포한된 것이라면 더 정밀하게 연산해봄
        Polygon_set_2 B(given_polygon.polyholes);
        if ((B1_contain_B2 && !B2_contain_B1) || same_bounding_box) {
            // B-A = 0이면, B가 A에 포함된 것임
            B.difference(polyholes);
            if (B.is_empty()) {
                new_polyholes.included_ids.push_back(it->first);
                continue;
            }
        }

        if (A.is_empty() && B.is_empty()) {
            spdlog::error("Fail to generate .poly structure. Domain {} and {} are the same!", region_id, it->second.region_id);
            return false;
        }

        // 포함관계가 아닌데 공통 부분이 있다면 서로 완전 독립이 아니므로 거짓이나
        // 살짝 겹쳐 있는 경우가 있을 수 있으므로, 체크
        Polygon_set_2 C(polyholes);
        C.intersection(given_polygon.polyholes);
        bool no_intersection = true;
        if (!C.is_empty()) {
            std::list<Polygon_with_holes_2> res;
            C.polygons_with_holes(std::back_inserter(res));
            for (auto ph = res.begin(); ph != res.end(); ++ph) {

                // 직선은 1개, 곡선은 1도단위로 커브를 세그먼트해서 겹치는 영역의 면적을 구함
                // 세그멘테이션시 아주 짧은 길이의 커브는 무시됨
                Polygon_with_holes_2 segmented_overlaped = segmentPolyholes(*ph, -1, 0, 1);
                float64 area = calculatePolyholeArea(segmented_overlaped, -1);
                if (area > 1e-3) {
                    spdlog::error("Fail to generate .poly structure. Domain {} and {} are overlapped", region_id, it->second.region_id);
                    std::cout << "Overlaped polyholes: " << std::endl;
                    print_general_polygon_with_holes(*ph);
                    return false;
                }
            }
        }
    }

    // 실제로 polyholes_ 추가
    polyholes_[unique_id_] = new_polyholes;
    unique_id_++;

    return true;
}

bool GeomToTriangle::isPointOnCurveWithTolerence(const X_monotone_curve_2& curve, 
    const Traits_2::Point_2& point, 
    Traits_2::Point_2& intersection_point)
{
    if ((curve.source() == point) || (curve.target() == point)) {
        return false;
    }

    float64 pt_x = CGAL::to_double(point.x());
    float64 pt_y = CGAL::to_double(point.y());

    // 1. Compute the perpendicular direction and endpoints
    float64 dir_x, dir_y;
    if (!curve.is_circular()) {
        const auto& v_line = curve.supporting_line().to_vector();
        dir_x = CGAL::to_double(v_line.x());
        dir_y = CGAL::to_double(v_line.y());
    }
    else {
        const auto& center = curve.supporting_circle().center();
        dir_x = CGAL::to_double(pt_x - center.x());
        dir_y = CGAL::to_double(pt_y - center.y());
    }

    // Normalize the perpendicular vector and scale to the given length
    float64 dir_length = std::sqrt(CGAL::to_double(dir_x * dir_x + dir_y * dir_y));
    if (dir_length == 0) return false; // Avoid division by zero

    float64 perpendicular_dir_x = -dir_y / dir_length * 2 * tol_;
    float64 perpendicular_dir_y =  dir_x / dir_length * 2 * tol_;

    Point_2 p1(pt_x + perpendicular_dir_x, pt_y + perpendicular_dir_y);
    Point_2 p2(pt_x - perpendicular_dir_x, pt_y - perpendicular_dir_y);

    // 2. Create the perpendicular segment
    X_monotone_curve_2 perpendicular_curve(p1, p2);

    // Create an arrangement
    Arrangement arr;

    // Insert the curves into the arrangement
    CGAL::insert(arr, curve);
    CGAL::insert(arr, perpendicular_curve);

    // Iterate over vertices to find intersection points
    for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
        if (vit->degree() > 1) { // Intersection points have degree > 1
            intersection_point = vit->point();
            return true;
        }
    }

    return false;
}

bool GeomToTriangle::isPointWithinBoundingBox(const X_monotone_curve_2& curve, const Traits_2::Point_2& point)
{
    auto min_x = CGAL::to_double(std::min(curve.source().x(), curve.target().x()));
    auto max_x = CGAL::to_double(std::max(curve.source().x(), curve.target().x()));
    auto min_y = CGAL::to_double(std::min(curve.source().y(), curve.target().y()));
    auto max_y = CGAL::to_double(std::max(curve.source().y(), curve.target().y()));

    min_x -= (10 * tol_);
    max_x += (10 * tol_);
    min_y -= (10 * tol_);
    max_y += (10 * tol_);

    auto pt_x = CGAL::to_double(point.x());
    auto pt_y = CGAL::to_double(point.y());

    return (pt_x >= min_x && pt_x <= max_x && pt_y >= min_y && pt_y <= max_y);
}

//-----------------------------------------------------------------------------
void GeomToTriangle::extractCurvesFromPolyholes(const Polygon_with_holes_2& polyholes, 
    std::list<X_monotone_curve_2>& outer_curves, 
    std::list<X_monotone_curve_2>& hole_curves)
{
    // 외곽경계 커브 추출
    const Polygon_2& outer = polyholes.outer_boundary();
    for (auto curve = outer.curves_begin(); curve != outer.curves_end(); ++curve) {
        outer_curves.push_back(*curve);
    }

    // 내부 홀경계 커브 추출
    for (auto hit = polyholes.holes_begin(); hit != polyholes.holes_end(); ++hit)
    {
        const Polygon_2& inner = *hit;
        for (auto curve = inner.curves_begin(); curve != inner.curves_end(); ++curve) {
            hole_curves.push_back(*curve);
        }
    }
}

void GeomToTriangle::generateBasedPointsFromPolyholes(const Polygon_with_holes_2& polyholes)
{
    std::list<X_monotone_curve_2> outer_curves;
    std::list<X_monotone_curve_2> hole_curves;
    extractCurvesFromPolyholes(polyholes, outer_curves, hole_curves);

    // Arrangement에 curves 삽입
    Arrangement test_arr;
    CGAL::insert(test_arr, outer_curves.begin(), outer_curves.end());
    CGAL::insert(test_arr, hole_curves.begin(), hole_curves.end());

    // 외곽 경계와 내부경계가 만나는 점이 있으면 확실한 세그멘테이션을 위해 짧은 수직선 추가
    std::list<X_monotone_curve_2> extra_curves;
    for (auto outer_curve = outer_curves.begin(); outer_curve != outer_curves.end(); ++outer_curve) {
        for (auto hole_curve = hole_curves.begin(); hole_curve != hole_curves.end(); ++hole_curve) {
            // 커브 둘다 아크인경우
            if (outer_curve->is_circular() && hole_curve->is_circular()) {
                const Circle_2& outer_circle = outer_curve->supporting_circle();
                float64 radii_o = sqrt(CGAL::to_double(outer_circle.squared_radius()));
                Vert    center_o{ CGAL::to_double(outer_circle.center().x()), CGAL::to_double(outer_circle.center().y()) };

                const Circle_2& hole_circle = hole_curve->supporting_circle();
                float64 radii_i = sqrt(CGAL::to_double(hole_circle.squared_radius()));
                Vert    center_i{ CGAL::to_double(hole_circle.center().x()), CGAL::to_double(hole_circle.center().y()) };

                float64 dx = center_i[0] - center_o[0];
                float64 dy = center_i[1] - center_o[1];
                float64 dist_centers = sqrt(dx * dx + dy * dy);
                if (abs(dist_centers + radii_i - radii_o) < tol_) {
                    Point_2 new_point1(center_o[0] + dx * radii_o / dist_centers * 0.9,
                        center_o[1] + dy * radii_o / dist_centers * 0.9);

                    Point_2 new_point2(center_o[0] + dx * radii_o / dist_centers * 1.1,
                        center_o[1] + dy * radii_o / dist_centers * 1.1);

                    X_monotone_curve_2 new_curve(new_point1, new_point2);
                    extra_curves.push_back(new_curve);
                }
            }
        }
    }
    CGAL::insert(test_arr, extra_curves.begin(), extra_curves.end());

    // based_points_에 절점 삽입
    for (auto point = test_arr.vertices_begin(); point != test_arr.vertices_end(); ++point) {
        based_points_.push_back(point->point());
    }
    based_points_.unique(); //동일점 없애기
}

//----------------------------------------------------------------------------
Polygon_with_holes_2 GeomToTriangle::refinePolyholesWithBasedPoints(const Polygon_with_holes_2& polyholes)
{
    // 외곽경계 커브 추출
    std::list<X_monotone_curve_2> outer_curves;
    std::list<X_monotone_curve_2> hole_curves;
    extractCurvesFromPolyholes(polyholes, outer_curves, hole_curves);

    // Arrangement 에 커브, 절점 추가
    Arrangement arr;
    CGAL::insert(arr, outer_curves.begin(), outer_curves.end());
    CGAL::insert(arr, hole_curves.begin(), hole_curves.end());

    // Arrangement에 기저절점 삽입; 기저절점이 세그먼트상에 있는 경우 자동으로 세그먼트 분할됨
    for (auto point : based_points_) {
        CGAL::insert_point(arr, point);
    }
    for (auto point : extra_points_) {
        CGAL::insert_point(arr, point);
    }
    
    // 새롭게 Polyholes을 구성하여 리턴한다
    std::vector<Polygon_with_holes_2> polygons_with_holes;
    rebuildPolyHoles(arr, polygons_with_holes);

    assert(polygons_with_holes.size() == 1);

    if (polygons_with_holes.size() > 0)
        return polygons_with_holes[0];
    else
        return Polygon_with_holes_2();
}

//----------------------------------------------------------------------------
// 리빌드 하기 전에 Arrangement에는 한개의 Polygon_with_holes_2 정보만 담겨야 하고
// 리빌드 하고 나서도 한개의 Polygon_with_holes_2이 만들어져야 한다.
void GeomToTriangle::rebuildPolyHoles(Arrangement& arr, std::vector<Polygon_with_holes_2>& polygons_with_holes)
{
    std::vector<Polygon_2> outer_boundary;
    std::vector<Polygon_2> holes;

    // Arrangement의 면 순회
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        if (!fit->is_unbounded()) {
            // 경계 엣지들을 수집하여 다각형 생성
            std::list<X_monotone_curve_2> curves;

            auto edge_iter = fit->outer_ccb();
            const X_monotone_curve_2& curve = edge_iter->curve();
            curves.push_back(curve);

            Traits_2::Point_2 ss = curve.source();
            Traits_2::Point_2 tt = curve.target();

            edge_iter++;
            for (; edge_iter != fit->outer_ccb(); ++edge_iter) {
                const X_monotone_curve_2& curve = edge_iter->curve();

                //bool inserted = false;
                //for (auto it = curves.begin(); it != curves.end(); ++it) {
                //    // curve가 curve2에 뒤에 붙어야 할 경우
                //    if (it->target() == curve.source()) {
                //        auto it2 = it;
                //        it2++;
                //        if (it2 == curves.end()) {
                //            curves.insert(it2, curve);
                //            inserted = true;
                //            break;
                //        }
                //        else if (it2->source() == curve.target())
                //        {
                //            curves.insert(it2, curve);
                //            inserted = true;
                //            break;
                //        }
                //    }
                //    else if (it->source() == curve.target()) {
                //        auto it2 = it;
                //        if (it2 != curves.begin()) {
                //            --it2;
                //            if (it2->target() == curve.source()) {
                //                curves.insert(it2, curve);
                //                inserted = true;
                //                break;
                //            }
                //        }
                //        else {
                //            curves.insert(it2, curve);
                //            inserted = true;
                //            break;
                //        }
                //    }
                //}
                //// 그렇지 않으면  끝에 삽입
                //if (!inserted)
                //    curves.push_back(curve);


                // 이전 끝점이 지금 커브의 시작점이면
                if (tt == curve.source()) {
                    curves.push_back(curve);
                }

                // 지금커브의 끝점이 이전커브의 시작점이면
                else {
                    curves.push_front(curve);
                }

                ss = curve.source();
                tt = curve.target();
            }

            while (rebuildPolygons(curves, outer_boundary, holes)) {}
        }
    }

    for (auto outer = outer_boundary.begin(); outer != outer_boundary.end(); ++outer) {
        Polygon_2& poly = *outer;
        Polygon_with_holes_2 polyholes(*outer);
        Polygon_set_2 polyset(polyholes);
        for (auto hole = holes.begin(); hole != holes.end(); ++hole) {
            Polygon_set_2 hh;
            try {
                hh = Polygon_set_2(*hole);  // 예외가 발생할 수 있는 코드
            }
            catch (const std::exception& e) {
                spdlog::error("Fail to rebuild polyholes: {}", e.what());
            }

            polyset.difference(hh); // 차집합 연산
        }

        std::list<Polygon_with_holes_2> res;
        polyset.polygons_with_holes(std::back_inserter(res));
        polygons_with_holes.assign(res.begin(), res.end());
    }
}

bool GeomToTriangle::rebuildPolygons(std::list<X_monotone_curve_2>& curves,
    std::vector<Polygon_2>& outer_boundary,
    std::vector<Polygon_2>& holes)
{
    // Polygon_with_holes 은 홀의 정의가 외부 바운드리 내에만 있으면 홀로 인정
    // 즉, 외부경계와, 홀의 경계가 일치하는 점이 있더라고 경계 내에만 있으면 홀로 인정
    // 하지만, Arrangement에서는 Face의 바운드리를 찾을때 다르게 해석할 수 있음
    std::vector<X_monotone_curve_2> poly(curves.begin(), curves.end());

    for (int i = 0; i < poly.size(); ++i) {
        const X_monotone_curve_2& c1 = poly[i];
        for (int j = 0; j < i; ++j) {
            const X_monotone_curve_2& c2 = poly[j];

            // j~i로 이루어진 폐곡선이 완성되었다
            if (c1.target() == c2.source()) {
                std::vector<X_monotone_curve_2> sub_curve(poly.begin() + j, poly.begin() + i + 1);
                Polygon_2 polygon(sub_curve.begin(), sub_curve.end());
                if (polygon.orientation() == CGAL::COUNTERCLOCKWISE) {
                    outer_boundary.push_back(polygon);
                }
                else {
                    polygon.reverse_orientation();
                    holes.push_back(polygon);
                }

                // 사용된 커브는 지운다
                auto it1 = curves.begin();
                std::advance(it1, j);       // j번째 원소

                auto it2 = it1;
                std::advance(it2, (i - j + 1));   // i+1번째 원소

                // i번째~j번째 원소 삭제
                curves.erase(it1, it2);

                return true;
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
std::pair<BCNode*, bool> GeomToTriangle::testCurveInBCNode(const X_monotone_curve_2& edge) const
{
    for (const auto& bc : bc_nodes_)
    {
        int test = bc->testCurve(edge);
        if (test != 0) {
            return std::pair<BCNode*, bool>(bc, test > 0);
        }
    }

    if (mb_) {
        {
            BCNode* bc = mb_->getOuterBCNode();
            int test = bc->testCurve(edge);
            if (test != 0) {
                return std::pair<BCNode*, bool>(bc, test > 0);
            }
        }
        {
            BCNode* bc = mb_->getInnerBCNode();
            int test = bc->testCurve(edge);
            if (test != 0) {
                return std::pair<BCNode*, bool>(bc, test > 0);
            }
        }
    }
    

    return std::pair<BCNode*, bool>(nullptr, true);
}

//----------------------------------------------------------------------------
// 하기 함수의 인자 polyholes는 세그멘트 되어 있어야 한다!
float64 GeomToTriangle::calculatePolyholeArea(const Polygon_with_holes_2& polyholes, int32 unique_polyholes_id)
{
    using Polygon = CGAL::Polygon_2<LK>;

    float64 total_area = 0;

    // 외부 경계의 면적 General Polygon_2가 아닌 그냥 Polygon_2를 사용한다
    Polygon segmented_outer;
    const Polygon_2& outer = polyholes.outer_boundary();
    for (auto seg = outer.curves_begin(); seg != outer.curves_end(); ++seg) {
        // 세그먼트 타입이 arc이면 면적을 못구한다
        if (seg->is_circular()) {
            spdlog::error("Can't calculate domain {} area becuase its outer boundaries are not segmented", unique_polyholes_id);
            return 0.0;
        }

        Traits_2::Point_2 point = seg->source();
        segmented_outer.push_back({ CGAL::to_double(point.x()), CGAL::to_double(point.y()) });
    }
    if (segmented_outer.is_empty()) return 0;

    assert(segmented_outer.is_simple());

    double outer_area = CGAL::to_double(segmented_outer.area());

    // 내부 홀의 면적을 합산(홀 세그먼트의 방향이 CW이므로, 면적이 마이너스로 나옴)
    double holes_area = 0;
    for (auto it = polyholes.holes_begin(); it != polyholes.holes_end(); ++it) {
        const Polygon_2& hole = *it;
        Polygon segmented_hole;
        for (auto seg = hole.curves_begin(); seg != hole.curves_end(); ++seg) {
            // 세그먼트 타입이 arc이면 면적을 못구한다
            if (seg->is_circular()) {
                spdlog::error("Can't calculate domain {} area becuase its hole boundaries are not segmented", unique_polyholes_id);
                return 0.0;
            }

            Traits_2::Point_2 point = seg->source();
            segmented_hole.push_back({ CGAL::to_double(point.x()), CGAL::to_double(point.y()) });
        }
        assert(segmented_hole.is_simple());
        assert(!segmented_hole.is_empty());
        holes_area += CGAL::to_double(segmented_hole.area());
    }

    // 최종 면적 = 외부 경계 면적 - 내부 홀들의 면적
    return outer_area + holes_area;
}

//----------------------------------------------------------------------------
Polygon_with_holes_2 GeomToTriangle::segmentPolyholes(const Polygon_with_holes_2& polyholes, int32 unique_polyholes_id,
    float64 based_length, float64 based_angle)
{
    // 외곽 커브 세그멘테이션
    const Polygon_2& outer = polyholes.outer_boundary();
    Polygon_2 segmented_outer = segmentPolygon(outer, unique_polyholes_id, based_length,  based_angle);

    // 세그먼트된 아우터 폴리곤이 비정상일수 있음
    //Polygon_with_holes_2 test(segmented_outer);
    //if (!test.is_plane()) return Polygon_with_holes_2();
    //if (test.is_unbounded()) return Polygon_with_holes_2();

    // 내부 홀들 커브 세그멘테이션
    Polygon_set_2 segmented_polyholes(segmented_outer);
    for (auto hit = polyholes.holes_begin(); hit != polyholes.holes_end(); ++hit)
    {
        Polygon_2 inner = *hit;
        inner.reverse_orientation();
        Polygon_2 segmented_hole = segmentPolygon(inner, unique_polyholes_id, based_length, based_angle);
        Polygon_with_holes_2 test(segmented_hole);
        if (!test.is_plane()) continue;
        if (test.is_unbounded()) continue;

        if (segmented_hole.orientation() == CGAL::CLOCKWISE) {
            segmented_hole.reverse_orientation();
        }

        segmented_polyholes.difference(segmented_hole);
    }

    std::list<Polygon_with_holes_2> res;
    segmented_polyholes.polygons_with_holes(std::back_inserter(res));

    if (res.size() > 0)
        return res.front();

    return Polygon_with_holes_2();
}

Polygon_2 GeomToTriangle::segmentPolygon(const Polygon_2& polygon, int32 unique_polyholes_id,
    float64 based_length, float64 based_angle)
{
    std::list<Vert> result;

    //std::cout << "Segment Polyholes:" << std::endl;
    //print_general_polygon(polygon);

    for (auto it = polygon.curves_begin(); it != polygon.curves_end(); ++it) {
        int32 curve_id = -1;
        float64 applied_based_length = based_length;
        float64 applied_based_angle = based_angle;
        std::list<Vert> seg_curves;
        const X_monotone_curve_2& curve = *it;

        // based_length, based_angle 가 주어지지 않은 경우 
        // based_curves_에서 정보를 찾아서 재설정하고
        // segmented_curves 도 가져온다.
        if (based_length < 0 || based_angle < 0) {
            int32 curve_id;
            bool direction;
            if (findBasedCurve(curve, curve_id, direction)) {
                const DomainCurve& based_curve = based_curves_[curve_id];
                applied_based_length = based_curve.based_length;
                applied_based_angle = based_curve.based_angle;

                seg_curves = based_curve.segmented_curve;
                std::list<Vert> temp_curve;
                if (!segmentCurve(curve, temp_curve, applied_based_length, applied_based_angle))
                    continue;

                if (!direction) {
                    seg_curves.reverse();
                }
            }
        }
        else {
            if (!segmentCurve(curve, seg_curves, applied_based_length, applied_based_angle))
                continue;
        }

        // 기존 커브의 끝점은 현재 커브의 시작점과 같을 것이므로
        if (result.size() > 0)
            result.pop_back();

        // 현재 세그먼트를 이전 세그먼트에 이어 붙임
        result.splice(result.end(), seg_curves);
    }

    // 실제 폴리곤을 만든다
    result.pop_back(); // 전체 결과의 끝점과 시작점은 (거의) 같아야 함 --> 끝점을 없애줌

    // 시작점 저장
    auto start_pt = result.front();
    Point_2 start_point{ start_pt[0],start_pt[1] };
    result.pop_front();

    // 세그먼트 만들기
    Point_2 prev_point = start_point;
    std::list<X_monotone_curve_2> segmented_poly;
    for (auto it = result.begin(); it != result.end(); ++it) {
        Point_2 current_point{ (*it)[0],(*it)[1] };
        segmented_poly.push_back(X_monotone_curve_2(prev_point, current_point));

        prev_point = current_point;
    }
    segmented_poly.push_back(X_monotone_curve_2(prev_point, start_point));

    filterPolygonBoundary(segmented_poly);
    if (segmented_poly.size() < 2) {
        return Polygon_2();
    }
    if (segmented_poly.size() == 2) {
        for (const auto& cc : segmented_poly) {
            if (!cc.is_circular()) {
                return Polygon_2();
            }
        }
    }

    // 폴리곤 만들어서 리턴
    return Polygon_2(segmented_poly.begin(), segmented_poly.end());
}

//----------------------------------------------------------------------------
bool GeomToTriangle::segmentCurve(const X_monotone_curve_2& edge, std::list<Vert>& result,
    float64 based_length, float64 based_angle)
{
    // result는 비어있어야 한다
    result.clear();

    // 커브의 시작점 끝점
    const Traits_2::Point_2& ss = edge.source();
    const Traits_2::Point_2& tt = edge.target();

    Vert source{ CGAL::to_double(ss.x()), CGAL::to_double(ss.y()) };
    Vert target{ CGAL::to_double(tt.x()), CGAL::to_double(tt.y()) };

    // 곡선이면, 바운드리 엣지인지의 여부와 상관없이 무조건 세그멘테이션
    if (edge.is_circular())
    {
        // 커브의 서포팅 원
        const Circle_2& circle = edge.supporting_circle();
        Point_2 cc = circle.center();
        Vert center{ CGAL::to_double(cc.x()) , CGAL::to_double(cc.y()) };
        float64 radii = std::sqrt(CGAL::to_double(circle.squared_radius()));

        float64 start_angle = std::atan2(source[1] - center[1], source[0] - center[0]);
        float64 end_angle = std::atan2(target[1] - center[1], target[0] - center[0]);

        float64 delta_angle = end_angle - start_angle;
        if (edge.orientation() == CGAL::COUNTERCLOCKWISE) {
            if (delta_angle < 0) delta_angle += (2 * CGAL_PI);
        }
        else {
            if (delta_angle > 0) delta_angle -= (2 * CGAL_PI);
        }

        // 세그먼트수는 아크를 10도간격으로 잘랐을때의 세그먼트 숫자와
        // 주어진 길이로 잘랐을때의 세그먼트의 숫자 중
        // 큰 것을 이용한다
        float64 arc_length = radii * abs(delta_angle);

        // 만약 선의 길이가 오차범위보다 작은 경우, 쓰래기 커브라 판단하고 리턴
        if (arc_length < tol_)
            return false;

        // 각도 기준이 없으면(0이면) 1도 기준으로 세그먼트한다
        int32 num_seg2 = 1;
        if (based_angle > 0) {
            num_seg2 = int32(abs(delta_angle) * 180.0 / CGAL_PI / based_angle);
        }
        else {
            num_seg2 = int32(abs(delta_angle) * 180.0 / CGAL_PI);
        }

        // 세그먼트 수; 일단 각도기준으로 세그먼트 수를 설정하고
        int32 num_seg = num_seg2;

        // 만약 길이제약이 존재했다면 길이 기준 세그먼트 수를 계산하여
        // 둘중 적은 것을 기준으로 삼는다
        if (based_length > 0) {
            int32 num_seg1 = 1;
            if (based_length > 0) num_seg1 = int32(arc_length / based_length);
            if (num_seg1 < 1) num_seg1 = 1;

            num_seg = std::min(num_seg1, num_seg2);
        }

        // 세그먼트 수를 짝수개로 조정한다
        //if ((num_seg % 2) == 1) num_seg = num_seg + 1;

        // 세그먼트 각도 ; 1도 근처가 될 것임
        float64 d_angle = delta_angle / num_seg;

        // 일단 시작점 추가,
        result.push_back(source);

        // 세그멘테이션을 위한 절점(시작점과 끝점은 제외)
        for (int32 i = 1; i < num_seg; ++i) {
            Vert point{ center[0] + radii * cos(start_angle + i * d_angle),
                     center[1] + radii * sin(start_angle + i * d_angle) };
            result.push_back(point);
        }

        // 마지막점도 추가
        result.push_back(target);
    }

    // 직선인 경우
    else
    {
        // 벡터 길이
        float64 line_length = sqrt((source[0] - target[0]) * (source[0] - target[0])
            + (source[1] - target[1]) * (source[1] - target[1]));

        // 만약 선의 길이가 오차범위보다 작은 경우, 쓰래기 커브라 판단하고 리턴
        if (line_length < tol_) 
            return false;

        int32 num_seg = 1;
        if (based_length > 0) {
            num_seg = (int32)(line_length / based_length);
            if (num_seg < 1) num_seg = 1;
        }

        // 짝수로 만든다
        //if ((num_seg % 2) == 1) num_seg = num_seg + 1;

        // 일단 시작점 추가,
        result.push_back(source);

        // 라인을 세그멘테이션 한다
        for (int32 i = 1; i < num_seg; ++i) {
            Vert point{ source[0] + (target[0] - source[0]) * ((float64)i / (float64)num_seg),
                     source[1] + (target[1] - source[1]) * ((float64)i / (float64)num_seg) };
            result.push_back(point);
        }

        // 마지막점 추가
        result.push_back(target);
    }

    return true;
}

void GeomToTriangle::filterPolygonBoundary(std::list<X_monotone_curve_2>& curves)
{
    bool found;
    do {
        found = false;
        for (auto it = curves.begin(); it != curves.end(); ) {
            auto next_it = std::next(it);

            // 다음 커브가 있는지 확인
            if (next_it != curves.end()) {
                // 반대방향 커브인지
                bool are_reversed = (it->source() == next_it->target() && it->target() == next_it->source());

                if (are_reversed) {
                    // 현재 커브와 다음 커브 삭제
                    it = curves.erase(it);      // 현재 커브 삭제
                    it = curves.erase(it);      // 다음 커브 삭제
                    found = true;               // 삭제가 발생했음을 표시
                    break;                      // 루프를 다시 시작
                }
                else {
                    ++it; // 다음 커브로 이동
                }
            }
            else {
                ++it; // 다음 커브로 이동
            }
        }
    } while (found); // 더 이상 삭제할 커브가 없을 때까지 반복
}

//----------------------------------------------------------------------------
int32 GeomToTriangle::insertVertex(const Vert& vert, int32 boundary_marker)
{
    for (int32 i = 0; i < vertices_.size(); ++i) {
        const Vert& v = vertices_[i];
        float64 dx = v[0] - vert[0];
        float64 dy = v[1] - vert[1];
        float64 delta = std::sqrt(dx * dx + dy * dy);
        if (delta < tol_) 
            return i;
    }
    vertices_.push_back(vert);
    vertices_maker_.push_back(boundary_marker);
    return ((int32)vertices_.size() - 1);
}

//----------------------------------------------------------------------------
int32 GeomToTriangle::insertSegment(const Seg& seg, int32 boundary_marker)
{
    for (int32 i = 0; i < segments_.size(); ++i) {
        const Seg& s = segments_[i];

        // 동일한 세그먼트 존재하면 해당 인덱스 리턴
        if ((seg[0] == s[0]) && (seg[1] == s[1]))
            return  i;

        // 동일한 세그먼트 존재하는데, 방향이 반대면, -붙인 인덱스 리턴
        if ((seg[0] == s[1]) && (seg[1] == s[0]))
            return -i;
    }

    // 동일 세그먼트 존재하지 않으면 새롭게 추가후 인덱스 리턴
    segments_.push_back(seg);
    segment_makers_.push_back(boundary_marker);
    return ((int32)segments_.size() - 1);
}

//----------------------------------------------------------------------------
void GeomToTriangle::generateBasedCurveFromPolyHoles(const Polygon_with_holes_2& polyholes, int32 unique_polyholes_id)
{
    const Polygon_2& outer = polyholes.outer_boundary();
    generateBasedCurveFromPolygon(outer, unique_polyholes_id);

    for (auto it = polyholes.holes_begin(); it != polyholes.holes_end(); ++it) {
        const Polygon_2& hole = *it;
        generateBasedCurveFromPolygon(hole, unique_polyholes_id);
    }
}

//----------------------------------------------------------------------------
void GeomToTriangle::generateBasedCurveFromPolygon(const Polygon_2& polygon, int32 unique_polyholes_id)
{
    for (auto it = polygon.curves_begin(); it != polygon.curves_end(); ++it)
    {
        const X_monotone_curve_2& my_curve = *it;
        int32 curve_id;
        bool direction;
        if (findBasedCurve(my_curve, curve_id, direction)) {
            based_curves_[curve_id].unique_polyholes_ids.push_back(unique_polyholes_id);
        }
        else {
            DomainCurve new_curve;
            new_curve.curve = my_curve;
            new_curve.unique_polyholes_ids.push_back(unique_polyholes_id);
            based_curves_.push_back(new_curve);
        }
    }
}

//----------------------------------------------------------------------------
bool GeomToTriangle::findBasedCurve(const X_monotone_curve_2& curve, int32& curve_id, bool& direction)
{
    bool flag_exist = false;
    for (auto i = 0; i < based_curves_.size(); ++i) {
        const X_monotone_curve_2& my_curve = based_curves_[i].curve;

        bool match_same = ((my_curve.source() == curve.source()) && (my_curve.target() == curve.target()));
        bool match_revese = ((my_curve.source() == curve.target()) && (my_curve.target() == curve.source()));

        if (match_same || match_revese) {
            if (curve.is_circular() && my_curve.is_circular()) {
                bool same_center = (curve.supporting_circle().center() == my_curve.supporting_circle().center());
                bool same_direction = (curve.orientation() == my_curve.orientation());
                if (same_center && match_same && same_direction) {
                    curve_id = i;
                    direction = true;    // 동일방향
                    flag_exist = true;
                    break;
                }
                if (same_center && !match_same && !same_direction) {
                    curve_id = i;
                    direction = false;   // 역방향
                    flag_exist = true;
                    break;
                }
            }
            if (curve.is_linear() && my_curve.is_linear()) {
                if (match_same) {
                    direction = true;
                }
                else {
                    direction = false;
                }
                curve_id = i;
                flag_exist = true;
                break;
            }
        }
    }

    return flag_exist;
}

//----------------------------------------------------------------------------
void GeomToTriangle::generateTriangleData()
{
    // 절점, 세그먼트 생성
    for (auto curve = based_curves_.begin(); curve != based_curves_.end(); ++curve) {
        std::pair<const BCNode*, bool> res = testCurveInBCNode(curve->curve);
        int32 seg_marker = 0;
        if(res.first) seg_marker = res.first->getID();

        auto seg_curve = curve->segmented_curve;
        if (seg_curve.size() < 1) continue;
        Vert prev_vert = seg_curve.front();
        int32 prev_id = insertVertex(prev_vert, 1);
        seg_curve.pop_front();

        for (auto vert : seg_curve) {
            int32 curr_id = insertVertex(vert, 1);
            if (prev_id == curr_id) continue;

            insertSegment({ prev_id, curr_id }, seg_marker);

            prev_id = curr_id;
        }
    }

    // 영역, 홀 생성
    std::vector< Polygon_2> holes;

    // final_polyholes을 기반으로 Arrangement 에 모든 커브를 추가하고
    // 새로운 폴리곤을 생성해 나감
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it)
    {
        Polygon_with_holes_2& polyholes = it->second.segmented_polyholes;
        int32 region_id = it->second.region_id;
        int32 id = it->first;

        // 영역 정보 만들기
        Vert on_vertx;
        if (getArbitraryPointInPolyHoles(polyholes, on_vertx)) {
            regions_[id].point = on_vertx;
            regions_[id].attribute = region_id;
            regions_[id].max_area = it->second.area / it->second.required_number_of_elements;
        }

        // 홀 정보 만들기
        for (auto hit = polyholes.holes_begin(); hit != polyholes.holes_end(); ++hit)
        {
            // CCW로 방향 바꾸어 저장
            Polygon_2 inner = *hit;
            inner.reverse_orientation();
            holes.push_back(inner);
        }
    }

    // 만들어진 홀은 다른 Polygon으로 채워져 있을 수도 있다
    // 진정한 홀은 홀에서 다른 도메인을 모두 빼도 남아 있는 것이다
    for (auto h = holes.begin(); h != holes.end(); ) {
        Polygon_set_2 hole(*h);
        bool erase = false;
        for (auto polyhole = polyholes_.begin(); polyhole != polyholes_.end(); ++polyhole) {
            Polygon_set_2& domain = polyhole->second.domain;
            hole.difference(domain);
            if (hole.is_empty()) {
                erase = true;
                break;
            }
        }
        if (erase) {
            h = holes.erase(h);
        }
        else {
            ++h;
        }
    }

    // 진정한 홀에 대해서 홀 내부 임의 점을 만들어 낸다
    for (auto hole = holes.begin(); hole != holes.end(); hole++) {
        Vert on_vertx;
        if (getArbitraryPointInPolyHoles(Polygon_with_holes_2(*hole), on_vertx)) {
            holes_.push_back(on_vertx);
        }
    }
}

//----------------------------------------------------------------------------
// 해당 Polyholes을 이용해 요소 생성한 후, 임의의 내부 요소의 중심점 리턴
// 매우 매우 쓸데없는 일을 많이 하는 것 같은 느낌임
bool GeomToTriangle::getArbitraryPointInPolyHoles(const Polygon_with_holes_2& polyholes, Vert& point)
{
    // 폴리홀을 Constrained_Delaunay_triangulation 을 통해
    // 요소를 생성 한 후, 임의의 삼각형의 중심점을 리턴하기로 함
    // 이때 삼각형의 중심이 폴리곤의 내부에 있지 않을 경우, 내부에 있는 다른 점을 찾을때 까지
    // 반복하여 특정 포인트를 찾아냄

    CDT cdt;
    if (triangulatePolyholes(cdt, polyholes))
    {
        for (auto face_it = cdt.finite_faces_begin(); face_it != cdt.finite_faces_end(); ++face_it) {
            if (face_it->is_in_domain()) {
                Vertex_handle v1 = face_it->vertex(0);
                Vertex_handle v2 = face_it->vertex(1);
                Vertex_handle v3 = face_it->vertex(2);
                
                Vert vv1{ CGAL::to_double(v1->point().x()) ,CGAL::to_double(v1->point().y()) };
                Vert vv2{ CGAL::to_double(v2->point().x()) ,CGAL::to_double(v2->point().y()) };
                Vert vv3{ CGAL::to_double(v3->point().x()) ,CGAL::to_double(v3->point().y()) };

                point = Vert{ (vv1[0] + vv2[0] + vv3[0]) / 3.0, (vv1[1] + vv2[1] + vv3[1]) / 3.0 };
                return true;
            }
        }
    }

    return false;
}

//----------------------------------------------------------------------------
bool GeomToTriangle::triangulatePolyholes(CDT& cdt, const Polygon_with_holes_2& polyholes)
{
    //if (polyholes.is_empty() || polyholes.is_unbounded())
    if (polyholes.is_unbounded())
        return false;

    // 1. 외부 경계 처리
    const Polygon_2& outer_boundary = polyholes.outer_boundary();
    for (auto it = outer_boundary.curves_begin(); it != outer_boundary.curves_end(); ++it) {
        const X_monotone_curve_2& curve = *it;
        try {
            Point_2 source(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
            Point_2 target(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
            // 직선 세그먼트로 CDT에 삽입
            cdt.insert_constraint(source, target);
        }
        catch (const std::exception& e) {
            // 예외가 발생하면 처리
            spdlog::error("Error inserting constraint for outer boundary: {}", e.what());
            return false;
        }
    }

    // 2. 홀 경계 처리
    for (auto hole_it = polyholes.holes_begin(); hole_it != polyholes.holes_end(); ++hole_it) {
        const Polygon_2& hole = *hole_it;
        for (auto it = hole.curves_begin(); it != hole.curves_end(); ++it) {
            const X_monotone_curve_2& curve = *it;
            try {
                Point_2 source(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
                Point_2 target(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
                // 직선 세그먼트로 CDT에 삽입
                cdt.insert_constraint(source, target);
            }
            catch (const std::exception& e) {
                // 예외가 발생하면 처리
                spdlog::error("Error inserting constraint for hole boundary: {}", e.what());
                return false;
            }
        }
    }

    // 내부와 외부를 마킹 (요소생성을 위해)
    CGAL::mark_domain_in_triangulation(cdt);

    return true;
}
/*
void GeomToTriangle::generateGmshData()
{
    // 서브도메인 만들기(refined_polyholes 로 작업한다)
    for (auto it = polyholes_.begin(); it != polyholes_.end(); ++it) {
        int32 unique_polyholes_id = it->first;

        int32 region_id = it->second.region_id;
        Polygon_with_holes_2& polyholes = it->second.refined_polyholes;
        
        // 외곽경계
        std::vector<int> outer_curve_ids;
        const Polygon_2& outer = polyholes.outer_boundary();
        generateGmshSurface(outer, outer_curve_ids, it->second.based_length);
        removeRedundancyGCurves(outer_curve_ids);
        if (outer_curve_ids.size() < 2) {
            continue;
        }
        if ((outer_curve_ids.size() == 2) && (outer_curve_ids[0] == -outer_curve_ids[1])) {
            continue;
        }

        GLoop outer_loop(gmsh_entity_id_++, outer_curve_ids);
        int outer_loop_id = insertGLoop(outer_loop);

        // 내부 홀경계
        std::vector<int> inner_loop_ids;
        for (auto hit = polyholes.holes_begin(); hit != polyholes.holes_end(); ++hit)
        {
            std::vector<int> inner_curve_ids;
            const Polygon_2& inner = *hit;
            generateGmshSurface(inner, inner_curve_ids, it->second.based_length);
            removeRedundancyGCurves(inner_curve_ids);
            if (inner_curve_ids.size() < 2) {
                continue;
            }
            if ((inner_curve_ids.size() == 2) && (inner_curve_ids[0] == -inner_curve_ids[1])) {
                continue;
            }
            GLoop inner_loop(gmsh_entity_id_++, inner_curve_ids);
            int inner_loop_id = insertGLoop(inner_loop);
            inner_loop_ids.push_back(inner_loop_id);
        }

        GSurface surface(gmsh_entity_id_++, outer_loop_id, inner_loop_ids);
        gmsh_surfaces_[region_id].push_back(surface);
    }
}
*/

void GeomToTriangle::generateGmshSurface(const Polygon_2& polygon, std::vector<int>& surface_ids, int domain_id)
{
    std::vector<int> domain_point_ids;
    for (auto curve = polygon.curves_begin(); curve != polygon.curves_end(); ++curve) {
        const X_monotone_curve_2& in_curve = (*curve);

        std::pair<BCNode*, bool> res = testCurveInBCNode(in_curve);
        if (in_curve.is_circular()) {
            const Traits_2::Point_2& source = in_curve.source();
            const Traits_2::Point_2& target = in_curve.target();
            const Point_2& center = in_curve.supporting_circle().center();

            const double radii = sqrt(CGAL::to_double(in_curve.supporting_circle().squared_radius()));
            Vert cc{ CGAL::to_double(center.x()), CGAL::to_double(center.y()) };
            Vert ss{ CGAL::to_double(source.x()), CGAL::to_double(source.y()) };
            Vert tt{ CGAL::to_double(target.x()), CGAL::to_double(target.y()) };

            // 시계방향이면 시작점과 끝점을 바꾼다(Gmsh는 항상 반시계 방향 기준임)
            if (curve->orientation() == CGAL::CLOCKWISE) {
                swap(ss, tt);
            }

            GPoint gss(gmsh_entity_id_++, ss[0], ss[1], 0);
            GPoint gtt(gmsh_entity_id_++, tt[0], tt[1], 0);
            GPoint gcc(gmsh_entity_id_++, cc[0], cc[1], 0);

            std::vector<int> curve_ids;
            int ss_id = insertGPoint(gss);
            int cc_id = insertGPoint(gcc);
            int tt_id = insertGPoint(gtt);
            curve_ids.push_back(ss_id);
            curve_ids.push_back(cc_id);
            curve_ids.push_back(tt_id);

            // 커브의 시작점과 끝점이 같다면 쓰래기 커브다
            if (curve_ids[0] == curve_ids[2]) {
                continue;
            }

            GCurve new_curve(gmsh_entity_id_++, GCurve::CurveType::Circle, curve_ids);
            int32 curve_id = insertGCurve(new_curve);
            if (curve->orientation() == CGAL::CLOCKWISE) {
                curve_id = -curve_id;
            }
            surface_ids.push_back(curve_id);

            domain_point_ids.push_back(ss_id);
            domain_point_ids.push_back(tt_id);
            
            // 경계 오브젝트 처리
            if (res.first) {
                auto& vec = gmsh_bc_points_[res.first];
                if (std::find(vec.begin(), vec.end(), ss_id) == vec.end()) {
                    vec.push_back(ss_id);
                }
                if (std::find(vec.begin(), vec.end(), tt_id) == vec.end()) {
                    vec.push_back(tt_id);
                }
                if (res.second) {
                    gmsh_bc_curves_[res.first].push_back(curve_id);
                }
                else {
                    gmsh_bc_curves_[res.first].push_back(-curve_id);
                }
            }
        }
        else {
            const Traits_2::Point_2& source = in_curve.source();
            const Traits_2::Point_2& target = in_curve.target();

            Vert ss{ CGAL::to_double(source.x()), CGAL::to_double(source.y()) };
            Vert tt{ CGAL::to_double(target.x()), CGAL::to_double(target.y()) };

            GPoint gss(gmsh_entity_id_++, ss[0], ss[1], 0);
            GPoint gtt(gmsh_entity_id_++, tt[0], tt[1], 0);

            std::vector<int> curve_ids;
            int ss_id = insertGPoint(gss);
            int tt_id = insertGPoint(gtt);
            curve_ids.push_back(ss_id);
            curve_ids.push_back(tt_id);

            // 커브의 시작점과 끝점이 같다면 쓰래기 커브다
            if (curve_ids[0] == curve_ids[1]) {
                continue;
            }

            GCurve new_curve(gmsh_entity_id_++, GCurve::CurveType::Line, curve_ids);
            int32 curve_id = insertGCurve(new_curve);
            surface_ids.push_back(curve_id);

            domain_point_ids.push_back(ss_id);
            domain_point_ids.push_back(tt_id);

            // 경계 오브젝트 처리
            if (res.first) {
                auto& vec = gmsh_bc_points_[res.first];
                if (std::find(vec.begin(), vec.end(), ss_id) == vec.end()) {
                    vec.push_back(ss_id);
                }
                if (std::find(vec.begin(), vec.end(), tt_id) == vec.end()) {
                    vec.push_back(tt_id);
                }
                if (res.second) {
                    gmsh_bc_curves_[res.first].push_back( curve_id);
                }
                else {
                    gmsh_bc_curves_[res.first].push_back(-curve_id);
                }
            }
        }
    }

    // 쓸데없는 커브는 커브리스트에서 제거함
    removeRedundancyGCurves(surface_ids);

    // 절점과 연결된 유니크 도메인 id 저장 --> 메시 싸이즈 정하기 위함
    for (auto point_id : domain_point_ids) {
        auto& vec = gmsh_related_domains_[point_id];
        if (std::find(vec.begin(), vec.end(), domain_id) == vec.end()) {
            // vec 내에 domain_id가 없다면 push_back
            vec.push_back(domain_id);
        }
    }
}

int32 GeomToTriangle::insertGPoint(const GPoint& new_point)
{
    for (const auto& [id, point] : gmsh_points_) {
        float64 dx = point.x - new_point.x;
        float64 dy = point.y - new_point.y;
        float64 delta = std::sqrt(dx * dx + dy * dy);
        if (delta < tol_) {
            return point.id;
        }
    }

    gmsh_points_.emplace(new_point.id, new_point);
    return new_point.id;
}

int32 GeomToTriangle::insertGCurve(const GCurve& new_curve)
{
    for (const auto& [id, curve] : gmsh_curves_) {

        // 동일한 커브 존재하면 해당 인덱스 리턴
        if (new_curve == curve) return id;

        // 커브 타입이 Line이면 방향이 반대인 커브가 존재할 수 있다
        if (new_curve.getTypeName() == "Line") {
            GCurve reverse_curve = curve;
            reverse_curve.reverse();
            if (new_curve == reverse_curve) {
                return -id;
            }
        }
    }

    // 동일 커브 존재하지 않으면 새롭게 추가후 인덱스 리턴
    gmsh_curves_.emplace(new_curve.id, new_curve);
    return new_curve.id;
}

int32 GeomToTriangle::insertGLoop(const GLoop& new_loop)
{
    for (const auto& [id, loop] : gmsh_loops_) {
        // 동일한 루프 존재하면 해당 인덱스 리턴
        if (new_loop == loop) return id;

        // 동일한 루프 존재하는데, 방향이 반대면, -붙인 인덱스 리턴
        GLoop reverse_loop = loop;
        reverse_loop.reverse();
        if (new_loop == reverse_loop) return -id;
    }

    // 동일 루프 존재하지 않으면 새롭게 추가후 인덱스 리턴
    gmsh_loops_.emplace(new_loop.id, new_loop);
    return new_loop.id;
}

void GeomToTriangle::removeRedundancyGCurves(std::vector<int>& curve_ids)
{
    std::vector<int> remove_ids;
    int i = 0, j = 1;
    while (curve_ids.size() > j) {
        auto curve1_id = curve_ids[i];
        auto curve2_id = curve_ids[j];
        if (curve1_id == -curve2_id) {
            remove_ids.push_back(curve1_id);
            remove_ids.push_back(curve2_id);

            if (i == 0) {
                i = j + 1;
                j = j + 2;
            }
            else {
                i = i - 1;
                j = j + 1;
            }
        }
        else {
            i = j;
            j = j + 1;
        }
    }

    // remove_ids의 원소들을 빠르게 조회하기 위해 unordered_set에 저장
    std::unordered_set<int> remove_ids_set(remove_ids.begin(), remove_ids.end());

    // curve_ids에서 remove_ids_set에 포함된 원소들을 제거
    curve_ids.erase(std::remove_if(curve_ids.begin(), curve_ids.end(), [&remove_ids_set](int value) {
        return remove_ids_set.find(value) != remove_ids_set.end(); // remove_ids_set에 존재하면 제거
    }), curve_ids.end());
}

void GeomToTriangle::calculateGmshSizeAtPoints()
{
    for (auto& [id, gpoint] : gmsh_points_) {
        std::vector<int>& related_regions = gmsh_related_domains_[id];

        // 중복원소 제거
        std::sort(related_regions.begin(), related_regions.end());
        auto last = std::unique(related_regions.begin(), related_regions.end());
        related_regions.erase(last, related_regions.end());

        float64 mesh_size = 0;
        int related_count = 0;
        for (auto domain_id : related_regions) {
            if (domain_id != -1 && polyholes_[domain_id].based_length > 0) {
                mesh_size += polyholes_[domain_id].based_length;
                related_count++;
            }
        }
        
        gpoint.size = mesh_size / related_count;
    }
}

void GeomToTriangle::adjustPointsOnPeriodicBC()
{
    for (auto bc : bc_nodes_) {
        SlavePeriodicBCNode* slave = dynamic_cast<SlavePeriodicBCNode*>(bc);
        if (slave && slave->checkValid()) {
            MasterPeriodicBCNode* master = slave->getPair();

            const auto& master_point_ids = gmsh_bc_points_[master];
            const auto& slave_point_ids = gmsh_bc_points_[slave];

            if (slave->isCircular()) {
                float64 angle = slave->getCircularCoefficient();
                for (auto m_id : master_point_ids) {
                    auto& pt1 = gmsh_points_.at(m_id);
                    Vector2 v1(pt1.x, pt1.y);
                    for (auto s_id : slave_point_ids) {
                        auto& pt2 = gmsh_points_.at(s_id);
                        Vector2 v2(pt2.x, pt2.y);
                        if (abs(v1.length() - v2.length()) < tol_) {
                            pt2.size = pt1.size;
                            break;
                        }
                    }
                }
            }
        }
    }

    // 무빙밴드에 포함된 절점은 공극사이즈의 절반으로 정함
    if (mb_) {
        const auto& mb_point_ids = gmsh_bc_points_[mb_];
        for (auto m_id : mb_point_ids) {
            auto& pt = gmsh_points_.at(m_id);
            pt.size = mb_->getAirgapLength()/2;
        }

        const auto& mb_inner_ids = gmsh_bc_points_[mb_->getInnerBCNode()];
        for (auto m_id : mb_inner_ids) {
            auto& pt = gmsh_points_.at(m_id);
            pt.size = mb_->getAirgapLength() / 2;
        }
        const auto& mb_outer_ids = gmsh_bc_points_[mb_->getOuterBCNode()];
        for (auto m_id : mb_outer_ids) {
            auto& pt = gmsh_points_.at(m_id);
            pt.size = mb_->getAirgapLength() / 2;
        }
    }
}

void GeomToTriangle::writePolyFile(const String& filename) const {
    if (!done_configure_) {
        spdlog::error("The polygon structure is not configured properly!", filename.c_str());
        return;
    }

    std::ofstream poly_file(filename.c_str());
    if (!poly_file.is_open()) {
        spdlog::error("Failed to open file: {}", filename.c_str());
        return;
    }

    // 1. Write vertices
    size_t num_vertices = vertices_.size();
    poly_file << num_vertices << " 2 0 1\n";
    size_t index = 0;
    for (auto it = vertices_.begin(); it != vertices_.end(); ++it, ++index) {
        poly_file << index + 1 << " " << std::fixed << std::setprecision(6)
            << (*it)[0] << " " << (*it)[1] << " "
            << (index < vertices_maker_.size() ? vertices_maker_[index] : 0) << "\n";
    }

    // 2. Write segments
    size_t num_segments = segments_.size();
    poly_file << num_segments << " 1\n";
    index = 0;
    for (auto it = segments_.begin(); it != segments_.end(); ++it, ++index) {
        poly_file << index + 1 << " " << (*it)[0] << " " << (*it)[1] << " "
            << (index < segment_makers_.size() ? segment_makers_[index] : 0) << "\n";
    }

    // 3. Write holes
    size_t num_holes = holes_.size();
    poly_file << num_holes << "\n";
    index = 0;
    for (auto it = holes_.begin(); it != holes_.end(); ++it, ++index) {
        poly_file << index + 1 << " " << std::fixed << std::setprecision(6)
            << (*it)[0] << " " << (*it)[1] << "\n";
    }

    // 4. Write regions
    size_t num_regions = regions_.size();
    poly_file << num_regions << "\n";
    index = 0;
    for (auto it = regions_.begin(); it != regions_.end(); ++it, ++index) {
        const auto& region = it->second;
        poly_file << index + 1 << " " << std::fixed << std::setprecision(6)
            << region.point[0] << " " << region.point[1] << " "
            << region.attribute << " " << region.max_area << "\n";
    }

    poly_file.close();
}

bool GeomToTriangle::writeGeoFile(const String& filename) {
    if (!done_configure_) {
        spdlog::error("The polygon structure is not configured properly!");
        return false;
    }

    std::ofstream geo_file(filename.c_str());
    if (!geo_file.is_open()) {
        return false;
    }

    // 메쉬 사이즈 계산
    calculateGmshSizeAtPoints();

    // 마스터 슬레이브 메쉬 사이즈 일치 조정
    adjustPointsOnPeriodicBC();

    geo_file
        << std::scientific << std::setprecision(16)
        << "// Generated by bzMagEditor\n";

    // 1. Points 출력
    geo_file << "// Points\n";
    for (const auto& [id, point] : gmsh_points_) {
        geo_file << "Point(" << point.id << ") = {"
            << point.x * 0.001 << ", " << point.y * 0.001 << ", " << point.z * 0.001
            << ", " << point.size * 0.001 << "};\n";
    }

    // 2. Curves 출력
    geo_file << "\n// Curves\n";
    for (const auto& [id,curve] : gmsh_curves_) {
        geo_file << curve.getTypeName() << "(" << curve.id << ") = {";
        for (size_t i = 0; i < curve.point_ids.size(); ++i) {
            geo_file << curve.point_ids[i];
            if (i != curve.point_ids.size() - 1) geo_file << ", ";
        }
        geo_file << "};\n";
    }

    // 3. Loops 출력
    geo_file << "\n// Loops\n";
    for (const auto& [id,loop] : gmsh_loops_) {
        geo_file << "Line Loop(" << loop.id << ") = {";
        for (size_t i = 0; i < loop.curve_ids.size(); ++i) {
            geo_file << loop.curve_ids[i];
            if (i != loop.curve_ids.size() - 1) geo_file << ", ";
        }
        geo_file << "};\n";
    }

    // 4. Surfaces 출력
    geo_file << "\n// Surfaces\n";
    for (const auto& [key, surfaces] : gmsh_surfaces_) {
        for (const auto& surface : surfaces) {
            geo_file << "Plane Surface(" << surface.id << ") = {"
                << surface.outer_loop_id;
            if (!surface.inner_loop_ids.empty()) {
                geo_file << ", ";
                for (size_t i = 0; i < surface.inner_loop_ids.size(); ++i) {
                    geo_file << surface.inner_loop_ids[i];
                    if (i != surface.inner_loop_ids.size() - 1) geo_file << ", ";
                }
                //geo_file << "}";
            }
            geo_file << "};\n";
        }
    }

    // 5.1 Physical Surface영역 출력
    geo_file << "\n// Physical Surface\n";
    for (const auto& [key, surfaces] : gmsh_surfaces_) {
        int32 region_id = key;
        auto it = domain_info_.find(region_id);
        if (it != domain_info_.end()) {
            String name = (*it).second.name;
            geo_file << "Physical Surface(\"" << name.c_str() << "\", " << region_id * 100 << ") = {";
            for (size_t i = 0; i < surfaces.size(); ++i) {
                geo_file  << surfaces[i].id;
                if (i != surfaces.size() - 1) geo_file << ", ";
            }
            geo_file << "};\n";
        }
    }
    // 5.2 Physical Line영역(경계) 출력
    geo_file << "\n// Physical Line\n";
    for (const auto& [bc, region_ids] : gmsh_bc_curves_) {
        if (bc == mb_) continue;

        String name = bc->getName();
        if(mb_) {
            if (bc == mb_->getInnerBCNode() || bc == mb_->getOuterBCNode()) {
                std::string mb_name(mb_->getName());
                name = mb_name + "_" + name;
            }
        }

        geo_file << "Physical Line(\"" << name.c_str() << "\", " << bc->getID() * 100 << ") = {";
        for (size_t i = 0; i < region_ids.size(); ++i) {
            geo_file << region_ids[i];
            if (i != region_ids.size() - 1) geo_file << ", ";
        }
        geo_file << "};\n";
    }
    // 5.3 Physical Point영역(경계) 출력
    /*
    geo_file << "\n// Physical Point\n";
    for (const auto& [bc, region_ids] : gmsh_bc_points_) {
        String name = bc->getName();
        geo_file << "Physical Point(\"" << name.c_str() << "_Point\", " << bc->getID() * 100 + 10 << ") = {";
        for (size_t i = 0; i < region_ids.size(); ++i) {
            geo_file << region_ids[i];
            if (i != region_ids.size() - 1) geo_file << ", ";
        }
        geo_file << "};\n";
    }
    */

    //5.4 무빙밴드 경계 복사
    geo_file << std::scientific << std::setprecision(16);
    if (mb_) {
        if (mb_->isCircular()) {
            float64 angle = mb_->getCircularCoefficient();
            int symmetry_factor = (int)round((2 * CGAL_PI) / angle);

            geo_file << "\n// Filling the gap for the whole 2*Pi\n";
            // Outer
            {
                BCNode* bc = mb_->getOuterBCNode();
                auto region_ids = gmsh_bc_curves_[bc];
                std::string line_name(bc->getName());
                std::string name(mb_->getName());
                std::string mb_name = name + "_" + line_name;

                geo_file
                    << mb_name << "_0[] = {";
                for (size_t i = 0; i < region_ids.size(); ++i) {
                    geo_file << region_ids[i];
                    if (i != region_ids.size() - 1) geo_file << ", ";
                }
                geo_file
                    << "};\n";
                geo_file
                    << mb_name << "[] += " << mb_name << "_0[];\n";

                for (auto i = 1; i < symmetry_factor; i++) {
                    std::string dup_name = mb_name + "_" + std::to_string(i);
                    geo_file
                        << dup_name << "[] = Rotate {{0, 0, 1}, {0, 0, 0}, " << i * angle << "} { Duplicata{Line{"
                        << mb_name << "_0[]};} };\n"
                        << mb_name << "[] += " << mb_name << "_" << i << "[];\n";

                    geo_file
                        << "Physical Line(\"" << dup_name << "\", " << bc->getID() * 100 + i << ") = " << dup_name << "[];\n";

                }
                //geo_file
                //    << "Transfinite Line{" << mb_name << "[]} = 180;\n";
            }
            // Inner
            {
                BCNode* bc = mb_->getInnerBCNode();
                auto region_ids = gmsh_bc_curves_[bc];
                std::string line_name(bc->getName());
                std::string name(mb_->getName());
                std::string mb_name = name + "_" + line_name;

                geo_file
                    << mb_name << "_0[] = {";
                for (size_t i = 0; i < region_ids.size(); ++i) {
                    geo_file << region_ids[i];
                    if (i != region_ids.size() - 1) geo_file << ", ";
                }
                geo_file
                    << "};\n";
                geo_file
                    << mb_name << "[] += " << mb_name << "_0[];\n";

                for (auto i = 1; i < symmetry_factor; i++) {
                    std::string dup_name = mb_name + "_" + std::to_string(i);
                    geo_file
                        << dup_name << "[] = Rotate {{0, 0, 1}, {0, 0, 0}, " << i * angle << "} { Duplicata{Line{"
                        << mb_name << "_0[]};} };\n"
                        << mb_name << "[] += " << mb_name << "_" << i << "[];\n";

                    geo_file
                        << "Physical Line(\"" << dup_name << "\", " << bc->getID() * 100 + i << ") = " << dup_name << "[];\n";

                }
                //geo_file
                //    << "Transfinite Line{" << mb_name << "[]} = 180;\n";
            }
        }
        // 선형기인 경우 아직 구현 안함
        else {

        }

    }

    geo_file.close();
    return true;
}


//----------------------------------------------------------------------------
void GeomToTriangle::bindProperty()
{
    BIND_PROPERTY(int32, BasedNumberOfElements, &setTotalRequiredNumberOfElements, &getTotalRequiredNumberOfElements);
    BIND_PROPERTY(float64, BasedAngle, &setBasedAngleForSegmentation, &getBasedAngleForSegmentation);
}
