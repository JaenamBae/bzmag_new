#include "TreeModel.h"
#include <QModelIndex>


using namespace bzmag;
using namespace bzmag::engine;

TreeModel::TreeModel(Node* root_rode, QObject* parent)
    : QAbstractItemModel(parent), root_node_(root_rode)
{
    
}

TreeModel::~TreeModel() 
{
    
}

// 행 수 반환
int TreeModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return (int)root_node_->getNumChildren();
    }

    Node* parent_node = static_cast<Node*>(parent.internalPointer());
    return parent_node ? (int)parent_node->getNumChildren() : 0; // 자식 수 반환
}

// 열 수 반환 (1열로 고정)
int TreeModel::columnCount(const QModelIndex& parent) const
{
    return 1; // 단일 열
}

// 헤더 데이터를 설정하는 부분 (수평 및 수직)
QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0: return QString("bzMag Tree");
            default: return QVariant();
            }
        }
        else if (orientation == Qt::Vertical) {
            return QString("Row %1").arg(section + 1);  // 행 헤더 설정
        }
    }
    return QVariant();
}

// 모델 데이터 반환
QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node* node = static_cast<Node*>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        QString name = QString::fromStdString(node->getName().c_str());
        if (node == selected_cs_node_ ||
            node == selected_template_node_)
            return QString("[*] %1").arg(name);
        return name;
    }

    return QVariant();
}

// 인덱스 생성
QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0) return QModelIndex();

    // 부모 노드 결정
    Node* parent_node = parent.isValid() ? static_cast<Node*>(parent.internalPointer()) : root_node_;
    if (!parent_node) return QModelIndex();

    // 자식 리스트의 크기를 확인
    int child_count = (int)parent_node->getNumChildren();
    if (row < 0 || row >= child_count) {
        return QModelIndex(); // row가 유효 범위를 벗어나면 빈 인덱스 반환
    }

    // 반복자를 이동하고 유효한 노드 가져오기
    auto it = parent_node->firstChildNode();
    std::advance(it, row);

    Node* node = it->get<Node*>();
    if (!node) {
        return QModelIndex(); // 유효하지 않은 노드라면 빈 인덱스 반환
    }

    return createIndex(row, column, node);
}

// 부모 인덱스 반환
QModelIndex TreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QModelIndex(); // 유효하지 않은 인덱스는 루트 반환
    }

    Node* child_node = static_cast<Node*>(index.internalPointer());
    if (!child_node || !child_node->getParent()) {
        return QModelIndex(); // 삭제된 노드나 부모가 없으면 루트 반환
    }

    Node* parent_node = child_node->getParent();
    if (!parent_node) {
        return createIndex(0,0, root_node_); // 부모 노드가 없으면 루트 반환
    }

    Node* grandparent_node = parent_node->getParent();
    int row = 0;
    if (grandparent_node) {
        auto it = grandparent_node->firstChildNode();
        for (; it != grandparent_node->lastChildNode(); ++it, ++row) {
            if (it->get<Node*>() == parent_node) {
                break;
            }
        }
    }

    return createIndex(row, 0, parent_node);
}

QModelIndex TreeModel::findItemIndexById(const QModelIndex& parent, int id)
{
    for (int row = 0; row < rowCount(parent); ++row) {
        QModelIndex idx = index(row, 0, parent);
        if (idx.isValid()) {
            Node* node = static_cast<Node*>(idx.internalPointer());
            if (node && node->getID() == id) {
                return idx; // 원하는 ID를 찾았을 때 해당 인덱스 반환
            }
            // 재귀적으로 자식 노드를 탐색
            QModelIndex childIndex = findItemIndexById(idx, id);
            if (childIndex.isValid()) {
                return childIndex; // 자식에서 원하는 ID를 찾은 경우 반환
            }
        }
    }
    return QModelIndex(); // 원하는 ID를 찾지 못한 경우
}

// 선택된 항목 처리
void TreeModel::selectItem(Node* selected_node)
{
    if (!selected_node) {
        return;
    }

    // 선택된 TemplateNode 또는 CSNode 추적
    if (dynamic_cast<TemplateNode*>(selected_node)) {
        selected_template_node_ = static_cast<TemplateNode*>(selected_node);
    }
    else if (dynamic_cast<CSNode*>(selected_node)) {
        selected_cs_node_ = static_cast<CSNode*>(selected_node);
    }
    else {
        return; // 선택된 노드가 둘 중 하나가 아니면 종료
    }

    // 선택된 항목의 QModelIndex 생성
    QModelIndex selected_index = createIndex(0, 0, selected_node);

    // 선택된 항목의 이름만 갱신
    emit dataChanged(selected_index, selected_index, { Qt::DisplayRole });
}

void TreeModel::onNodeRemoved(Node* node) {
    if (!node || !node->getParent()) return;

    // 부모 노드 가져오기
    Node* parent = node->getParent();

    // 삭제할 노드의 row 계산
    int row = 0;
    for (auto it = parent->firstChildNode(); it != parent->lastChildNode(); ++it, ++row) {
        if (it->get<Node*>() == node) break;
    }

    // 삭제 작업 시작
    beginRemoveRows(createIndex(0, 0, parent), row, row);

    // 선택된 노드 참조 초기화 (뷰와 동기화 유지)
    if (node == selected_cs_node_) {
        selected_cs_node_ = nullptr;
    }
    else if (node == selected_template_node_) {
        selected_template_node_ = nullptr;
    }

    endRemoveRows();

    // 트리 구조 변경 (detach 호출은 endRemoveRows 이후로 이동)
    node->detach();
}


void TreeModel::reloadData()
{
    beginResetModel(); // 모델 갱신 시작

    selected_cs_node_ = nullptr;
    selected_template_node_ = nullptr;

    endResetModel(); // 모델 갱신 완료
}
