#include "TemplatePropertyDialog.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QPainter>
#include <QComboBox>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include "Modeler.h"
#include "core/tokenizer.h"
#include "engine/Expression.h"
#include "PlatformDepandent.h"

using namespace bzmag;
using namespace bzmag::engine;

// 생성자
TemplatePropertyDialog::TemplatePropertyDialog(TemplateNode* node, QWidget* parent)
    : QDialog(parent), node_(node) {
    building_ = true;

    if (!node) return;

    manager_ = node->getManager();
    if (!manager_ || !manager_->valid()) return;

    template_name_ = manager_->getTemplateName();

    // 이미지 및 트리 기본 너비 설정
    const int image_width = 400;
    const int tree_width = 300;
    const int height = 370;

    // 최상위 레이아웃 (좌우 분할)
    QHBoxLayout* main_layout = new QHBoxLayout(this);

    // 좌측: 이미지 출력
    QWidget* left_widget = new QWidget(this);
    left_widget->setStyleSheet("background-color: white;");

    QVBoxLayout* left_layout = new QVBoxLayout(left_widget);
    left_layout->setAlignment(Qt::AlignCenter);

    image_label_ = new QLabel(this);
    image_label_->setAlignment(Qt::AlignCenter);
    image_label_->setMinimumWidth(image_width);
    image_label_->setMinimumHeight(height / 2);
    left_layout->addWidget(image_label_, 0);

    main_layout->addWidget(left_widget, 1);

    // 우측: 네비게이션, 트리, 버튼
    QVBoxLayout* right_layout = new QVBoxLayout();

    navigator_ = new QComboBox(this);
    right_layout->addWidget(navigator_);

    property_browser_ = new QtTreePropertyBrowser(this);
    variant_manager_ = new QtVariantPropertyManager(this);
    editor_factory_ = new QtVariantEditorFactory(this);
    property_browser_->setFactoryForManager(variant_manager_, editor_factory_);
    property_browser_->setMinimumWidth(tree_width);
    right_layout->addWidget(property_browser_, 1);

    QHBoxLayout* button_layout = new QHBoxLayout();
    apply_button_ = new QPushButton("Apply", this);
    close_button_ = new QPushButton("Close", this);
    button_layout->addWidget(apply_button_);
    button_layout->addWidget(close_button_);
    right_layout->addLayout(button_layout);

    main_layout->addLayout(right_layout, 1);

    resize(image_width + tree_width, height);

    connect(navigator_, &QComboBox::currentTextChanged, this, &TemplatePropertyDialog::onNavigatorTextChanged);
    connect(variant_manager_, &QtVariantPropertyManager::valueChanged, this, &TemplatePropertyDialog::onValueChanged);
    connect(property_browser_, &QtTreePropertyBrowser::currentItemChanged, this, &TemplatePropertyDialog::onPropertyFocused);
    connect(apply_button_, &QPushButton::clicked, this, &TemplatePropertyDialog::onApplyButtonClicked);
    connect(close_button_, &QPushButton::clicked, this, &TemplatePropertyDialog::close);

    prepairPropertyBrowser();
    populatePropertyBrowser();

    building_ = false;
}

void TemplatePropertyDialog::setReadOnly(bool readonly)
{
    if (readonly) {
        property_browser_->unsetFactoryForManager(variant_manager_);
        //property_browser_->setEnabled(false); // 전체 비활성화
    }
    else {
        property_browser_->setFactoryForManager(variant_manager_, editor_factory_);
        //property_browser_->setEnabled(true); // 전체 활성화
    }

    // 버튼 비활성화/활성화
    apply_button_->setEnabled(!readonly);
    //close_button_->setEnabled(!readonly);

    property_browser_->update();
}

// 속성 브라우저 준비
void TemplatePropertyDialog::prepairPropertyBrowser() {
    json root_obj = manager_->getConfigureObject();
    if (!root_obj.is_object()) return;

    for (auto& [group_key, group_variant] : root_obj.items()) {
        QtVariantProperty* group_item = variant_manager_->addProperty(QtVariantPropertyManager::groupTypeId(), QString::fromStdString(group_key));
        for (auto& [item_key, item_variant] : group_variant.items()) {
            if (!item_variant.is_object()) continue;

            auto dtype = item_variant["dtype"].get<std::string>();
            auto description = item_variant["description"].get<std::string>();
            auto value = item_variant["temp_value"];
            bool readonly = item_variant["readonly"].get<bool>();

            QtVariantProperty* property_item = nullptr;
            if (dtype == "int") {
                property_item = variant_manager_->addProperty(QVariant::Int, QString::fromStdString(item_key));
                property_item->setValue(value.get<int>());
            }
            else if (dtype == "float") {
                property_item = variant_manager_->addProperty(QVariant::String, QString::fromStdString(item_key));
                property_item->setValue(value.get<double>());
            }
            else if (dtype == "string") {
                property_item = variant_manager_->addProperty(QVariant::String, QString::fromStdString(item_key));
                property_item->setValue(QString::fromStdString(value.get<std::string>()));
            }
            else {
                // dtype이 지원되지 않음
            }

            if (property_item) {
                property_item->setToolTip(QString::fromStdString(description));
                property_item->setAttribute("readOnly", readonly);
                group_item->addSubProperty(property_item);

                std::string path = group_key + "/" + item_key;// +"/value";
                path_map_[property_item] = path;
            }
        }
        if (current_group_.empty()) current_group_ = group_key;
        property_group_[group_key] = group_item;
        navigator_->addItem(QString::fromStdString(group_key));
    }
}

