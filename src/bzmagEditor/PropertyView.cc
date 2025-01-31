#include "PropertyView.h"
#include <qradiobutton.h>
#include <spdlog/spdlog.h>

#include "core/typeid.h"
#include "core/autoreleasepool.h"
#include "core/simpleproperty.h"
#include "core/enumproperty.h"
#include "engine/CSNode.h"
#include "engine/BCNode.h"
#include "engine/MaterialNode.h"
#include "engine/GeomHeadNode.h"

#include "modeler.h"
#include "DataInputDialog.h"
#include <QLabel>

using namespace bzmag;
using namespace bzmag::engine;

PropertyView::PropertyView(QWidget* parent) : QWidget(parent), modeler_(nullptr),
    vbox_(new QVBoxLayout(this)), node_(nullptr), editable_name_(true),
    flag_reset_(false)
{
    // QtPropertyBrowser 구성 요소 초기화
    property_browser_ = new QtTreePropertyBrowser(this);
    variant_manager_ = new QtVariantPropertyManager(this);
    editor_factory_ = new QtVariantEditorFactory(this);

    property_browser_->setFactoryForManager(variant_manager_, editor_factory_);
    vbox_->setMargin(0);
    vbox_->addWidget(property_browser_);
    
    // 옛 스타일 시그널-슬롯 연결
    connect(variant_manager_, SIGNAL(valueChanged(QtProperty*, const QVariant&)),
        this, SLOT(valueChanged(QtProperty*, const QVariant&)));

    modeler_ = Modeler::instance();
}

void PropertyView::displayProperties(Node* node)
{
    flag_reset_ = true;

    node_ = node;
    
    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(node);
    if (head) {
        editable_name_ = true;
    }
    else {
        editable_name_ = false;
    }
    
    property_browser_->clear();

    if (node_ != nullptr) {
        makeObjectEnumProperty();
        Type* type = node_->getType();
        while (type) {
            addGroupProperty(type);
            type = type->getBase();
        }

        setReadOnly(modeler_->isModelerLocked());
    }

    flag_reset_ = false;
}

void PropertyView::addGroupProperty(const Type* type)
{
    QString group_name(tr(type->getName().c_str()));
    if (type->firstProperty() == type->lastProperty())
        return;

    QtVariantProperty* group = variant_manager_->addProperty(QtVariantPropertyManager::groupTypeId(), group_name);
    for (auto prop = type->firstProperty(); prop != type->lastProperty(); ++prop)
    {
        auto type_id(prop->second->getType());
        QString prop_name(tr(prop->second->getName()));
        QString prop_type(tr(prop->second->getTypeKeyword()));
        QString prop_value(tr(prop->second->toString(node_)));
        bool readonly = prop->second->isReadOnly();

        addProperty(group, prop_name, prop_type, prop_value, readonly);
    }
    property_browser_->addProperty(group);
}

