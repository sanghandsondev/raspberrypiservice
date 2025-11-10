#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include "Define.hpp"
#include <string>
#include <mutex>

#define R_LOG(level, fmt, ...) \
    Logger("Common").printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

class Logger {
    public:
        explicit Logger(std::string moduleName);
        virtual ~Logger() = default;
        
        void printLog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...);

    private:
        const char* levelToString(LogLevel level);

        std::string m_moduleName;
        std::mutex m_logMutex; // Mutex để đảm bảo thread-safety
};

#endif // LOGGER_HPP_