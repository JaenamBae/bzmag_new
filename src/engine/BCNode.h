#ifndef BZMAG_ENGINE_BCNODE_H
#define BZMAG_ENGINE_BCNODE_H

/*
Description : Boundary condition Node
Last Update : 2020.07.22
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "core/node.h"
#include "core/vector2.h"
#include <vector>
#include <list>

namespace bzmag
{
    namespace engine
    {
        class GeomHeadRefNode;
        class ENGINELIBRARY_API BCNode : public Node
        {
            friend class GeomHeadRefNode;

        public:
            typedef std::list<X_monotone_curve_2> Curves;
            typedef Vector2 Vert;

        public:
            BCNode();
            virtual ~BCNode();
            DECLARE_CLASS(BCNode, Node);

        public:
            // 주어진 커브가 경계 커브에 포함되는지 테스트 ; 세그멘테이션 이전의 커브로 테스트 해야 함
            int testCurve(const X_monotone_curve_2& test_curve) const;

            // 시작점과 끝점의 직선으로 구성된 세그먼트가 경계에 포함되는지 테스트
            bool testSegment(const Vert& v1, const Vert& v2) const;
            const Curves& getCurves();
            void addCurve(X_monotone_curve_2& curve);

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            // 경계(커브) 추가
            void addBoundary(GeomHeadRefNode* ref_head);
            void removeBoundary(GeomHeadRefNode* ref_head);

        public:
            static void bindMethod();
            static void bindProperty();
            static bool testVertex(const Vert& pt, const X_monotone_curve_2& test_curve);

        protected:
            // 헤더노드
            std::vector<GeomHeadRefNode*> heades_;

            // 경계조건 커브
            Curves curves_;

            // 동일점 인정 오차
            static float64 tol_;
        };

#include "BCnode.inl"

    }
}

#endif //BZMAG_ENGINE_BCNODE_H