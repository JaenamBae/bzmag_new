#include "WindingInputDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDialogButtonBox>

WindingInputDialog::WindingInputDialog(QWidget* parent)
    : InputDialog(parent),
      name_edit_(new QLineEdit(this)),
      current_edit_(new QLineEdit(this)),
      parallel_branch_edit_(new QLineEdit(this)) {
    setWindowTitle("Winding");

    // 레이아웃 설정
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Name:", name_edit_);
    form_layout->addRow("Current:", current_edit_);
    form_layout->addRow("Parallel Branch:", parallel_branch_edit_);

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
        if (!checkValue(current_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the Current.");
            current_edit_->setFocus();
            return;
        }
        if (!checkValue(parallel_branch_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the parallel branch.");
            parallel_branch_edit_->setFocus();
            return;
        }
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString WindingInputDialog::getName() const {
    return name_edit_->text();
}

QString WindingInputDialog::getCurrent() const {
    return current_edit_->text();
}

QString WindingInputDialog::getParallelBranch() const {
    return parallel_branch_edit_->text();
}
