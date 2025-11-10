#include "ThreadBase.hpp"
#include "Logger.hpp"

ThreadBase::ThreadBase(std::string threadName) : threadName_(threadName), runningFlag_(false) {
    R_LOG(INFO, "%s thread is created", threadName_.c_str());
}

ThreadBase::~ThreadBase() {
    R_LOG(INFO, "%s thread is destroyed", threadName_.c_str());
}

void ThreadBase::run() {
    if (!runningFlag_) {
        runningFlag_ = true;
        try {
            threadObj_ = std::thread(&ThreadBase::threadFunction, this);
            R_LOG(INFO, "%s thread start SUCCESS", threadName_.c_str());
        } catch (const std::exception& e) {
            R_LOG(ERROR, "%s thread start FAILED: %s", threadName_.c_str(), e.what());
            runningFlag_ = false;
        }
    } else {
        R_LOG(WARN, "%s thread is already running", threadName_.c_str());
    }
}

void ThreadBase::join() {
    if (threadObj_.joinable()) {
        threadObj_.join();
    }
}

void ThreadBase::stop() {
    if (runningFlag_) {
        runningFlag_ = false;
        R_LOG(INFO, "%s thread is stopping", threadName_.c_str());
    } else {
        R_LOG(WARN, "%s thread is not running", threadName_.c_str());
    }
}

