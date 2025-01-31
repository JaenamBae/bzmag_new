#ifndef BZMAG_ENGINE_CSNODE_H
#define BZMAG_ENGINE_CSNODE_H

/*
Description : Coordinate System Nodes
Last Update : 2017.09.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "GeometricEntity.h"
#include "core/node.h"
#include <list>


namespace bzmag
{
    namespace engine
    {
        class Expression;
        class ENGINELIBRARY_API CSNode : public Node
        {
        public:
            typedef std::list<Node*> CitingNodes;
            typedef CitingNodes::iterator CitingIter;

        public:
            CSNode();
            virtual ~CSNode();
            DECLARE_CLASS(CSNode, Node);

            bool setParameters(const String& origin, const String& angle);
            void setOrigin(const String& origin);
            void setAngle(const String& angle);

            const String& getOrigin() const;
            const String& getAngle() const;

            float64 getGlobalOriginX() const;  // 글로벌 좌표계 기준
            float64 getGlobalOriginY() const;  // 글로벌 좌표계 기준
            float64 getGlobalAngle() const;    // 글로벌 좌표계 기준

            Transformation& transformation();

            void insertReferenceNode(Node* node);
            void removeReferenceNode(Node* node);

            CitingNodes::iterator firstReferred() { return citingNodes_.begin(); }
            CitingNodes::iterator lastReferred() { return citingNodes_.end(); }

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        protected:
            virtual void clearBelongings() override;

        public:
            static void bindMethod();
            static void bindProperty();

        private:
            Ref<Expression> ox_;
            Ref<Expression> oy_;
            Ref<Expression> angle_;

            String sorigin_;
            String sangle_;

            Transformation transform_;

            // CS정보가 변경되면 CS를 참조하고 있는 Node들을 업데이트 시켜야 하기 때문에
            // CS를 참조하고 있는 리스트를 관리해야 함
            CitingNodes citingNodes_;
        };

#include "csnode.inl"

    }
}

#endif //BZMAG_ENGINE_CSNODE_H