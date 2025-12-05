#ifndef THREAD_BASE_HPP_
#define THREAD_BASE_HPP_

#include <thread>
#include <atomic>
#include <string>

class ThreadBase {
    private:
        std::thread threadObj_;
        std::string threadName_;

    protected:
        std::atomic<bool> runningFlag_;
        virtual void threadFunction() = 0;
        virtual void onStop() {}

    public:
        explicit ThreadBase(std::string threadName);
        virtual ~ThreadBase();

        bool isRunning() const {
            return runningFlag_;
        }
        void run();
        void join();
        virtual void stop();
};

#endif // THREAD_BASE_HPP_