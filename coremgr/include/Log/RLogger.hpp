#ifndef CM_LOGGER_HPP_
#define CM_LOGGER_HPP_

#include "Logger.hpp"

#define CM_LOG(level, fmt, ...) \
    RLogger::getInstance()->printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

class RLogger : public Logger {
    public:
        static RLogger *getInstance(){
            static RLogger instance;
            return &instance;
        }
        RLogger(const RLogger &) = delete;
        RLogger &operator=(const RLogger &) = delete;

    private:
        RLogger() : Logger("CoreManager") {}
        ~RLogger() override = default;
};

#endif // CM_LOGGER_HPP_