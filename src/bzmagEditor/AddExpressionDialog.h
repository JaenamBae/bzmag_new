#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>

class AddExpressionDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddExpressionDialog(QWidget* parent = nullptr);

    QString getKey() const;
    QString getExpression() const;
    QString getComment() const;

private:
    QLineEdit* expression_edit;
    QTextEdit* comment_edit; // QTextEdit으로 변경하여 높이 조정 가능

    QString key_;
    QString expression_;
};