void PropertyView::addProperty(QtProperty* group,
    const QString& prop_name,
    const QString& prop_type,
    const QString& prop_value,
    bool readonly)
{
    QtVariantProperty* property_item = nullptr;
    if (prop_type == "bool") {
        property_item = variant_manager_->addProperty(QVariant::Bool, prop_name);
        property_item->setValue((prop_value == "true") ? true : false);
    }
    else if (prop_type.contains("int")) {
        property_item = variant_manager_->addProperty(QVariant::Int, prop_name);
        property_item->setValue(prop_value.toInt());
        if (prop_type.contains("uint")) {
            // property_item 의 범위를 0 이상으로 제한
            variant_manager_->setAttribute(property_item, "minimum", 0);  // 최소값을 0으로 설정
        }
    }
    else if (prop_type.contains("float")) { // 소숫점 형태는 텍스로 표현한다
        property_item = variant_manager_->addProperty(QVariant::String, prop_name);
        property_item->setValue(prop_value);
        //variant_manager_->setAttribute(property_item, "decimals", 10); // 소수점 10자리까지 표시
    }
    else if (prop_type == "string" || prop_type == "uri") {
        property_item = variant_manager_->addProperty(QVariant::String, prop_name);
        property_item->setValue(prop_value);

        if (prop_name == "name") {
            // 헤더노드를 제외한 모든 노드의 이름은 변경 불가
            if (!editable_name_) {
                readonly = true;
            }
        }
    }
    else if (prop_type == "color") {
        property_item = variant_manager_->addProperty(QVariant::Color, prop_name);
        auto rgba = prop_value.split(",");
        if (rgba.size() != 4) return;

        QColor color(rgba.at(0).toInt(), rgba.at(1).toInt(), rgba.at(2).toInt(), rgba.at(3).toInt());
        variant_manager_->setValue(property_item, color);
    }
    else if (prop_type == "vector2") {
        property_item = variant_manager_->addProperty(QVariant::PointF, prop_name);
        auto xy = prop_value.split(",");
        if (xy.size() != 2) return;

        QPointF pt(xy.at(0).toDouble(), xy.at(1).toDouble());
        variant_manager_->setValue(property_item, pt);
    }
    else if (prop_type == "dataset") {
        // do nothing
    }
    else if (prop_type == "GeomHeadNode") {
        String path(prop_value.toStdString().c_str());
        Kernel* kernel = Kernel::instance();
        GeomHeadNode* node = dynamic_cast<GeomHeadNode*>(kernel->lookup(path));

        property_item = variant_manager_->addProperty(QtVariantPropertyManager::enumTypeId(), prop_name);

        // Enum 목록 설정
        variant_manager_->setAttribute(property_item, "enumNames", enum_geoms_);

        if (node) {
            // 초기 선택값 설정
            property_item->setValue(map_geom_[node->getAbsolutePath()]);
        }
        else {
            property_item->setValue(0);
        }
    }
    else if (prop_type == "CSNode") {
        String path(prop_value.toStdString().c_str());
        Kernel* kernel = Kernel::instance();
        CSNode* node = dynamic_cast<CSNode*>(kernel->lookup(path));

        property_item = variant_manager_->addProperty(QtVariantPropertyManager::enumTypeId(), prop_name);

        // Enum 목록 설정
        variant_manager_->setAttribute(property_item, "enumNames", enum_CSs_);

        if (node) {
            // 초기 선택값 설정
            property_item->setValue(map_CS_[node->getAbsolutePath()]);
        }
        else {
            property_item->setValue(0);
        }
    }
    else if (prop_type == "MaterialNode") {
        String path(prop_value.toStdString().c_str());
        Kernel* kernel = Kernel::instance();
        MaterialNode* node = dynamic_cast<MaterialNode*>(kernel->lookup(path));

        property_item = variant_manager_->addProperty(QtVariantPropertyManager::enumTypeId(), prop_name);

        // Enum 목록 설정
        variant_manager_->setAttribute(property_item, "enumNames", enum_materials_);

        if (node) {
            // 초기 선택값 설정
            property_item->setValue(map_material_[node->getAbsolutePath()]);
        }
        else {
            property_item->setValue(0);
        }
    }
    else if (prop_type == "MasterPeriodicBCNode") {
        String path(prop_value.toStdString().c_str());
        Kernel* kernel = Kernel::instance();
        MasterPeriodicBCNode* node = dynamic_cast<MasterPeriodicBCNode*>(kernel->lookup(path));

        property_item = variant_manager_->addProperty(QtVariantPropertyManager::enumTypeId(), prop_name);

        // Enum 목록 설정
        variant_manager_->setAttribute(property_item, "enumNames", enum_master_);

        if (node) {
            // 초기 선택값 설정
            property_item->setValue(map_master_[node->getAbsolutePath()]);
        }
        else {
            property_item->setValue(0);
        }
    }
    else if (prop_type == "dataset") {
        // do nothing
        // 별도 다이얼로그로 처리
    }
    else {
        // GeomSpiltNode 의 Plane 속성은 특별처리
        GeomSplitNode* spilt = dynamic_cast<GeomSplitNode*>(node_);
        if (spilt && prop_name == "Plane") {
            property_item = variant_manager_->addProperty(QtVariantPropertyManager::enumTypeId(), prop_name);
            //if (!GeomSpiltNode::TYPE.hasBindedProperty())
            //    spilt->bindProperty();
            typedef EnumProperty<GeomSplitNode::SPLIT_PLANE> SplitEnumProperty;
            SplitEnumProperty* property =
                static_cast<SplitEnumProperty*>
                (spilt->TYPE.findProperty(prop_name.toStdString().c_str()));
            QStringList enum_prop;
            int selected_enum = 0;
            for (auto it = property->firstEnumerator(); it != property->lastEnumerator(); ++it) {
                QString enum_name(it->second);
                enum_prop.push_back(enum_name);
                if (spilt->getPlane() == it->first) {
                    selected_enum = enum_prop.size() - 1;
                }
            }
            variant_manager_->setAttribute(property_item, "enumNames", enum_prop);
            // 초기 선택값 설정
            property_item->setValue(selected_enum);
        }
        else {
            spdlog::warn("Property type '{}' is not supported yet!", prop_type.toStdString());
        }
    }

    readonly = (readonly || modeler_->isModelerLocked());
    if (property_item) {
        //property_item->setToolTip(description);
        property_item->setAttribute("readOnly", readonly);
        group->addSubProperty(property_item);
    }
}

