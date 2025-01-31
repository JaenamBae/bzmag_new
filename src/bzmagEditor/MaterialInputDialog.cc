#include "MaterialInputDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "DataInputDialog.h"

MaterialInputDialog::MaterialInputDialog(QWidget* parent)
    : InputDialog(parent),
    name_edit_(new QLineEdit(this)),
    nonlinear_checkbox_(new QCheckBox("NonLinear", this)),
    permeability_edit_(new QLineEdit(this)),
    bh_curve_button_(new QPushButton("BH-Curve", this)),
    conductivity_edit_(new QLineEdit(this)),
    magnetization_edit_(new QLineEdit(this)),
    magnetization_direction_edit_(new QLineEdit(this)) {
    setWindowTitle("Material");

    // 초기 상태 설정
    bh_curve_button_->setEnabled(false);
    magnetization_direction_edit_->setEnabled(false);

    // 레이아웃 구성
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Name:", name_edit_);
    form_layout->addRow("NonLinear:", nonlinear_checkbox_);
    form_layout->addRow("Permeability:", permeability_edit_);
    form_layout->addRow("", bh_curve_button_);
    form_layout->addRow("Conductivity:", conductivity_edit_);
    form_layout->addRow("Magnetization:", magnetization_edit_);
    form_layout->addRow("Magnetization Direction:", magnetization_direction_edit_);

    conductivity_edit_->setText("0");
    magnetization_edit_->setText("0");

    // 버튼 박스
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 메인 레이아웃
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(form_layout);
    main_layout->addWidget(button_box);
    setLayout(main_layout);

    // 시그널 연결
    //DataInputDialog
    connect(nonlinear_checkbox_, &QCheckBox::toggled, this, &MaterialInputDialog::updatePermeabilityField);
    connect(magnetization_edit_, &QLineEdit::textChanged, this, &MaterialInputDialog::updateMagnetizationDirectionField);
    connect(bh_curve_button_, &QPushButton::clicked, this, [this]() {
        DataInputDialog bh_dialog(nullptr, this);
        bh_dialog.resize(700, 400);
        if (bh_dialog.exec() == QDialog::Accepted) {
            dataset_ = bh_dialog.getDataSet();
        }
    });
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (name_edit_->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Name cannot be empty.");
            name_edit_->setFocus();
            return;
        }
        if (permeability_edit_->isEnabled()) {
            if (!checkValue(permeability_edit_->text().toStdString())) {
                QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the permeability.");
                permeability_edit_->setFocus();
                return;
            }
        }
        if (!checkValue(conductivity_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the conductivity.");
            conductivity_edit_->setFocus();
            return;
        }
        if (!magnetization_edit_->text().isEmpty() && !checkValue(magnetization_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the magnetization.");
            magnetization_edit_->setFocus();
            return;
        }
        if (magnetization_direction_edit_->isEnabled()) {
            if (!checkPoint(magnetization_direction_edit_->text())) {
                QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the direction of magnetization.");
                magnetization_direction_edit_->setFocus();
                return;
            }
        }

        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString MaterialInputDialog::getName() const {
    return name_edit_->text();
}

bool MaterialInputDialog::isNonLinear() const {
    return nonlinear_checkbox_->isChecked();
}

bzmag::DataSet MaterialInputDialog::getBHCurve() const
{
    return dataset_;
}

QString MaterialInputDialog::getPermeability() const {
    return permeability_edit_->isEnabled() ? permeability_edit_->text() : "BH-Curve";
}

QString MaterialInputDialog::getConductivity() const {
    return conductivity_edit_->text();
}

QString MaterialInputDialog::getMagnetization() const {
    return magnetization_edit_->text();
}

QString MaterialInputDialog::getMagnetizationDirection() const {
    return magnetization_direction_edit_->text();
}

void MaterialInputDialog::updatePermeabilityField() {
    bool is_nonlinear = nonlinear_checkbox_->isChecked();
    permeability_edit_->setEnabled(!is_nonlinear);
    bh_curve_button_->setEnabled(is_nonlinear);
}

void MaterialInputDialog::updateMagnetizationDirectionField() {
    bool has_magnetization = !magnetization_edit_->text().trimmed().isEmpty() && magnetization_edit_->text() != "0";
    magnetization_direction_edit_->setEnabled(has_magnetization);
}
