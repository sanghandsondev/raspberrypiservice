#ifndef RM_LOGGER_HPP_
#define RM_LOGGER_HPP_

#include "Logger.hpp"

#define RM_LOG(level, fmt, ...) \
    RMLogger::getInstance()->printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

class RMLogger : public Logger {
    public:
        static RMLogger *getInstance(){
            static RMLogger instance;
            return &instance;
        }
        RMLogger(const RMLogger &) = delete;
        RMLogger &operator=(const RMLogger &) = delete;

    private:
        RMLogger() : Logger("RecordManager") {}
        ~RMLogger() override = default;
};

#endif // RM_LOGGER_HPP_