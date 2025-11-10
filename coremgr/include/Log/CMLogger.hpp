#ifndef CM_LOGGER_HPP_
#define CM_LOGGER_HPP_

#include "Logger.hpp"

#define CM_LOG(level, fmt, ...) \
    CMLogger::getInstance()->printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

class CMLogger : public Logger {
    public:
        static CMLogger *getInstance(){
            static CMLogger instance;
            return &instance;
        }
        CMLogger(const CMLogger &) = delete;
        CMLogger &operator=(const CMLogger &) = delete;

    private:
        CMLogger() : Logger("CoreManager") {}
        ~CMLogger() override = default;
};

#endif // CM_LOGGER_HPP_