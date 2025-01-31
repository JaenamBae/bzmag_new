#include "AddExpressionDialog.h"
#include <QMessageBox>
#include "engine/Expression.h"
#include "engine/ExpressionServer.h"

using namespace bzmag::engine;

AddExpressionDialog::AddExpressionDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Add Expression");
    resize(350, 150); // 창의 폭을 넓히기 위해 수정

    QVBoxLayout* main_layout = new QVBoxLayout(this);

    expression_edit = new QLineEdit(this);
    comment_edit = new QTextEdit(this); // QTextEdit 사용
    comment_edit->setFixedHeight(expression_edit->height() * 2); // 높이를 expression_edit의 두 배로 설정

    expression_edit->setPlaceholderText("Enter Expression (e.g., key=expression)");
    comment_edit->setPlaceholderText("Enter Comment");

    main_layout->addWidget(expression_edit);
    main_layout->addWidget(comment_edit);

    QHBoxLayout* button_layout = new QHBoxLayout();
    QPushButton* ok_button = new QPushButton("OK", this);
    QPushButton* cancel_button = new QPushButton("Cancel", this);

    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    main_layout->addLayout(button_layout);

    connect(ok_button, &QPushButton::clicked, [this]() {
        QString input = expression_edit->text();
        QStringList parts = input.split('=');

        if (parts.size() != 2) {
            QMessageBox::warning(this, "Invalid Input", "The expression must be in the format 'key=expression'.");
            return;
        }

        key_ = parts[0].trimmed();
        expression_ = parts[1].trimmed();

        // Expression 객체 검증
        ExpressionServer* expr_server = ExpressionServer::instance();
        if (expr_server->findExpression(key_.toStdString())) {
            QMessageBox::warning(this, "Invalid Key", "The key is invalid or already exists.");
            return;
        }
        if (!expr_server->checkConsistancy(key_.toStdString(), expression_.toStdString())) {
            QMessageBox::warning(this, "Invalid Expression", "The expression is invalid.");
            return;
        }

        accept(); // 입력이 유효하면 다이얼로그 닫기
    });

    connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);
}

QString AddExpressionDialog::getKey() const {
    return key_;
}

QString AddExpressionDialog::getExpression() const {
    return expression_;
}

QString AddExpressionDialog::getComment() const {
    return comment_edit->toPlainText(); // QTextEdit에서 텍스트 가져오기
}
