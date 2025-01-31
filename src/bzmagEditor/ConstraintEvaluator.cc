#include "ConstraintEvaluator.h"
#include "spdlog/spdlog.h"

bool ConstraintEvaluator::evaluateConstraints(const std::string& group_key, const std::string& item_key)
{
    const json& item = data_[group_key][item_key];
    std::string dtype = item["dtype"].get<std::string>();
    double this_value = item["temp_value"].get<double>();

    // "constraint"가 있는 경우 제약 조건을 평가
    if (item.contains("constraints")) {
        // exprtk 설정
        typedef exprtk::symbol_table<double> symbol_table_t;
        typedef exprtk::expression<double> expression_t;
        typedef exprtk::parser<double> parser_t;

        symbol_table_t symbol_table;
        expression_t expression;
        parser_t parser;

        // 그룹 내 변수들의 값 설정 (유효 범위를 보장하기 위해 변수들을 저장)
        std::map<std::string, double> variables;
        for (const auto& sibling : data_[group_key].items()) {
            std::string var_name = sibling.key();
            double sibling_value = sibling.value()["temp_value"].get<double>();
            variables[var_name] = sibling_value; // 변수 값을 맵에 저장하여 메모리 주소를 유지
            symbol_table.add_variable(var_name, variables[var_name]);
        }
        //symbol_table.add_constants(); // exprtk에서 기본 상수 (예: pi, e) 사용 가능

        // symbol_table을 expression에 등록
        expression.register_symbol_table(symbol_table);

        for (const auto& constraint : item["constraints"]) {
            // 수식을 평가
            try {
                std::string expression_str = constraint.get<std::string>();

                // 표현식에서 '$this'를 'item_key'로 치환
                expression_str = std::regex_replace(expression_str, std::regex(R"(\$this)"), item_key);

                // 표현식에서 '$variable_name'을 'variable_name'으로 치환
                expression_str = std::regex_replace(expression_str, std::regex(R"(\$([A-Za-z_]\w*))"), "$1");

                // exprtk로 수식을 파싱 및 평가
                if (!parser.compile(expression_str, expression)) {
                    spdlog::error("Error compiling expression: {}", parser.error());
                    return false;
                }

                // 평가 결과 확인
                if (expression.value() == 0) {  // 0은 false로 간주
                    spdlog::error("Constraint failed: {}", expression_str);
                    return false;
                }
            }
            catch (const std::exception& e) {
                spdlog::error("Error evaluating expression: {}", e.what());
                return false;
            }
        }
    }
    return true;
}