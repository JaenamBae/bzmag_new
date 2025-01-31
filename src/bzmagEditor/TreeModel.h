#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStandardItem>
#include <QSet>
#include <nlohmann/json.hpp>
#include "core/node.h"
#include "TemplateNode.h"
#include "engine/CSNode.h"

using json = nlohmann::ordered_json;

class TreeModel : public QAbstractItemModel 
{
    Q_OBJECT  // 시그널과 슬롯을 사용하기 위해 필요

public:
    TreeModel(bzmag::Node* root_node, QObject* parent = nullptr);
    ~TreeModel();

    // 행 수 반환
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // 열 수 반환 (1열로 고정)
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // 헤더 데이터 반환
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 모델 데이터 반환
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    // 인덱스 생성
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    // 부모 인덱스 반환
    QModelIndex parent(const QModelIndex& index) const override;

    QModelIndex findItemIndexById(const QModelIndex& parent, int id);

    void selectItem(bzmag::Node* selectedNode);

    void onNodeRemoved(bzmag::Node* removed_node);

    void reloadData();


private:
    bzmag::Node* root_node_; // 루트 노드

    bzmag::engine::CSNode* selected_cs_node_ = nullptr;
    TemplateNode* selected_template_node_ = nullptr;
};