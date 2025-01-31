#ifndef BZMAG_ENGINE_GEOMSPILTNODE_H
#define BZMAG_ENGINE_GEOMSPILTNODE_H

/*
Description : Spilt Node
Last Update : 2020.10.14
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeomBaseNode.h"

namespace bzmag
{
    namespace engine
    {
        class CSNode;
        class ENGINELIBRARY_API GeomSplitNode : public GeomBaseNode
        {
        public:
            enum SPLIT_PLANE {
                SPLIT_ZX = 0,
                SPLIT_YZ = 1,
            };

        public:
            GeomSplitNode();
            virtual ~GeomSplitNode();
            DECLARE_CLASS(GeomSplitNode, GeomBaseNode);

            // 자르는 기준면
            void setPlane(const SPLIT_PLANE& plane);
            const SPLIT_PLANE& getPlane() const;

            // 자르고 남길 면: true 이면 Positive면, flase이면 Negative면을 남김
            void setOrientation(bool o);
            bool getOrientation() const;

            // 참조하는 CS
            void setReferedCS(CSNode* cs);
            CSNode* getReferedCS() const;

        public:
            // 이하 재정의 되어야 함
            virtual String description() const override;

        protected:
            virtual void clearBelongings() override;
            virtual bool make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform) override;

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            // 어떻한 평면으로 자를것인가? (ZX 평면? YZ 평면?)
            SPLIT_PLANE plane_;

            // 자른후 어떠한 면을 남길것인가? 
            // true : 오른손법칙에 따라 Z->X인 경우 y>=0 인 영역, Y->Z인 경우 x>0= 인 영역
            // true : 오른손법칙에 따라 Z->X인 경우 y<=0 인 영역, Y->Z인 경우 x<=0 인 영역
            bool selectd_plane_;

            // 참조 좌표계
            Ref<CSNode> cs_;

            static float64 INF_;
        };

#include "geomsplitnode.inl"

    }
}

#endif //BZMAG_ENGINE_GEOMSPILTNODE_H