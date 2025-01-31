#ifndef BZMAG_ENGINE_GEOMHEADNODE_H
#define BZMAG_ENGINE_GEOMHEADNODE_H

/*
Description : Head Node for Handling a Geometry Node
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"
#include "core/color.h"

namespace bzmag
{
    namespace engine
    {
        class MaterialNode;
        class CSNode;
        class ENGINELIBRARY_API GeomHeadNode : public GeomBaseNode
        {
            friend class GeomBaseNode;

        public:
            GeomHeadNode();
            virtual ~GeomHeadNode();
            DECLARE_CLASS(GeomHeadNode, GeomBaseNode);

        public:
            // 컬러설정
            void setColor(const Color& color);
            const Color& getColor() const;

            // Visualization을 하지 않을 것인지?
            void setHideStatus(bool hide);
            bool isHide() const;

            // Boolean 연산에 의해 참조되는 않는 노드인지?
            bool isStandAlone() const;

            // Z-Order 설정용
            bool contain(GeomHeadNode* node);

            // 해석에 쓰일 모델 노드인지?
            void setModelNode(bool model);
            bool isModelNode() const;

            // 요소생성 갯수 (원하는 값)
            void setNumberOfElements(int32 ne);
            int32 getNumberOfElements() const;

            // 재질 설정 
            void setMaterialNode(MaterialNode* material);
            MaterialNode* getMaterialNode() const;

            // 참조하는 CS
            void setReferedCS(CSNode* cs);
            CSNode* getReferedCS() const;

            // Head노드가 참조하는 가장 말단 자식 노드
            GeomBaseNode* getLastNode() const;
            
            // 현재 노드 이하 자식 노드들 강제 업데이트
            void forcedUpdate();

        protected:
            void setStandAlone(bool standalone);

            // Head노드가 참조하는 가장 말단 자식 노드
            void setLastNode(GeomBaseNode* last);


        public:
            virtual bool isCovered() const override;
            virtual bool makeGeometry(Transformation trans = Transformation()) override;
            virtual String description() const override;
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;
            virtual void updateCovered() override;
            virtual void updateTransform() override;
            virtual void updateLinkedNode() override;

        public:
            static void bindMethod();
            static void bindProperty();

        protected:
            bool standalone_;
            bool model_node_;
            bool hide_;

            Color color_;
            int32 num_elements_;

            // 최종 연산된 Geometry 노드
            GeomBaseNode* last_node_;

            // 참조 재질
            Ref<MaterialNode> material_;

            // 참조 좌표계
            Ref<CSNode> cs_;
        };

#include "geomheadnode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMHEADNODE_H