// 속성 브라우저 업데이트
void TemplatePropertyDialog::populatePropertyBrowser() {
    property_browser_->clear();

    QtVariantProperty* group_item = property_group_[current_group_];
    if (group_item) {
        property_browser_->addProperty(group_item);

        std::string template_path(QCoreApplication::applicationDirPath().toStdString());
        std::string image_path = template_path + "/template/images/" + template_name_ + "/" + current_group_ + "/default.png";  // 이미지 경로 생성
        loadImageAsync(image_path);
    }
}

void TemplatePropertyDialog::onNavigatorTextChanged(const QString& group_name) {
    if (building_) return;

    current_group_ = group_name.toStdString();
    populatePropertyBrowser();
}

// 값 변경 슬롯
void TemplatePropertyDialog::onValueChanged(QtProperty* property, const QVariant& new_value) {
    if (building_) return;

    json item = manager_->getValue(path_map_[property]);
    std::string path = path_map_[property] + "/temp_value";
    std::string dtype = item["dtype"];
    
    if (dtype == "bool") {
        manager_->setValue(path, new_value.toBool());
    }
    else if (dtype == "int") {
        manager_->setValue(path, new_value.toInt());
    }
    else if (dtype == "float") {
        manager_->setValue(path, new_value.toDouble());
    }
    else if (dtype == "string") {
        manager_->setValue(path, new_value.toString().toStdString());
    }
}

// 속성 포커스 변경 슬롯
void TemplatePropertyDialog::onPropertyFocused(QtBrowserItem* item) {
    if (!item || building_) return;

    QtProperty* property = item->property();
    QString item_key = property->propertyName();

    // 특정 프로퍼티 이름에 따라 이미지 경로 설정
    std::string path = path_map_[property] + "/image";  // 경로 매핑에서 경로 조회

    Tokenizer token(path.c_str(), '/');
    std::string group_name(token.begin()->c_str());
    
    Modeler* modeler = Modeler::instance();
    json temp = manager_->getValue(path);
    if (temp.is_string()) {
        std::string image_name = temp.get<std::string>();
        //std::string template_path(QCoreApplication::applicationDirPath().toStdString());
        
        std::string template_path(getExecPath());
        std::string image_path = template_path + "/template/images/" + template_name_ + "/" + group_name + "/" + image_name;  // 이미지 경로 생성
        loadImageAsync(image_path);
    }

    // 타겟 이미지 이름이 없는 경우 대표 이미지 이름 사용
    else {
        std::string template_path(getExecPath());
        std::string image_path = template_path + "/template/images/" + template_name_ + "/" + group_name + "/default.png";  // 이미지 경로 생성
        loadImageAsync(image_path);
    }
}

void TemplatePropertyDialog::onApplyButtonClicked()
{
    // 제약조건 체크; 모든 값은 temp_value에 저장된 상태로 다음 작업에 동시에 일어난다
    // - temp_value에 값이 채워지지 않은 경우 default값으로 채움
    // - temp_value기반 제약조건 테스트가 성공하면 temp_value를 value에 저장한다
    if (!manager_->checkConstraints()) {
        QMessageBox::warning(this, "Invalid Expression", "Constraints check failed");
        return;
    }

    // default_value, 혹은 value를 이용해 expression dictionary를 만든다
    json configure_object = manager_->buildExpressionsFromConfigure(false);
    for (const auto& child : configure_object["children"]) {
        std::string key = child["Key"];
        Expression* exprv = node_->getExpression(key);
        if (exprv) {
            std::string expression = child["Expression"];
            exprv->setExpression(expression);
        }
    }

    manager_->templateUpdated();
    QMessageBox::information(this, "Apply Successful", "Configurations are applied");
}

// 이미지 비동기 로드
void TemplatePropertyDialog::loadImageAsync(const std::string& image_path) {
    QString q_image_path = QString::fromStdString(image_path);
    q_image_path.replace(" ", "_");
    // 캐시에 이미지가 있는지 확인
    if (pixmap_cache_.contains(q_image_path)) {
        image_label_->setPixmap(*pixmap_cache_[q_image_path]);  // 캐시된 이미지를 바로 사용
        return;
    }

    // 캐시에 없는 경우 비동기로 이미지 로드
    QFuture<QPixmap> future = QtConcurrent::run([=]() {
        QPixmap pixmap(q_image_path);
        if (!pixmap.isNull()) {
            // 흰색 배경의 QPixmap 생성
            QPixmap white_background_pixmap(400, pixmap.height() * 400 / pixmap.width());
            white_background_pixmap.fill(Qt::white);  // 배경을 흰색으로 설정

            // QPainter로 이미지를 흰색 배경 위에 그림
            QPainter painter(&white_background_pixmap);
            QPixmap scaled_pixmap = pixmap.scaledToWidth(400, Qt::SmoothTransformation);
            painter.drawPixmap(0, 0, scaled_pixmap);
            painter.end();

            return white_background_pixmap;
        }
        else {
            return QPixmap();
        }
    });

    // 기존 연결 확인 및 제거
    disconnect(&future_watcher_, &QFutureWatcher<QPixmap>::finished, nullptr, nullptr);

    // 비동기 결과가 완료되면 QLabel에 설정 및 캐시에 저장
    connect(&future_watcher_, &QFutureWatcher<QPixmap>::finished, [=]() {
        QPixmap pixmap = future_watcher_.result();
        if (!pixmap.isNull()) {
            image_label_->setPixmap(pixmap);
            pixmap_cache_.insert(q_image_path, new QPixmap(pixmap));  // 캐시에 삽입
        }
        else {
            image_label_->setText("Image not found");
        }
    });

    future_watcher_.setFuture(future);
}
