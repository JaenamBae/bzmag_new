#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <qstring.h>

using json = nlohmann::ordered_json;

class BzmagFileManager {
private:
    json data_;
    double version_;
    bool is_template_;

public:
    BzmagFileManager();

    // 파일 입출력
    void loadFromFile(const QString& filename);
    void saveToFile(const QString& filename) const;

    // 버전 및 템플릿 정보
    double getVersion() const;
    bool isTemplate() const;

    // 데이터 읽기 및 설정
    json getSectionData(const std::string& section) const;
    void setSectionData(const std::string& section, const json& newData);

    // 일반 구조에 객체 추가
    void addTreeObject(const json& value, const std::string& section);

    // 데이터 확인용 출력
    void printData() const;
};
