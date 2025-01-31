#include "CSInputDialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDoubleValidator>

CSInputDialog::CSInputDialog(QWidget* parent)
    : InputDialog(parent),
    name_edit_(new QLineEdit(this)),
    center_edit_(new QLineEdit(this)),
    angle_edit_(new QLineEdit(this)) {
    setWindowTitle("Coordinate System");

    // 입력 필드 레이아웃
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow(new QLabel("Name:", this), name_edit_);
    form_layout->addRow(new QLabel("Origin (x,y):", this), center_edit_);
    form_layout->addRow(new QLabel("Angle (radian):", this), angle_edit_);

    // 코멘트 라벨 추가
    QLabel* comment_label = new QLabel("'_pi' is available", this);
    comment_label->setStyleSheet("color: gray; font-style: italic;"); 
    comment_label->setAlignment(Qt::AlignRight); // 텍스트 중앙 정렬

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
        if (!checkValue(angle_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the angle.");
            angle_edit_->setFocus();
            return;
        }
        accept();
    });

    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 메인 레이아웃 구성
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(form_layout);  // 입력 필드
    main_layout->addWidget(comment_label); // 코멘트 라벨 추가
    main_layout->addWidget(button_box);   // 버튼 박스 (하단)
    setLayout(main_layout);
}

QString CSInputDialog::getName() const {
    return name_edit_->text();
}

QString CSInputDialog::getCenter() const {
    return center_edit_->text();
}

QString CSInputDialog::getAngle() const {
    return angle_edit_->text();
}
