#ifndef RM_LOGGER_HPP_
#define RM_LOGGER_HPP_

#include "Logger.hpp"

#define R_LOG(level, fmt, ...) \
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
        RLogger() : Logger("RecordManager") {}
        ~RLogger() override = default;
};

#endif // RM_LOGGER_HPP_