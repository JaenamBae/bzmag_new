#pragma once

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QtConcurrent>
#include <unordered_map>
#include "TemplateNode.h"
#include "qttreepropertybrowser.h"
#include "qtgroupboxpropertybrowser.h"
#include "qtvariantproperty.h"

class TemplatePropertyDialog : public QDialog {
    Q_OBJECT

public:
    TemplatePropertyDialog(TemplateNode* node, QWidget* parent = nullptr);
    void setReadOnly(bool readonly);

public slots:
    void onNavigatorTextChanged(const QString& group_name);
    void onValueChanged(QtProperty* property, const QVariant& new_value);
    void onPropertyFocused(QtBrowserItem* item);
    void onApplyButtonClicked();


protected:
    void prepairPropertyBrowser();
    void populatePropertyBrowser();
    void loadImageAsync(const std::string& image_path);

private:
    TemplateNode* node_ = nullptr;
    TemplateManager* manager_ = nullptr;
    QComboBox* navigator_ = nullptr;
    std::string current_group_;
    std::unordered_map<std::string, QtVariantProperty*> property_group_;
    QtTreePropertyBrowser* property_browser_ = nullptr;
    QtVariantPropertyManager* variant_manager_ = nullptr;
    QtVariantEditorFactory* editor_factory_ = nullptr;
    std::unordered_map<QtProperty*, std::string> path_map_;

    QLabel* image_label_ = nullptr;
    QPixmap original_pixmap_;
    QFutureWatcher<QPixmap> future_watcher_;
    QCache<QString, QPixmap> pixmap_cache_;

    QPushButton* apply_button_ = nullptr;
    QPushButton* close_button_ = nullptr;

    std::string template_name_;

    bool building_ = false;
};