void PropertyView::makeObjectEnumProperty()
{
    enum_CSs_.clear();
    map_CS_.clear();
    reverse_map_CS_.clear();

    enum_materials_.clear();
    map_material_.clear();
    reverse_map_material_.clear();

    enum_geoms_.clear();
    map_geom_.clear();
    reverse_map_geom_.clear();

    enum_mb_.clear();
    map_mb_.clear();
    reverse_map_mb_.clear();

    enum_master_.clear();
    map_master_.clear();
    reverse_map_master_.clear();
    
    Node* coord_root = modeler_->getWorkingCoordRootNode();
    Node* material_root = modeler_->getWorkingMaterialRootNode();
    Node* geom_root = modeler_->getWorkingGeomRootNode();
    Node* bc_root = modeler_->getWorkingBCRootNode();

    enum_CSs_.push_back("Undefined");
    extractNodeName(coord_root, enum_CSs_, map_CS_, reverse_map_CS_, true);

    enum_materials_.push_back("Undefined");
    extractNodeName(material_root, enum_materials_, map_material_, reverse_map_material_, false);

    enum_geoms_.push_back("Undefined");
    extractNodeName(geom_root, enum_geoms_, map_geom_, reverse_map_geom_, false);

    enum_mb_.push_back("Undefined");
    enum_master_.push_back("Undefined");
    extractBCName(bc_root);
}

bool PropertyView::extractNodeName(Node* root, QStringList& enum_strings,
    std::map<String, int>& map,
    std::map<int, String>& reverse_map, bool with_child)
{
    if (!root) return false;

    for (auto it = root->firstChildNode(); it != root->lastChildNode(); ++it) {
        enum_strings << QString(tr((*it)->getName().c_str()));
        map[(*it)->getAbsolutePath()] = enum_strings.size() - 1;
        reverse_map[enum_strings.size() - 1] = (*it)->getAbsolutePath();

        if(with_child) {
            while (extractNodeName(*it, enum_strings, map, reverse_map, with_child)) {
                return true;
            }
        }
    }
    return false;
}

bool PropertyView::extractBCName(Node* root)
{
    if (!root) return false;

    for (auto it = root->firstChildNode(); it != root->lastChildNode(); ++it) {
        Node* node = *it;
        String type_name = node->getType()->getName();
        if (type_name.contains("MasterPeriodicBCNode")) {
            enum_master_ << QString(tr(node->getName().c_str()));
            map_master_[node->getAbsolutePath()] = enum_master_.size() - 1;
            reverse_map_master_[enum_master_.size() - 1] = node->getAbsolutePath();
        }
        else if (type_name.contains("MovingBandBCNode")) {
            enum_mb_ << QString(tr(node->getName().c_str()));
            map_mb_[node->getAbsolutePath()] = enum_mb_.size() - 1;
            reverse_map_mb_[enum_mb_.size() - 1] = node->getAbsolutePath();
        }
    }
    return true;
}

