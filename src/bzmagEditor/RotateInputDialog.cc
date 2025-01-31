#include "RotateInputDialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>

RotateInputDialog::RotateInputDialog(QWidget* parent)
    : InputDialog(parent),
    angle_edit_(new QLineEdit(this)) {
    setWindowTitle("Rotate");

    // 입력 필드 레이아웃
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow(new QLabel("Angle (radian):", this), angle_edit_);

    // 코멘트 라벨 추가
    QLabel* comment_label = new QLabel("'_pi' is available", this);
    comment_label->setStyleSheet("color: gray; font-style: italic;");
    comment_label->setAlignment(Qt::AlignRight); // 텍스트 중앙 정렬

    // 버튼 박스 생성
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 버튼 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
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
    main_layout->addWidget(comment_label);
    main_layout->addWidget(button_box);  // 버튼 박스 (하단)

    setLayout(main_layout);
}

QString RotateInputDialog::getAngle() const {
    return angle_edit_->text();
}
