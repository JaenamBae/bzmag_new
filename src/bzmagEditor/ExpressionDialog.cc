#include "ExpressionDialog.h"
#include "AddExpressionDialog.h"
#include <QMessageBox>
#include <QCloseEvent>
#include "engine/Expression.h"
#include "Modeler.h"

using namespace bzmag::engine;

ExpressionDialog::ExpressionDialog(ExpressionTableModel* model, QWidget* parent)
    : QDialog(parent), table_model_(model) {
    setWindowTitle("Expression Editor");
    resize(600, 400);

    QVBoxLayout* main_layout = new QVBoxLayout(this);

    // 레이아웃 여백 및 간격 최소화
    //main_layout->setContentsMargins(0, 0, 0, 0);
    //main_layout->setSpacing(0);

    table_view_ = new QTableView(this);
    table_view_->setModel(table_model_);
    table_view_->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: lightblue; }");

    // 열 크기를 사용자가 조절할 수 있도록 Interactive 모드 설정
    table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_view_->horizontalHeader()->setStretchLastSection(true);

    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setAlternatingRowColors(true); // 교차 색상 활성화

    // 테이블 뷰의 크기 조정 정책 설정
    table_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout* button_layout = new QHBoxLayout();
    add_button_ = new QPushButton("Add Expression", this);
    delete_button_ = new QPushButton("Delete Expression", this);
    apply_button_ = new QPushButton("Apply", this);
    apply_button_->setEnabled(false); // 초기에는 비활성화

    button_layout->addWidget(add_button_);
    button_layout->addWidget(delete_button_);
    button_layout->addWidget(apply_button_);

    main_layout->addWidget(table_view_);
    main_layout->addLayout(button_layout);

    connect(add_button_, &QPushButton::clicked, this, &ExpressionDialog::addExpression);
    connect(delete_button_, &QPushButton::clicked, this, &ExpressionDialog::deleteSelectedExpression);
    connect(apply_button_, &QPushButton::clicked, this, [this]() {
        applyChanges();
    });

    connect(table_model_, &ExpressionTableModel::dataModified, this, &ExpressionDialog::onDataModified);
}

void ExpressionDialog::onDataModified(bool modified) {
    apply_button_->setEnabled(modified);
}

void ExpressionDialog::applyChanges() {
    // 실제 데이터 변경 내용을 저장하는 로직
    table_model_->expressionUpdated();  // 수식 변경후 렌더뷰 오브젝트 업데이트
    table_model_->layoutChanged();      // 테이블 모델 업데이트
    apply_button_->setEnabled(false);   // 적용 후 버튼 비활성화
}

void ExpressionDialog::setReadOnly(bool readonly) {
    // 테이블 뷰를 읽기 전용으로 설정
    if (readonly) {
        table_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else {
        table_view_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    }

    // 버튼 비활성화/활성화
    add_button_->setEnabled(!readonly);
    delete_button_->setEnabled(!readonly);
}

void ExpressionDialog::closeEvent(QCloseEvent* event)
{
    if (apply_button_->isEnabled()) { // 데이터가 변경되었지만 저장되지 않은 상태
        applyChanges(); // 변경 사항 저장
    }
    event->accept(); 
}

void ExpressionDialog::addExpression() {
    AddExpressionDialog variable_dialog(this);

    // Modeler를 통해 수식 생성
    Modeler* modeler = Modeler::instance();

    // 다이얼로그 실행
    if (variable_dialog.exec() == QDialog::Accepted) {
        QString key = variable_dialog.getKey();
        QString expression = variable_dialog.getExpression();
        QString comment = variable_dialog.getComment();

        Expression* expr = modeler->createExpression(key.toStdString(), expression.toStdString(), comment.toStdString());
        if (!expr) {
            QMessageBox::warning(this, "Error", "Failed to register expression.");
            return;
        }
        table_model_->layoutChanged();
    }
}

void ExpressionDialog::deleteSelectedExpression() {
    auto current_index = table_view_->currentIndex();
    if (current_index.isValid()) {
        // 선택된 Expression 가져오기
        Expression* expr = table_model_->getExpression(current_index.row());
        if (!expr) {
            QMessageBox::warning(this, "Error", "Failed to retrieve the selected expression.");
            return;
        }

        Modeler* modeler = Modeler::instance();
        if (modeler->removeExpression(expr->getKey())) {
            table_model_->layoutChanged();
            QMessageBox::information(this, "Deleted", "Expression successfully deleted.");
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to delete the expression.");
        }
    }
}