#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_EVENTQUEUE_TIMEOUT_MS  2500

class EventQueue;
class Event;
class RecordWorker;
class Payload;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<class EventQueue> eventQueue, std::shared_ptr<RecordWorker> recordWorker);
        ~MainWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<RecordWorker> recordWorker_;

        void threadFunction() override;

        void processEvent(const std::shared_ptr<Event> event);

        void processStartRecordEvent();
        void processStopRecordEvent();
        void processCancelRecordEvent();
        void processFilterWavFileEvent(std::shared_ptr<Payload>);
};

#endif // MAIN_WORKER_HPP_