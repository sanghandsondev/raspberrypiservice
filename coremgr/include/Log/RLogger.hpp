#ifndef RLOGGER_HPP_
#define RLOGGER_HPP_

#include "Define.hpp"
#include <string>
#include <mutex>

#define CM_LOG(level, fmt, ...) \
    RLogger::getInstance()->printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

class RLogger {
    public:
        static RLogger *getInstance(){
            static RLogger instance;
            return &instance;
        }
        RLogger(const RLogger &) = delete;
        RLogger &operator=(const RLogger &) = delete;

        void printLog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...);

    private:
        RLogger() = default;
        ~RLogger() = default;

        const char* levelToString(LogLevel level);

        std::mutex m_logMutex; // Mutex để đảm bảo thread-safety
};

#endif // RLOGGER_HPP_