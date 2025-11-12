#include "EventQueue.hpp"
#include "Event.hpp"

EventQueue::EventQueue() {}

EventQueue::~EventQueue() {}

bool EventQueue::pushEvent(const std::shared_ptr<Event> event) {
    bool ret = false;

    if (event != nullptr) {
        std::lock_guard<std::mutex> lk(queueMutex_); 
        eventList_.push_back(event);

        notifyEvent();
        ret = true;
    }

    return ret;
}

std::shared_ptr<Event> EventQueue::popEvent() {
    std::shared_ptr<Event> event = nullptr;
    std::lock_guard<std::mutex> lk(queueMutex_);

    if (!eventList_.empty()) {
        event = eventList_.front();
        eventList_.pop_front();
    }

    return event;
}

bool EventQueue::hasEvent() {
    std::lock_guard<std::mutex> lk(queueMutex_);
    return !eventList_.empty();
}

std::size_t EventQueue::size() {
    std::lock_guard<std::mutex> lk(queueMutex_);
    return eventList_.size();
}

void EventQueue::notifyEvent() {
    std::lock_guard<std::mutex> lk_cv(wakeupMutex_);
    wakeupCondition_.notify_all();
}

void EventQueue::waitForEvent(const uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lk_cv(wakeupMutex_);
    wakeupCondition_.wait_for(lk_cv, std::chrono::milliseconds(timeout_ms));
}
