#include "MovingBandInputDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

MovingBandInputDialog::MovingBandInputDialog(QWidget* parent)
    : InputDialog(parent),
      name_edit_(new QLineEdit(this)),
      initial_position_edit_(new QLineEdit(this)),
      speed_edit_(new QLineEdit(this)) {
    setWindowTitle("Moving Band");

    // 레이아웃 설정
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Name:", name_edit_);
    form_layout->addRow("Initial Position:", initial_position_edit_);
    form_layout->addRow("Speed:", speed_edit_);

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
        if (!checkValue(initial_position_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the Initial Position.");
            initial_position_edit_->setFocus();
            return;
        }
        if (!checkValue(speed_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the Speed.");
            speed_edit_->setFocus();
            return;
        }
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString MovingBandInputDialog::getName() const {
    return name_edit_->text();
}

QString MovingBandInputDialog::getInitialPosition() const {
    return initial_position_edit_->text();
}

QString MovingBandInputDialog::getSpeed() const {
    return speed_edit_->text();
}
