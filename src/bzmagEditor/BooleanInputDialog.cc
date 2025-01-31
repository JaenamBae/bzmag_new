#include "BooleanInputDialog.h"
#include <QListWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

BooleanInputDialog::BooleanInputDialog(bzmag::engine::GeomHeadNode* head, std::vector<bzmag::engine::GeomHeadNode*> tools, int type, QWidget* parent)
    : QDialog(parent),
    left_list_widget_(new QListWidget(this)),
    right_list_widget_(new QListWidget(this)),
    to_right_button_(new QPushButton(" >> ", this)),
    to_left_button_(new QPushButton(" << ", this)) {
    if (type == 1) {
        setWindowTitle("Subtract");
    }
    else if (type == 2) {
        setWindowTitle("Unite");
    }
    else if (type == 3) {
        setWindowTitle("Intersect");
    }

    // 리스트 창 크기 조정
    left_list_widget_->setFixedWidth(150);
    right_list_widget_->setFixedWidth(150);

    // 버튼 크기 조정
    to_right_button_->setFixedSize(40, 25);
    to_left_button_->setFixedSize(40, 25);

    // 초기 리스트 설정
    QStringList left_list;
    QString name(head->getName().c_str());
    left_list.append(name);
    string_to_node_[name] = head;

    QStringList right_list;
    for (auto& tool : tools) {
        QString name(tool->getName().c_str());
        right_list.append(name);
        string_to_node_[name] = tool;
    }
    left_list_widget_->addItems(left_list);
    right_list_widget_->addItems(right_list);

    // 버튼 비활성화 (초기에는 아무것도 선택되지 않았으므로)
    to_right_button_->setEnabled(false);
    to_left_button_->setEnabled(false);

    // 중앙 버튼 레이아웃
    QVBoxLayout* button_layout = new QVBoxLayout();
    button_layout->addWidget(to_right_button_);
    button_layout->addWidget(to_left_button_);
    button_layout->addStretch();

    // 리스트와 버튼을 배치
    QHBoxLayout* list_layout = new QHBoxLayout();
    list_layout->addWidget(left_list_widget_);
    list_layout->addLayout(button_layout);
    list_layout->addWidget(right_list_widget_);

    // 버튼 박스 (OK / Cancel)
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 메인 레이아웃
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addLayout(list_layout); // 리스트와 버튼
    main_layout->addWidget(button_box);  // 하단 버튼 박스

    setLayout(main_layout);

    // 시그널-슬롯 연결
    connect(left_list_widget_, &QListWidget::itemSelectionChanged, this, [this]() {
        to_right_button_->setEnabled(!left_list_widget_->selectedItems().isEmpty());
    });
    connect(right_list_widget_, &QListWidget::itemSelectionChanged, this, [this]() {
        to_left_button_->setEnabled(!right_list_widget_->selectedItems().isEmpty());
    });
    connect(to_right_button_, &QPushButton::clicked, this, &BooleanInputDialog::moveToRight);
    connect(to_left_button_, &QPushButton::clicked, this, &BooleanInputDialog::moveToLeft);
    // OK 버튼 동작
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        if (left_list_widget_->count() != 1) {
            QMessageBox::warning(this, "Validation Error", "The left list must contain exactly one item.");
            return;
        }
        accept();
    });

    // Cancel 버튼 동작
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bzmag::engine::GeomBaseNode* BooleanInputDialog::getLeft() const {
    QStringList items;
    if (left_list_widget_->count() == 1) {
        QString name = left_list_widget_->item(0)->text();
        return string_to_node_[name];
    }
    return nullptr;
}

std::vector<bzmag::engine::GeomBaseNode*> BooleanInputDialog::getRightList() const {
    std::vector<bzmag::engine::GeomBaseNode*> tools;
    for (int i = 0; i < right_list_widget_->count(); ++i) {
        QString name = right_list_widget_->item(i)->text();
        tools.push_back(string_to_node_[name]);
    }
    return tools;
}

void BooleanInputDialog::moveToRight() {
    for (QListWidgetItem* item : left_list_widget_->selectedItems()) {
        right_list_widget_->addItem(item->text());
        delete left_list_widget_->takeItem(left_list_widget_->row(item));
    }
    to_right_button_->setEnabled(false);
}

void BooleanInputDialog::moveToLeft() {
    for (QListWidgetItem* item : right_list_widget_->selectedItems()) {
        left_list_widget_->addItem(item->text());
        delete right_list_widget_->takeItem(right_list_widget_->row(item));
    }
    to_left_button_->setEnabled(false);
}
