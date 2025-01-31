#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <regex>
#include "exprtk.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::ordered_json;

class ConstraintEvaluator {
public:
    ConstraintEvaluator(const json& data) : data_(data) {}
    bool evaluateConstraints(const std::string& group_key, const std::string& item_key);

private:
    const json& data_;
};