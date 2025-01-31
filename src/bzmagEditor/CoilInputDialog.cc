#include "CoilInputDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>

CoilInputDialog::CoilInputDialog(bzmag::engine::WindingNode* winding, std::vector<bzmag::engine::GeomHeadNode*> heads, QWidget* parent)
    : InputDialog(parent),
    name_edit_(new QLineEdit(this)),
    direction_combo_(new QComboBox(this)),
    turns_edit_(new QLineEdit(this)),
    available_list_widget_(new QListWidget(this)),
    selected_list_widget_(new QListWidget(this)),
    move_down_button_(new QPushButton("▼", this)),
    move_up_button_(new QPushButton("▲", this)),
    button_box_(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this)) {
    
    QString title(winding->getName().c_str());
    title = title + " - Add Coils";
    setWindowTitle(title);

    // Direction 리스트 설정
    direction_combo_->addItems({ "Positive", "Negative" });

    // 왼쪽 입력 필드 레이아웃
    QFormLayout* form_layout = new QFormLayout();
    form_layout->addRow("Name:", name_edit_);
    form_layout->addRow("Direction:", direction_combo_);
    form_layout->addRow("Turns:", turns_edit_);

    // QListWidget 크기 조정
    available_list_widget_->setFixedSize(150, 100);
    selected_list_widget_->setFixedSize(150, 100);

    // 버튼 크기 조정
    move_down_button_->setFixedSize(30, 20);
    move_up_button_->setFixedSize(30, 20);

    // 버튼 레이아웃 (수평으로 배치)
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    button_layout->addWidget(move_down_button_);
    button_layout->addWidget(move_up_button_);
    button_layout->addStretch();

    // 오른쪽 레이아웃: 위-아래 리스트와 버튼
    QVBoxLayout* right_layout = new QVBoxLayout();
    right_layout->addWidget(available_list_widget_);
    right_layout->addLayout(button_layout); // 버튼 레이아웃 추가
    right_layout->addWidget(selected_list_widget_);

    // 메인 레이아웃: 좌우 구성
    QHBoxLayout* main_layout = new QHBoxLayout();
    main_layout->addLayout(form_layout);   // 왼쪽 입력 필드
    main_layout->addLayout(right_layout); // 오른쪽 리스트와 버튼

    QVBoxLayout* final_layout = new QVBoxLayout(this);
    final_layout->addLayout(main_layout);
    final_layout->addWidget(button_box_); // 하단에 버튼 추가

    QStringList top_list;
    for (const auto& head : heads) {
        QString name(head->getName().c_str());
        top_list.append(name);
        string_to_node_[name] = head;
    }
    available_list_widget_->addItems(top_list);

    setLayout(final_layout);

    // 시그널-슬롯 연결
    setupConnections();
    // 버튼 시그널 연결
    connect(button_box_, &QDialogButtonBox::accepted, this, [this]() {
        if (name_edit_->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Name cannot be empty.");
            name_edit_->setFocus();
            return;
        }
        if (!checkValue(turns_edit_->text().toStdString())) {
            QMessageBox::warning(this, "Input Error", "Failed to evaluate the value at the Turns.");
            turns_edit_->setFocus();
            return;
        }
        if (selected_list_widget_->count() == 0) {
            QMessageBox::warning(this, "Input Error", "The selected items must be existed.");
            return;
        }
        accept();
    });
    connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CoilInputDialog::setupConnections() {
    // ▼ 버튼: 위 → 아래
    connect(move_down_button_, &QPushButton::clicked, this, [this]() {
        auto items = available_list_widget_->selectedItems();
        for (auto item : items) {
            available_list_widget_->takeItem(available_list_widget_->row(item));
            selected_list_widget_->addItem(item);
        }
    });

    // ▲ 버튼: 아래 → 위
    connect(move_up_button_, &QPushButton::clicked, this, [this]() {
        auto items = selected_list_widget_->selectedItems();
        for (auto item : items) {
            selected_list_widget_->takeItem(selected_list_widget_->row(item));
            available_list_widget_->addItem(item);
        }
    });
}

QString CoilInputDialog::getName() const {
    return name_edit_->text();
}

QString CoilInputDialog::getDirection() const {
    return direction_combo_->currentText();
}

QString CoilInputDialog::getTurns() const {
    return turns_edit_->text();
}

std::vector<bzmag::engine::GeomHeadNode*> CoilInputDialog::getSelectedNodes() const
{
    std::vector<bzmag::engine::GeomHeadNode*> selected;
    for (int i = 0; i < selected_list_widget_->count(); ++i) {
        QString name = selected_list_widget_->item(i)->text();
        selected.push_back(string_to_node_[name]);
    }
    return selected;
}

void CoilInputDialog::setStringList(const QStringList& strings) {
    available_list_widget_->clear();
    selected_list_widget_->clear();
    available_list_widget_->addItems(strings);
}
