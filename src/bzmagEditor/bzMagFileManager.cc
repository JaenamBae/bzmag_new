#include "bzMagFileManager.h"
#include "core/string.h"

BzmagFileManager::BzmagFileManager() : version_(1.0), is_template_(false) {
    // 기본 데이터 구조 생성
    data_ = {
        {"file_type", "bzmag"},
        {"version", version_},
        {"template", nullptr},
        {"expression", {{"children", json::array()}}},
        {"geometry", {{"children", json::array()}}},
        {"coordinate_system", {{"children", json::array()}}},
        {"material", {{"children", json::array()}}},
        {"boundary_condition", {{"children", json::array()}}},
        {"excitation", {{"children", json::array()}}},
        {"setup", {{"children", json::array()}} }
    };
}

void BzmagFileManager::loadFromFile(const QString& filename) {
    bzmag::String temp(filename.toStdWString().c_str());
    std::string name(temp.c_str());
    std::ifstream file(name);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + name);
    }

    data_.clear();
    file >> data_;

    // 파일 검증
    if (data_["file_type"] != "bzmag") {
        throw std::runtime_error("Invalid file type. Expected 'bzmag'.");
    }

    // 버전 확인
    if (data_.contains("version") && data_["version"].is_number()) {
        version_ = data_["version"];
    }
    else {
        throw std::runtime_error("Version information missing or invalid.");
    }

    // 템플릿 여부 확인
    is_template_ = data_.contains("template") && data_["template"].is_object();
}

void BzmagFileManager::saveToFile(const QString& filename) const {
    bzmag::String temp(filename.toStdWString().c_str());
    std::string name(temp.c_str());
    std::ofstream file(name);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + name);
    }

    file << data_.dump(2); // Pretty print with 2 spaces
}

double BzmagFileManager::getVersion() const {
    return version_;
}

bool BzmagFileManager::isTemplate() const {
    return is_template_;
}

json BzmagFileManager::getSectionData(const std::string& section) const {
    if (!data_.contains(section)) {
        throw std::runtime_error("Section not found: " + section);
    }
    return data_[section];
}

void BzmagFileManager::setSectionData(const std::string& section, const json& newData) {
    if (!data_.contains(section)) {
        throw std::runtime_error("Section not found: " + section);
    }
    data_[section] = newData;
}

void BzmagFileManager::addTreeObject(const json& value, const std::string& section) {
    if (is_template_) {
        throw std::runtime_error("Cannot add objects in template mode.");
    }

    if (data_.contains(section)) {
        data_[section]["children"].push_back(value);
    }
    else {
        throw std::runtime_error("Section not found: " + section);
    }
}

void BzmagFileManager::printData() const {
    std::cout << data_.dump(2) << std::endl;
}
