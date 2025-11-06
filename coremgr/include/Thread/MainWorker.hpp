#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"

#define INTERNAL_EVENTQUEUE_TIMEOUT_MS  2500

class EventQueue;
class ThreadBase;
class GPIOHandler;
class Event;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<EventQueue> eventQueue);
        ~MainWorker();

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<GPIOHandler> gpioHandler_;

        void threadFunction() override;

        void processEvent(const std::shared_ptr<Event> event);
};

#endif // MAIN_WORKER_HPP_