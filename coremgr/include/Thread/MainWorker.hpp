#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_EVENTQUEUE_TIMEOUT_MS  2500

class EventQueue;
class ThreadBase;
class Event;
class WebSocket;
class HardwareHandler;
class RecordHandler;
class Payload;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<EventQueue> eventQueue);
        ~MainWorker();

        void setWebSocket(std::shared_ptr<WebSocket> ws);

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<WebSocket> webSocket_;
        std::shared_ptr<HardwareHandler> hardwareHandler_;
        std::shared_ptr<RecordHandler> recordHandler_;

        void threadFunction() override;

        void processEvent(const std::shared_ptr<Event> event);
        
        // Hardware request
        void processOnOffLEDEvent();

        // Record request
        void processStartRecordEvent();
        void processStopRecordEvent();

        // Hardware notification
        void processTurnOnLEDNOTIEvent(std::shared_ptr<Payload>);
        void processTurnOffLEDNOTIEvent(std::shared_ptr<Payload>);

        // Record notification
        void processStartRecordNOTIEvent(std::shared_ptr<Payload>);
        void processStopRecordNOTIEvent(std::shared_ptr<Payload>);
        void processFilterWavFileNOTIEvent(std::shared_ptr<Payload>);
};

#endif // MAIN_WORKER_HPP_