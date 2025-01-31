#include "ExpressionTableModel.h"
#include "core/primitivetype.h"
#include "engine/ExpressionServer.h"
#include "Modeler.h"
#include "TemplateNode.h"

using namespace bzmag::engine;

ExpressionTableModel::ExpressionTableModel(QObject* parent)
    : QAbstractTableModel(parent) {

}

int ExpressionTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return (int)(filteredExpressions().size());
}

int ExpressionTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 4; // Name, Expression, Evaluated Value, Comment
}

QVariant ExpressionTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= filteredExpressions().size() || index.column() >= 4)
        return QVariant();

    const auto* expr = filteredExpressions().at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return QString::fromStdString(expr->getKey().c_str());
        case 1: return QString::fromStdString(expr->getExpression().c_str());
        case 2: {
            // const_cast를 사용하여 eval 호출
            auto* non_const_expr = const_cast<Expression*>(expr);
            return QString::number(non_const_expr->eval(), 'f', 2);
        }
        case 3: return QString::fromStdString(expr->getComment().c_str());
        }
    }
    return QVariant();
}

QVariant ExpressionTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "Name";
        case 1: return "Expression";
        case 2: return "Evaluated Value";
        case 3: return "Comment";
        }
    }
    return QVariant();
}

bool ExpressionTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    Modeler* modeler = Modeler::instance();
    if (modeler->isModelerLocked()) {
        return false;
    }

    auto* expr = filteredExpressions().at(index.row());

    switch (index.column()) {
    case 0:
        // Key 열은 수정 불가
        return false;
    case 1:
        // Expression 업데이트
        if (!expr->setExpression(value.toString().toStdString())) {
            emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
            return false;
        }
        break;
    case 3:
        // Comment 업데이트
        expr->setComment(value.toString().toStdString());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    emit dataModified(true); // 데이터가 수정되었음을 알림
    return true;
}

Qt::ItemFlags ExpressionTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 0 || index.column() == 2) {
        // Key, evaluation는 수정 불가능
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

Expression* ExpressionTableModel::getExpression(int row) const {
    if (row < 0 || row >= static_cast<int>(filteredExpressions().size()))
        return nullptr;

    return filteredExpressions().at(row);
}

std::vector<Expression*> ExpressionTableModel::filteredExpressions() const {
    std::vector<Expression*> result;

    ExpressionServer* server = ExpressionServer::instance();
    for (auto it = server->firstExpression(); it != server->lastExpression(); ++it) {
        if (it->second->isUserDefined() && !it->second->isHidden()) {
            result.push_back(it->second);
        }
    }
    return result;
}
