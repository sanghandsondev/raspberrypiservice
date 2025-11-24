#ifndef RECORD_WORKER_HPP_
#define RECORD_WORKER_HPP_

#include "ThreadBase.hpp"
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

class EventQueue;
class AlsaHelper;

class RecordWorker : public ThreadBase {
    public:
        explicit RecordWorker(std::shared_ptr<EventQueue> eventQueue);
        ~RecordWorker();

        void startRecording();
        void stopRecording();
        void cancelRecording();
        void stop() override;

    private:
        enum class State {
            IDLE,
            RECORDING
        };

        void threadFunction() override;

        std::shared_ptr<EventQueue> eventQueue_;
        std::unique_ptr<AlsaHelper> alsaHelper_;

        // Thread control
        std::mutex mtx_;
        std::condition_variable cv_;
        std::atomic<State> state_;
        std::atomic<bool> cancelRequested_;
};

#endif // RECORD_WORKER_HPP_