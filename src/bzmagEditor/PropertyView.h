#pragma once

#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMetaProperty>
#include <qgroupbox.h>

#include "Modeler.h"
#include "QtTreePropertyBrowser.h"
#include <QtVariantProperty.h>
//#include "bzMagVariantEditorFactory.h"
//#include "bzMagVariantPropertyManager.h"

#include "core/node.h"

class PropertyView : public QWidget {
    Q_OBJECT
public:
    PropertyView(QWidget* parent = nullptr);

    void addGroupProperty(const bzmag::Type* type);
    void addProperty(QtProperty* group,
        const QString& prop_name,
        const QString& prop_type,
        const QString& prop_value,
        bool readonly);

public slots:
    void displayProperties(bzmag::Node* node);
    void setReadOnly(bool read_only);

signals:
    void propertyChanged(bzmag::Node* node);  // 객체 속성이 변했을때 발생하는 시그널

private:
    void makeObjectEnumProperty();
    bool extractNodeName(bzmag::Node* root, QStringList& enum_strings,
        std::map<bzmag::String, int>& map,
        std::map<int, bzmag::String>& reverse_map, bool with_child = true);
    bool extractBCName(bzmag::Node* root);

private slots:
    void valueChanged(QtProperty* property, const QVariant& new_value);

private:
    Modeler* modeler_;
    QVBoxLayout* vbox_;
    bzmag::Node* node_;
    bool editable_name_;

    QtTreePropertyBrowser* property_browser_;
    QtVariantPropertyManager* variant_manager_;
    QtVariantEditorFactory* editor_factory_;

    QStringList enum_geoms_;
    std::map<bzmag::String, int> map_geom_;
    std::map<int, bzmag::String> reverse_map_geom_;

    QStringList enum_CSs_;
    std::map<bzmag::String, int> map_CS_;
    std::map<int, bzmag::String> reverse_map_CS_;
    
    QStringList enum_materials_;
    std::map<bzmag::String, int> map_material_;
    std::map<int, bzmag::String> reverse_map_material_;

    QStringList enum_mb_;
    std::map<bzmag::String, int> map_mb_;
    std::map<int, bzmag::String> reverse_map_mb_;

    QStringList enum_master_;
    std::map<bzmag::String, int> map_master_;
    std::map<int, bzmag::String> reverse_map_master_;
    
    bool flag_reset_;
};