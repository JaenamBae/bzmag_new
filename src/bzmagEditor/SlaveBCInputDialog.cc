#include "SlaveBCInputDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include "Modeler.h"

SlaveBCInputDialog::SlaveBCInputDialog(QWidget* parent)
    : QDialog(parent),
    name_edit_(new QLineEdit(this)),
    direction_check_(new QCheckBox("Direction", this)),
    match_combo_(new QComboBox(this)),
    master_combo_(new QComboBox(this)) {
    setWindowTitle("Slave BC");

    Modeler* modeler = Modeler::instance();
    bzmag::Node* root = modeler->getWorkingBCRootNode();

    // Master 리스트 설정
    QStringList master_list;
    for (auto it = root->firstChildNode(); it != root->lastChildNode(); ++it) {
        bzmag::engine::MasterPeriodicBCNode* master = it->get<bzmag::engine::MasterPeriodicBCNode*>();
        if (master) {
            QString name(master->getName().c_str());
            master_list.push_back(name);
            string_to_node_[name] = master;
        }
    }

    master_combo_->addItems(master_list);

    // Match 리스트 설정
    match_combo_->addItems({ "Even", "Odd" });

    // 레이아웃 설정
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Name:", name_edit_);
    form_layout->addRow("Direction:", direction_check_);
    form_layout->addRow("Master:", master_combo_);
    form_layout->addRow("Match:", match_combo_);

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
        if (master_combo_->currentText().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "A master must be selected.");
            return;
        }
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString SlaveBCInputDialog::getName() const {
    return name_edit_->text();
}

bool SlaveBCInputDialog::getDirection() const {
    return direction_check_->isChecked();
}

bzmag::engine::MasterPeriodicBCNode* SlaveBCInputDialog::getMaster() const {
    QString master_name = master_combo_->currentText();
    return string_to_node_[master_name];
}

QString SlaveBCInputDialog::getMatch() const {
    return match_combo_->currentText();
}
