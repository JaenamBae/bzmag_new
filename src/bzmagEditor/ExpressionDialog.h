#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include "ExpressionTableModel.h"

class ExpressionDialog : public QDialog {
    Q_OBJECT

public:
    ExpressionDialog(ExpressionTableModel* model, QWidget* parent = nullptr);
    void setReadOnly(bool readonly); // 읽기 전용 설정 함수 추가

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void addExpression();
    void deleteSelectedExpression();
    void onDataModified(bool modified); // 데이터 변경 시 호출
    void applyChanges();

private:
    QTableView* table_view_;
    ExpressionTableModel* table_model_;

    QPushButton* add_button_;       // 버튼 멤버 변수로 변경
    QPushButton* delete_button_;    // 버튼 멤버 변수로 변경
    QPushButton* apply_button_;     // 버튼 멤버 변수로 변경
};
