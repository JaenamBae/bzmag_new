#pragma once

#include <QObject>
#include <string>
#include <nlohmann/json.hpp>
#include "bzMagFileManager.h"

using json = nlohmann::ordered_json;

class TemplateManager : public QObject {
    Q_OBJECT

public:
    explicit TemplateManager(const std::string& file_path);

    bool load();
    bool save(const std::string& file_path = "", int indent = 2) const;

    json getValue(const std::string& path) const;
    void setValue(const std::string& path, const json& value);
    bool checkConstraints();

    json getBuildObject() const;
    json getConfigureObject() const;
    json getRootObject() const;
    const std::string& getTemplateName() const { return template_name_; }

    bool valid() const { return load_succeed_; }
    json buildExpressionsFromConfigure(bool with_default = true);

    bool saveCurrentData(BzmagFileManager& result);
    bool loadSavedData(const BzmagFileManager& saved);

private:
    json extractValues(const json& source);
    void fillConfigure();

signals:
    void templateUpdated();

private:
    std::string file_path_;
    json json_doc_;
    std::string template_name_;
    double version_ = 0;
    bool load_succeed_ = false;
};
