#include "SplitInputDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

SplitInputDialog::SplitInputDialog(QWidget* parent)
    : QDialog(parent),
    plane_combo_box_(new QComboBox(this)),
    positive_button_(new QRadioButton("Positive", this)),
    negative_button_(new QRadioButton("Negative", this)),
    direction_group_(new QButtonGroup(this)) {
    setWindowTitle("Split");

    // 평면 선택 콤보박스 설정
    plane_combo_box_->addItem("YZ-Plane");
    plane_combo_box_->addItem("ZX-Plane");

    // 라디오 버튼 그룹 설정
    direction_group_->addButton(positive_button_);
    direction_group_->addButton(negative_button_);
    positive_button_->setChecked(true); // 기본값 Positive

    // 레이아웃 구성
    QVBoxLayout* main_layout = new QVBoxLayout(this);

    // 평면 선택 레이아웃
    QHBoxLayout* plane_layout = new QHBoxLayout();
    plane_layout->addWidget(new QLabel("Select Plane:", this));
    plane_layout->addWidget(plane_combo_box_);
    main_layout->addLayout(plane_layout);

    // 방향 선택 레이아웃
    QHBoxLayout* direction_layout = new QHBoxLayout();
    direction_layout->addWidget(new QLabel("Select Direction:", this));
    direction_layout->addWidget(positive_button_);
    direction_layout->addWidget(negative_button_);
    main_layout->addLayout(direction_layout);

    // 버튼 박스
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    main_layout->addWidget(button_box);

    // 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString SplitInputDialog::getSelectedPlane() const {
    return plane_combo_box_->currentText();
}

QString SplitInputDialog::getSelectedDirection() const {
    return positive_button_->isChecked() ? "Positive" : "Negative";
}
