#ifndef HM_LOGGER_HPP_
#define HM_LOGGER_HPP_

#include "Logger.hpp"

#define HM_LOG(level, fmt, ...) \
    HMLogger::getInstance()->printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

class HMLogger : public Logger {
    public:
        static HMLogger *getInstance(){
            static HMLogger instance;
            return &instance;
        }
        HMLogger(const HMLogger &) = delete;
        HMLogger &operator=(const HMLogger &) = delete;

    private:
        HMLogger() : Logger("HardwareManager") {}
        ~HMLogger() override = default;
};

#endif // HM_LOGGER_HPP_