void PropertyView::valueChanged(QtProperty* property, const QVariant& new_value)
{
    if (flag_reset_) return;

    // 모델러가 잠겨있으면 변경이 안됨
    if (modeler_->isModelerLocked()) return;

    QString prop_name = property->propertyName();
    Type* type = node_->getType();
    Property* prop = type->findProperty(prop_name.toStdString());
    if (prop == nullptr) return;

    QString prop_value_old(tr(prop->toString(node_)));

    QString prop_type(tr(prop->getTypeKeyword()));
    bool readonly = prop->isReadOnly();

    if (readonly || modeler_->isModelerLocked()) {
        spdlog::warn("Update failed! Property type '{}' is readonly!", prop_type.toStdString());
    }
    else if (prop_type == "bool") {
        typedef SimpleProperty<bool> BoolProperty;
        BoolProperty* property = static_cast<BoolProperty*>(prop);
        property->set(node_, new_value.toBool());
    }
    else if (prop_type.contains("int")) {
        if (prop_type.contains("uint")) {
            typedef SimpleProperty<unsigned int> UIntProperty;
            UIntProperty* property = static_cast<UIntProperty*>(prop);
            property->set(node_, new_value.toUInt());
        }
        else {
            typedef SimpleProperty<int> IntProperty;
            IntProperty* property = static_cast<IntProperty*>(prop);
            property->set(node_, new_value.toInt());
        }
    } 
    else if (prop_type.contains("float")) {
        typedef SimpleProperty<double> DoubleProperty;
        DoubleProperty* property = static_cast<DoubleProperty*>(prop);
        bool ok;
        double value = new_value.toDouble(&ok);
        if (ok) {
            property->set(node_, new_value.toDouble());
        }
    }
    else if (prop_type == "string") {
        if (prop_name == "name") {
            if (!editable_name_) return;
        }
        typedef SimpleProperty<const String&> StringProperty;
        StringProperty* property = static_cast<StringProperty*>(prop);
        property->set(node_, new_value.toString().toStdString());
    }
    else if (prop_type == "color") {
        typedef SimpleProperty<const Color&> ColorProperty;
        ColorProperty* property = static_cast<ColorProperty*>(prop);
        QColor value = new_value.value<QColor>();
        const Color color(value.red(), value.green(), value.blue(), value.alpha());
        property->set(node_, color);
    }
    else if (prop_type == "vector2") {
        typedef SimpleProperty<const Vector2&> PointProperty;
        PointProperty* property = static_cast<PointProperty*>(prop);
        QPointF value = new_value.toPointF();
        const Vector2 point(value.x(), value.y());
        property->set(node_, point);
    }
    else if (prop_type == "dataset") {
        // do nothing
    }
    else if (prop_type == "GeomHeadNode") {
        typedef SimpleProperty<Node*> NodeProperty;
        NodeProperty* property = static_cast<NodeProperty*>(prop);
        Kernel* kernel = Kernel::instance();
        if (reverse_map_geom_.find(new_value.toInt()) == reverse_map_geom_.end()) {
            property->set(node_, nullptr);
        }
        else {
            Node* new_node = kernel->lookup(reverse_map_geom_[new_value.toInt()]);
            property->set(node_, new_node);
        }
    }
    else if (prop_type == "CSNode") {
        typedef SimpleProperty<Node*> NodeProperty;
        NodeProperty* property = static_cast<NodeProperty*>(prop);
        Kernel* kernel = Kernel::instance();
        if (reverse_map_CS_.find(new_value.toInt()) == reverse_map_CS_.end()) {
            property->set(node_, nullptr);
        }
        else {
            Node* new_node = kernel->lookup(reverse_map_CS_[new_value.toInt()]);
            property->set(node_, new_node);
        }
    }
    else if (prop_type == "MaterialNode") {
        typedef SimpleProperty<Node*> NodeProperty;
        NodeProperty* property = static_cast<NodeProperty*>(prop);
        Kernel* kernel = Kernel::instance();
        if (reverse_map_material_.find(new_value.toInt()) == reverse_map_material_.end()) {
            property->set(node_, nullptr);
        }
        else {
            Node* new_node = kernel->lookup(reverse_map_material_[new_value.toInt()]);
            property->set(node_, new_node);
        }
    }
    else if (prop_type == "MasterPeriodicBCNode") {
        typedef SimpleProperty<Node*> NodeProperty;
        NodeProperty* property = static_cast<NodeProperty*>(prop);
        Kernel* kernel = Kernel::instance();
        if (reverse_map_master_.find(new_value.toInt()) == reverse_map_master_.end()) {
            property->set(node_, nullptr);
        }
        else {
            Node* new_node = kernel->lookup(reverse_map_master_[new_value.toInt()]);
            property->set(node_, new_node);
        }
    }
    else {
        // GeomSpiltNode 의 Plane 속성은 특별처리
        GeomSplitNode* spilt = dynamic_cast<GeomSplitNode*>(node_);
        if (spilt && prop_name == "Plane") {
            GeomSplitNode* spilt = dynamic_cast<GeomSplitNode*>(node_);
            if (spilt) {
                typedef EnumProperty<GeomSplitNode::SPLIT_PLANE> SplitEnumProperty;
                SplitEnumProperty* property = static_cast<SplitEnumProperty*>(prop);

                auto it = property->firstEnumerator();
                std::advance(it, new_value.toInt());
                property->set(spilt, it->first);

                // enum타입의 속성은 바뀌었는지 스트링으로 판별할 수 없기에
                // 제체적으로 시그널 생성해서 통지
                emit propertyChanged(node_);
            }
        }
        else {
            spdlog::warn("Update failed! Property type '{}' is not supported yet!", prop_type.toStdString());
        }
    }

    // 속성이 변했으면 변경 시그널 생성
    QString prop_value_new(tr(prop->toString(node_)));
    if (prop_value_new != prop_value_old) {
        emit propertyChanged(node_);
    }
}

void PropertyView::setReadOnly(bool read_only)
{
    if (read_only) {
        property_browser_->unsetFactoryForManager(variant_manager_);
        //property_browser_->setEnabled(false); // 전체 비활성화
    }
    else {
        property_browser_->setFactoryForManager(variant_manager_, editor_factory_);
        //property_browser_->setEnabled(true); // 전체 활성화
    }

    property_browser_->update();
}