#ifndef BZMAG_ENGINE_DATASETNODE_H
#define BZMAG_ENGINE_DATASETNODE_H

#include "platform.h"
#include "core/node.h"
#include <vector>
#include <array>
#include "core/dataset.h"

namespace bzmag {
    namespace engine {
        class ENGINELIBRARY_API DataSetNode : public Node
        {
        public:
            DataSetNode();
            virtual ~DataSetNode();
            DECLARE_CLASS(DataSetNode, Node);

            // 데이터 전체를 가져오는 함수
            const DataSet& getDataset() const;

            // 데이터 전체를 설정하는 함수
            void setDataset(const DataSet& dataset);
            void setDataset(const String& data);

            // 특정 인덱스의 데이터를 가져오는 함수
            DataPair getData(size_t index) const;

            std::vector<float64> extractXComponents() const;
            std::vector<float64> extractYComponents() const;

            // 특정 인덱스의 데이터를 설정하는 함수
            void setData(size_t index, const DataPair& data);

            // 데이터를 추가하는 함수
            void addData(const DataPair& data);

            // 전체 데이터를 삭제하는 함수
            void clearDataset();

            // 데이터의 개수를 가져오는 함수
            size_t size() const;

        public:
            virtual bool update() override;
            virtual void onAttachTo(Node* parent) override;
            virtual void onDetachFrom(Node* parent) override;

        public:
            static void bindMethod();
            static void bindProperty();


        private:
            DataSet dataset_;
        };

    }
}


#endif //BZMAG_ENGINE_DATASETNODE_H