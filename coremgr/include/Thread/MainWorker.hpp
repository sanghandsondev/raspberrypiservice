#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_EVENTQUEUE_TIMEOUT_MS  2500

class EventQueue;
class ThreadBase;
class Event;
class WebSocket;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<EventQueue> eventQueue);
        ~MainWorker();

        void setWebSocket(std::shared_ptr<WebSocket> ws);

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<WebSocket> webSocket_;

        void threadFunction() override;

        void processEvent(const std::shared_ptr<Event> event);
        void processOnOffLEDEvent();
        void processStartRecordEvent();
        // void processStopRecordEvent();
        void processStartRecordNOTIEvent();
        // void processStopRecordNOTIEvent();
};

#endif // MAIN_WORKER_HPP_