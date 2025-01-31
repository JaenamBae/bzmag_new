#include "MasterBCInputDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

MasterBCInputDialog::MasterBCInputDialog(QWidget* parent)
    : QDialog(parent),
      name_edit_(new QLineEdit(this)),
      direction_check_(new QCheckBox("Direction", this)) {
    setWindowTitle("Master BC");

    // 레이아웃 설정
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Name:", name_edit_);
    form_layout->addRow("Direction:", direction_check_);

    // 버튼 박스
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 메인 레이아웃 구성
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(form_layout);
    main_layout->addWidget(button_box);
    setLayout(main_layout);

    // 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (name_edit_->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Name cannot be empty.");
            return;
        }
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString MasterBCInputDialog::getName() const {
    return name_edit_->text();
}

bool MasterBCInputDialog::getDirection() const {
    return direction_check_->isChecked();
}
