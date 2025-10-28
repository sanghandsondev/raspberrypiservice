#ifndef THREAD_BASE_HPP_
#define THREAD_BASE_HPP_

#include <thread>
#include <atomic>

class ThreadBase {
    protected:
        std::atomic<bool> runningFlag_;
        virtual void threadFunction() = 0;

    private:
        std::thread threadObj_;
        std::string threadName_;
    public:
        explicit ThreadBase(std::string threadName);
        virtual ~ThreadBase();

        void run();
        void stop();
        void join();
};

#endif // THREAD_BASE_HPP_