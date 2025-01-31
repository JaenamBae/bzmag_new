#ifndef BZMAG_ENGINE_GEOMBASENODE_H
#define BZMAG_ENGINE_GEOMBASENODE_H

/*
Description : Abstract Node for Geometry Related to
Last Update : 2020.08.03
- GeomCSNode를 Ref<>로 구현함 (메모리 해제시의 문제점 해결 위해)
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "core/node.h"
#include <vector>
#include <mutex>

namespace bzmag
{
    namespace engine
    {
        class GeomHeadNode;
        //class CSNode;
        class ENGINELIBRARY_API GeomBaseNode : public Node
        {
            friend class GeomHeadNode;

        public:
            GeomBaseNode();
            virtual ~GeomBaseNode();
            DECLARE_ABSTRACTCLASS(GeomBaseNode, Node);

            typedef std::pair<GeomHeadNode*, Transformation> LinkedHead;
            typedef std::vector<LinkedHead> LinkedHeads;
            typedef std::vector<X_monotone_curve_2> Curves;
            typedef std::vector<Traits_2::Point_2> Vertices;

        public:
            // 참조하는 CS
            //void setReferedCS(CSNode* cs);
            //CSNode* getReferedCS() const;

            // 해드노드 얻기
            GeomHeadNode* getHeadNode();

            // 폴리셋 만들기
            bool makeHistory(std::list<GeomBaseNode*>& history, GeomHeadNode* head);

            // 폴리셋, 커브, 점 얻기
            const Polygon_set_2& getPolyset();
            const Curves& getCurves();
            const Vertices& getVertices();

            // hit 테스트
            bool hitTest(float64 x, float64 y);
            
        public:
            // 아래 두 함수는 해드노드에서 재정의 되어야 함
            virtual bool isCovered() const; 
            virtual bool makeGeometry(Transformation trans = Transformation());

            // 좌표변환을 일으키는 노드에서 재정의 필요함; Primitive(Coordination) Move, Rotate
            virtual Transformation getMyTransform();
            virtual bool update();
            virtual void onAttachTo(Node* parent);
            virtual void onDetachFrom(Node* parent);

            // 각 노드별 재정의 필요할 수 있음
            virtual void updateCovered();       // Head, CoverLine, CloneFrom
            virtual void updateTransform();     // Head, Primitive(?)
            virtual void updateLinkedNode();    // Head, Primitive, Boolean

            // 순수 가상함수
            virtual String description() const = 0;

            // 주어진 transform을 이용해 polyset 혹은 curve 혹은 vertices를 생성하는 함수
            // primitive node / boolean node / spilt node 에서 실질적인 일을 하게 됨
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) = 0;

        protected:
            virtual void clearBelongings() override;
            
        protected:
            void setupHead(GeomBaseNode* parent);
            void indexingGeometry(const Polygon_set_2& geometry, Curves& curves, Vertices& vertices) const;
            void indexingPolygon(const Polygon_2& poly, Curves& curves, Vertices& vertices) const;

        public:
            static void bindMethod();
            static void bindProperty();

        protected:
            // 하기 4항목은 부모의 속성을 일단 물려받고, 현재 노드의 속성으로 업데이트 한다
            GeomHeadNode* head_;        // 참조 헤드노드
            bool be_covered_;             // 오브젝트가 되었는지? (Boundary Object --> Surface Object)

            Transformation last_trans_; // 현재 Head 트리상에서 Geometry 생성에 적용시킬 변환식
            LinkedHeads linked_heads_;  // 연관된 헤드노드들(Boolean 연산에 의해서)

            // 형상정보 ; makeGeometry() 에 의해서 생성될 것이다
            Polygon_set_2 geometry_;
            Curves curves_;
            Vertices vertices_;

            // 형상이 만들어 졌는지?
            bool be_geometry_;

            // 현재 부모로부터 Detach중인지(아직 Detach전임)
            bool be_detaching_;

            std::mutex mtx_;
        };
    }
}

#endif //BZMAG_ENGINE_GEOMBASENODE_H