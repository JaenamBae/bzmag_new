#ifndef BZMAG_ENGINE_MATERIALNODE_H
#define BZMAG_ENGINE_MATERIALNODE_H

/*
Description : Material Node for Handling a Materials
Last Update : 2019.01.28
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "core/node.h"
#include "core/vector2.h"
#include "core/dataset.h"

namespace bzmag
{
    namespace engine
    {
        class Expression;
        class DataSetNode;
        class ENGINELIBRARY_API MaterialNode : public Node
        {
        public:
            MaterialNode();
            virtual ~MaterialNode();
            DECLARE_CLASS(MaterialNode, Node);

        public:
            const String& getPermeability() const;
            const String& getConductivity() const;
            const String& getMagnetization() const;
            const String& getDirectionOfMagnetization() const;
            bool isLinear() const;

            void setPermeability(const String& mu);
            void setConductivity(const String& sigma);
            void setMagnetization(const String& mag);
            void setDirectionOfMagnetization(const String& dir);
            float64 evalPermeability() const;
            float64 evalConductivity() const;
            float64 evalMagnetization() const;
            Vector2 evalDirectionOfMagnetization() const;
            bool hasEvenDirectionOfMagnetization() const;

            DataSetNode* getDataSetNode();
            void setDataSetNode(DataSetNode* node);
            const DataSet getBHdata() const;

        public:
            virtual void clearBelongings() override;
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        public:
            static void bindMethod();
            static void bindProperty();

        protected:
            Ref<Expression> conductivity_;
            Ref<Expression> permeability_;
            Ref<Expression> magnetization_;
            Ref<Expression> dir_magnetization_x_;
            Ref<Expression> dir_magnetization_y_;

            String sconductivity_;
            String spermeability_;
            String smagnetization_;
            String sdir_magnetization_;
            DataSetNode* bh_data_ = nullptr;

            // 자화의 방향이 공간에 대한 함수인지
            bool spatial_dependent_ = false;
        };

#include "materialnode.inl"

    }
}

#endif //BZMAG_ENGINE_MATERIALNODE_H