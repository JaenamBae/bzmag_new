#include "CircleInputDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFormLayout>
#include <QDoubleValidator>
#include <QDialogButtonBox>

CircleInputDialog::CircleInputDialog(QWidget* parent)
    : InputDialog(parent),
    name_edit_(new QLineEdit(this)),
    center_edit_(new QLineEdit(this)),
    radius_edit_(new QLineEdit(this)) {
    setWindowTitle("Circle");

    // 입력 필드 레이아웃
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow(new QLabel("Name:", this), name_edit_);
    form_layout->addRow(new QLabel("Center (x,y):", this), center_edit_);
    form_layout->addRow(new QLabel("Radius:", this), radius_edit_);

    // 버튼 박스 생성
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 버튼 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (name_edit_->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Name cannot be empty.");
            name_edit_->setFocus();
            return;
        }
        if (!checkPoint(center_edit_->text())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the center point.");
            center_edit_->setFocus();
            return;
        }
        if (!checkValue(radius_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the radius.");
            radius_edit_->setFocus();
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

QString CircleInputDialog::getName() const {
    return name_edit_->text();
}

QString CircleInputDialog::getCenter() const {
    return center_edit_->text();
}

QString CircleInputDialog::getRadius() const {
    return radius_edit_->text();
}
