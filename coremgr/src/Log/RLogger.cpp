#include "RLogger.hpp"
#include <cstdarg>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h> // getpid()

const char* RLogger::levelToString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default: break;
    }
    return "DEBUG";
}

void RLogger::printLog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(m_logMutex); // Đảm bảo thread-safety

    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream timeStream;
    timeStream << std::put_time(std::localtime(&now_time_t), "%H:%M:%S")
               << '.' << std::setfill('0') << std::setw(3) << ms.count();

    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    std::string message;
    if (len >= 0) {
        std::vector<char> buffer(len + 1);
        vsnprintf(buffer.data(), buffer.size(), fmt, args);
        message = buffer.data();
    }
    va_end(args);

    const char* short_file = strrchr(file, '/');
    if (short_file) {
        short_file++;
    } else {
        short_file = file;
    }

    printf("[CoreManager:%d][%s][%s:%d][%s] [%s]%s\n",
        getpid(),
        timeStream.str().c_str(),
        short_file,
        line,
        func,
        levelToString(level),
        message.c_str());
    fflush(stdout); // Đảm bảo log được ghi ra ngay lập tức
}