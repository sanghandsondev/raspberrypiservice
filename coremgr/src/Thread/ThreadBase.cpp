#include "ThreadBase.hpp"

ThreadBase::ThreadBase(std::string threadName) : threadName_(threadName), runningFlag_(false) {
    printf("[ThreadBase] %s thread is created\n", threadName_.c_str());
}

ThreadBase::~ThreadBase() {
    printf("[ThreadBase] %s thread is destroyed\n", threadName_.c_str());
}

void ThreadBase::run() {
    if (!runningFlag_) {
        runningFlag_ = true;
        try {
            threadObj_ = std::thread(&ThreadBase::threadFunction, this);
            printf("[ThreadBase] %s thread start SUCCESS\n", threadName_.c_str());  
        } catch (const std::exception& e) {
            printf("[ThreadBase] %s thread start FAILED: %s\n", threadName_.c_str(), e.what());
            runningFlag_ = false;
        }
    } else {
        printf("[ThreadBase] %s thread is already running\n", threadName_.c_str());
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
        printf("[ThreadBase] %s thread is stopping\n", threadName_.c_str());
    } else {
        printf("[ThreadBase] %s thread is not running\n", threadName_.c_str());
    }
}

