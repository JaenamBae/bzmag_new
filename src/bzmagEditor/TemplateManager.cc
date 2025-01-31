#include "TemplateManager.h"
#include <fstream>
#include <sstream>
#include <vector>
#include "core/tokenizer.h"
#include "ConstraintEvaluator.h"

// 생성자
TemplateManager::TemplateManager(const std::string& file_path) : version_(1.0), file_path_(file_path)
{

}

// JSON 파일 로드 함수
bool TemplateManager::load()
{
    load_succeed_ = false;

    std::ifstream file(file_path_);
    if (!file.is_open()) {
        return false;  // 파일 열기 실패
    }
    try {
        file >> json_doc_;
    }
    catch (const json::parse_error&) {
        return false;  // 파싱 오류
    }

    // 파일 검증
    if (json_doc_["file_type"] != "bzmag_template") {
        throw std::runtime_error("Invalid file type. Expected 'bzmag'.");
        return false;
    }

    // 버전 확인
    if (json_doc_.contains("version") && json_doc_["version"].is_number()) {
        version_ = json_doc_["version"];
    }
    else {
        throw std::runtime_error("Version information missing or invalid.");
    }

    // 템플릿 데이터가 있는지 판단
    if (!json_doc_.contains("template") || !json_doc_["template"].is_object()) {
        throw std::runtime_error("Invalid file structure. Expected 'template' section.");
        return false;
    }

    if (!json_doc_["template"].contains("name") || !json_doc_["template"]["name"].is_string()) {
        throw std::runtime_error("Invalid file structure. Expected 'configure' field.");
        return false;
    }
    template_name_ = json_doc_["template"]["name"];

    if (!json_doc_["template"].contains("configure") || !json_doc_["template"]["configure"].is_object()) {
        throw std::runtime_error("Invalid file structure. Expected 'configure' field.");
        return false;
    }

    // 빌드 데이터가 있는지 판단
    if (!json_doc_.contains("build") || !json_doc_["build"].is_object()) {
        throw std::runtime_error("Invalid file structure. Expected 'build' section.");
        return false;
    }

    // 컨피규어 항목의 temp_value 값을 초기화 한다
    // 이제 모든 항목은 temp_value 값을 가진다
    fillConfigure();

    load_succeed_ = true;
    return load_succeed_;
}

// JSON 파일 저장 함수
bool TemplateManager::save(const std::string& file_path, int indent) const
{
    std::string path = file_path.empty() ? file_path_ : file_path;
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;  // 파일 열기 실패
    }

    file << json_doc_.dump(indent);  // JSON을 저장
    return true;
}

// JSON 데이터 접근 함수
json TemplateManager::getValue(const std::string& path) const
{
    if (!load_succeed_) return json();

    const json* current = &json_doc_["template"]["configure"];
    bzmag::Tokenizer keys(path.c_str(), '/');
    for (const auto& key : keys) {
        if (current->contains(key)) {
            current = &(*current)[key]; // 다음 레벨로 이동
        }
        else {
            return json(); // 경로가 존재하지 않으면 빈 JSON 반환
        }
    }
    return *current; // 경로 끝까지 도달한 값을 반환
}

// JSON 데이터 수정 함수
void TemplateManager::setValue(const std::string& path, const json& value)
{
    if (!load_succeed_) return;

    json* current = &json_doc_["template"]["configure"];
    bzmag::Tokenizer keys(path.c_str(), '/');
    for (size_t i = 0; i < keys.size(); ++i) {
        const std::string key(keys[i].c_str());
        if (i == keys.size() - 1) {
            (*current)[key] = value;
        }
        else {
            current = &(*current)[key];
        }
    }
}

bool TemplateManager::checkConstraints()
{
    json& build_object = json_doc_["template"]["configure"];

    // 그룹별로 제약조건 체크
    ConstraintEvaluator check(build_object);
    for (auto& [group, section] : build_object.items()) {
        for (auto& [key, item] : section.items()) {
            if (!check.evaluateConstraints(group, key)) {
                return false;
            }
        }
    }

    // 제약조건 테스트 성공하면 임시 저장소의 값을 실제로 기록
    for (auto& [group, section] : build_object.items()) {
        for (auto& [key, item] : section.items()) {
            item["value"] = json(item["temp_value"]);
        }
    }

    return true;
}

json TemplateManager::getBuildObject() const
{
    if (!load_succeed_) return json();
    return json_doc_["build"];
}

json TemplateManager::getConfigureObject() const
{
    if (!load_succeed_) return json();
    return json_doc_["template"]["configure"];
}

// JSON의 루트 객체 반환
json TemplateManager::getRootObject() const
{
    return json_doc_;
}

