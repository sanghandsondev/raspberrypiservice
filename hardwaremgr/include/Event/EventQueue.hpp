#ifndef EVENT_QUEUE_HPP_
#define EVENT_QUEUE_HPP_

#include <memory>
#include <list>
#include <mutex>
#include <cstdint> // for uint32_t
#include <condition_variable>

class Event;

class EventQueue {
    public:
        EventQueue();
        ~EventQueue();

        bool pushEvent(const std::shared_ptr<Event> event);
        std::shared_ptr<Event> popEvent();
        bool hasEvent();
        std::size_t size();

        void waitForEvent(const uint32_t timeout_ms);
    private:
        std::mutex queueMutex_;
        std::mutex wakeupMutex_;
        std::condition_variable wakeupCondition_;

        std::list<std::shared_ptr<Event>> eventList_;

        void notifyEvent();
};

#endif // EVENT_QUEUE_HPP_