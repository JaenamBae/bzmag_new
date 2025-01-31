#pragma once

#include <QAbstractTableModel>
#include "engine/Expression.h"

class ExpressionTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ExpressionTableModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bzmag::engine::Expression* getExpression(int row) const;

signals:
    void expressionUpdated();
    void dataModified(bool modified); // 데이터 변경 여부 알림 신호

private:
    std::vector<bzmag::engine::Expression*> filteredExpressions() const;
};
