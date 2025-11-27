#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_EVENTQUEUE_TIMEOUT_MS  2500

class EventQueue;
class Event;
class Payload;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<EventQueue> eventQueue);
        ~MainWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;

        void threadFunction() override;

        void processEvent(const std::shared_ptr<Event> event);

        // void processUpdateTemperatureEvent(std::shared_ptr<Payload> payload);
};

#endif // MAIN_WORKER_HPP_