#include "JsonHelper.hpp"
#include "RLogger.hpp"

std::optional<int> JsonHelper::getIntField(const json& data, const std::string& key) {
    if (data.contains(key)) {
        const auto& value = data[key];
        if (value.is_number()) {
            return value.get<int>();
        } else {
            R_LOG(WARN, "Field '%s' is not a number. It is: %s", key.c_str(), value.dump().c_str());
        }
    } else {
        R_LOG(WARN, "JSON data does not contain key '%s'", key.c_str());
    }
    return std::nullopt;
}