json TemplateManager::buildExpressionsFromConfigure(bool with_default)
{
    const json& build_object = json_doc_["template"]["configure"];
    json configure_variables = json::object({ {"children", json::array()} });

    for (auto& [group, section] : build_object.items()) {
        for (auto& [key, item] : section.items()) {
            std::string dtype = item["dtype"];
            json value;
            if (item.contains("default_value")) {
                value = item["default_value"];
            }

            if (!with_default) {
                if (item.contains("value")) {
                    value = item["value"];
                }
            }

            std::string str_value = "0";
            if (dtype == "bool") {
                if (value.is_boolean()) {
                    str_value = value.get<bool>() ? "true" : "false";
                }
            }
            else if (dtype == "int") {
                if (value.is_number()) {
                    str_value = std::to_string(value.get<int>());
                }
            }
            else if (dtype == "float") {
                if (value.is_number()) {
                    str_value = std::to_string(value.get<float>());
                }
            }
            else if (dtype == "string") {
                if (value.is_string()) {
                    str_value = value.get<std::string>();
                }
            }

            json expr;
            expr["Key"] = key;
            expr["Expression"] = str_value;
            expr["Comment"] = std::string(item["description"]) + " [" + std::string(item["unit"]) + "]";

            configure_variables["children"].push_back(expr);
        }
    }

    return configure_variables;
}

json TemplateManager::extractValues(const json& source) {
    json result;

    if (source.is_object()) {
        for (const auto& [key, value] : source.items()) {
            if (value.is_object()) {
                // 재귀적으로 처리
                result[key] = extractValues(value);
            }
            else if (key == "temp_value" || key == "value") {
                // "value"와 "temp_value"만 복사
                result[key] = value;
            }
        }
    }

    return result;
}

void TemplateManager::fillConfigure()
{
    json& build_object = json_doc_["template"]["configure"];
    for (auto& [group, section] : build_object.items()) {
        for (auto& [key, item] : section.items()) {
            std::string dtype = item["dtype"];
            if (!item.contains("temp_value")) {
                // default값이 있으면 그걸로, 없으면 0으로 temp 초기값을 채움
                // 데이터 타입별로 설정함
                if (dtype == "bool") {
                    if (item.contains("default_value")) {
                        item["temp_value"] = item["default_value"];
                    }
                    else {
                        item["temp_value"] = false;
                    }
                }
                else if (dtype == "int") {
                    if (item.contains("default_value")) {
                        item["temp_value"] = item["default_value"];
                    }
                    else {
                        item["temp_value"] = 0;
                    }
                }
                else if (dtype == "float") {
                    if (item.contains("default_value")) {
                        item["temp_value"] = item["default_value"];
                    }
                    else {
                        item["temp_value"] = 0.0;
                    }
                }
                else if (dtype == "string") {
                    if (item.contains("default_value")) {
                        item["temp_value"] = item["default_value"];
                    }
                    else {
                        item["temp_value"] = "0";
                    }
                }
            }
        }
    }
}

bool TemplateManager::saveCurrentData(BzmagFileManager& result)
{
    json template_section = json_doc_["template"];

    json configure_result = extractValues(template_section["configure"]);
    template_section["configure"] = configure_result;

    result.setSectionData("template", template_section);
    return true;
}

bool TemplateManager::loadSavedData(const BzmagFileManager& saved)
{
    json file_type = saved.getSectionData("file_type");
    if (!file_type.is_string() || file_type != "bzmag") {
        return false;
    }

    json version = saved.getSectionData("version");
    if (!version.is_number() || version != 1.0) {
        return false;
    }

    json template_section = saved.getSectionData("template");
    if (!template_section.is_object()) {
        return false;
    }

    if (!template_section.contains("name") || !template_section["name"].is_string()) {
        return false;
    }
    if (template_name_ != template_section["name"]) {
        return false;
    }

    if (!template_section.contains("configure") || !template_section["configure"].is_object()) {
        return false;
    }

    json& my_template_section = json_doc_["template"];

    // my_template_section 기준으로 temp_value와 value 복사
    for (auto& [group, section] : my_template_section["configure"].items()) {
        if (!template_section["configure"].contains(group)) {
            continue; // template_section에 해당 group이 없으면 건너뜀
        }

        const auto& template_group = template_section["configure"][group];

        for (auto& [key, my_item] : section.items()) {
            if (!template_group.contains(key)) {
                continue; // template_group에 해당 key가 없으면 건너뜀
            }

            const auto& template_item = template_group[key];

            // temp_value 복사
            if (template_item.contains("temp_value") && 
                !template_item["temp_value"].is_null() &&
                !template_item["temp_value"].empty()) {
                my_item["temp_value"] = template_item["temp_value"];
            }

            // value 복사
            if (template_item.contains("value") && 
                !template_item["value"].is_null() &&
                !template_item["value"].empty()) {
                my_item["value"] = template_item["value"];
            }
        }
    }

    return true;
}
