#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_EVENTQUEUE_TIMEOUT_MS  (2500)

class EventQueue;
class Event;
class Payload;
class BluezDBus;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus);
        ~MainWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<BluezDBus> bluezDBus_;

        void threadFunction() override;

        void processEvent(const std::shared_ptr<Event> event);

        void processStartScanBTDeviceEvent();
        void processStopScanBTDeviceEvent();
};

#endif // MAIN_WORKER_HPP_