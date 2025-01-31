#include "MoveInputDialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>

MoveInputDialog::MoveInputDialog(QWidget* parent)
    : InputDialog(parent),
    dx_edit_(new QLineEdit(this)),
    dy_edit_(new QLineEdit(this)) {
    setWindowTitle("Move");

    // 입력 필드 레이아웃
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow(new QLabel("dx:", this), dx_edit_);
    form_layout->addRow(new QLabel("dy:", this), dy_edit_);

    // 버튼 박스 생성
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 버튼 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (!checkValue(dx_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the dx.");
            dx_edit_->setFocus();
            return;
        }
        if (!checkValue(dy_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the dy.");
            dy_edit_->setFocus();
            return;
        }
        accept();
    });

    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 메인 레이아웃 구성
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(form_layout);  // 입력 필드
    main_layout->addWidget(button_box);  // 버튼 박스 (하단)

    setLayout(main_layout);
}

QString MoveInputDialog::getDx() const {
    return dx_edit_->text();
}

QString MoveInputDialog::getDy() const {
    return dy_edit_->text();
}
