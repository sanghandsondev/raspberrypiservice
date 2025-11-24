#ifndef JSON_HELPER_HPP_
#define JSON_HELPER_HPP_

#include "json.hpp"     // nlohmann::json
#include <string>
#include <optional>

using json = nlohmann::json;

#define JSON_HELPER_INSTANCE() JsonHelper::getInstance()

class JsonHelper {
    public:
        static JsonHelper *getInstance() {
            static JsonHelper instance;
            return &instance;
        }
        JsonHelper(const JsonHelper &) = delete;
        JsonHelper &operator=(const JsonHelper &) = delete;

        std::optional<int> getIntField(const json& data, const std::string& key);

    private:
        JsonHelper() = default;
        ~JsonHelper() = default;
};

#endif // JSON_HELPER_HPP_