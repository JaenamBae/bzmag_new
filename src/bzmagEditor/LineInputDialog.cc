﻿#include "LineInputDialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDoubleValidator>

LineInputDialog::LineInputDialog(QWidget* parent)
    : InputDialog(parent),
    name_edit_(new QLineEdit(this)),
    start_point_edit_(new QLineEdit(this)),
    end_point_edit_(new QLineEdit(this)) {
    setWindowTitle("Line");

    // 입력 필드 레이아웃
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow(new QLabel("Name:", this), name_edit_);
    form_layout->addRow(new QLabel("Start Point (x,y):", this), start_point_edit_);
    form_layout->addRow(new QLabel("End Point (x,y):", this), end_point_edit_);

    // 버튼 박스 생성
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 버튼 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (name_edit_->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Name cannot be empty.");
            name_edit_->setFocus();
            return;
        }
        if (!checkPoint(start_point_edit_->text())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the start point.");
            start_point_edit_->setFocus();
            return;
        }
        if (!checkPoint(end_point_edit_->text())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the end point.");
            end_point_edit_->setFocus();
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

QString LineInputDialog::getName() const {
    return name_edit_->text();
}

QString LineInputDialog::getStartPoint() const {
    return start_point_edit_->text();
}

QString LineInputDialog::getEndPoint() const {
    return end_point_edit_->text();
}
