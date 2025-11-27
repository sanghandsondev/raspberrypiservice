#ifndef MONITOR_WORKER_HPP_
#define MONITOR_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_MONITOR_TEMPERATURE_TIMEOUT_MS  30000  // 30 seconds

class EventQueue;
class Event;
class Payload;

class MonitorWorker : public ThreadBase {
    public:
        explicit MonitorWorker(std::shared_ptr<EventQueue> eventQueue);
        ~MonitorWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;

        void threadFunction() override;
};

#endif // MONITOR_WORKER_HPP_