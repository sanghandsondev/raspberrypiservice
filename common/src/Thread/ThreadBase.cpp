#include "ThreadBase.hpp"
#include "Logger.hpp"

ThreadBase::ThreadBase(std::string threadName) : threadName_(threadName), runningFlag_(false) {
    CMN_LOG(INFO, "%s thread is created", threadName_.c_str());
}

ThreadBase::~ThreadBase() {
    CMN_LOG(INFO, "%s thread is destroyed", threadName_.c_str());
}

void ThreadBase::run() {
    if (!runningFlag_) {
        runningFlag_ = true;
        try {
            threadObj_ = std::thread(&ThreadBase::threadFunction, this);
            CMN_LOG(INFO, "%s thread start SUCCESS", threadName_.c_str());
        } catch (const std::exception& e) {
            CMN_LOG(ERROR, "%s thread start FAILED: %s", threadName_.c_str(), e.what());
            runningFlag_ = false;
        }
    } else {
        CMN_LOG(WARN, "%s thread is already running", threadName_.c_str());
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
        CMN_LOG(INFO, "%s thread is stopping", threadName_.c_str());
    } else {
        CMN_LOG(WARN, "%s thread is not running", threadName_.c_str());
    }
    onStop();
}

