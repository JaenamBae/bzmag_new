#include "TransientInputDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

TransientInputDialog::TransientInputDialog(QWidget* parent)
    : InputDialog(parent),
      stop_time_edit_(new QLineEdit(this)),
      time_step_edit_(new QLineEdit(this)) {
    setWindowTitle("Transient");

    // 레이아웃 설정
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Stop Time:", stop_time_edit_);
    form_layout->addRow("Time Step:", time_step_edit_);

    // 버튼 박스
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 메인 레이아웃 구성
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(form_layout);
    main_layout->addWidget(button_box);
    setLayout(main_layout);

    // 시그널 연결
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (!checkValue(stop_time_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the Stop Time.");
            stop_time_edit_->setFocus();
            return;
        }
        if (!checkValue(time_step_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the Time Step.");
            time_step_edit_->setFocus();
            return;
        }
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString TransientInputDialog::getStopTime() const {
    return stop_time_edit_->text();
}

QString TransientInputDialog::getTimeStep() const {
    return time_step_edit_->text();
}
