#include "Timer.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "RLogger.hpp"
#include <thread>
#include <time.h>

std::unique_ptr<Timer> &Timer::getInstance() {
    static std::unique_ptr<Timer> instance;
    std::call_once(onceFlag, []() {
        instance.reset(new Timer());
        instance->run();
    });
    return instance;
}

Timer::Timer() : ThreadBase("Timer") {}

void Timer::onStop() {
    std::lock_guard<std::mutex> lock(timerMutex);
    timerWakeupCondition.notify_all();
}

void Timer::threadFunction(){
    std::chrono::steady_clock::time_point currentTime;
    R_LOG(INFO, "Timer Thread function started");
    while (runningFlag_)
    {
        currentTime = std::chrono::steady_clock::now();
        if (timerTableMap.empty() == false) {
            excuteIfExpiredExistAndUpdateTimerElements(currentTime);
        }
        sleepForNextItem(currentTime);
    }
}

void Timer::excuteIfExpiredExistAndUpdateTimerElements(std::chrono::steady_clock::time_point &currentTime) {
    std::unique_lock<std::mutex> lock(timerMutex);
    for (auto iter = timerTableMap.begin(); iter != timerTableMap.end(); ) {
        auto timerElement = iter->second;
        if (timerElement->expireTime <= currentTime) {
            expiredTimerElementList.push_back(timerElement);
            iter = timerTableMap.erase(iter);
        } else {
            ++iter;
        }
    }
    lock.unlock();

    for (const auto& timerElement : expiredTimerElementList) {
        if (eventQueue_ && timerElement->event) {
            eventQueue_->pushEvent(timerElement->event);
            R_LOG(DEBUG, "Timer expired: TimerID=%d, Timeout=%u ms", timerElement->timerId, timerElement->timeout_ms);
        }
    }
    expiredTimerElementList.clear();
}

void Timer::sleepForNextItem(std::chrono::steady_clock::time_point &currentTime) {
    std::unique_lock<std::mutex> lock(timerMutex);

    std::chrono::steady_clock::time_point nextItemTimeout;
    const std::chrono::steady_clock::time_point defaultTimeout =
        currentTime + std::chrono::milliseconds(INTERNAL_SLEEP_TIMEOUT_MS);

    if (!timerTableMap.empty()) {
        // Find minimun expire time
        nextItemTimeout = timerTableMap.begin()->second->expireTime;
        for (const auto& pair : timerTableMap) {
            if (pair.second->expireTime < nextItemTimeout) {
                nextItemTimeout = pair.second->expireTime;
            }
        }

        //  If the time to the next item is longer than default timeout, set to default timeout
        if (nextItemTimeout > defaultTimeout) {
            nextItemTimeout = defaultTimeout;
        }

    } else {
        nextItemTimeout = defaultTimeout;
    }
    size_t mapSize = timerTableMap.size();
    R_LOG(DEBUG, "Timer sleep for next item: TimerCount=%zu", mapSize);

    // Wait until next item timeout or new timer added/stopped
    timerWakeupCondition.wait_until(lock, nextItemTimeout, [this, mapSize]() -> bool {
        return (!runningFlag_ || timerTableMap.size() != mapSize);
    });
}

void Timer::SetEventQueue(std::shared_ptr<EventQueue> eventQueue) {
    eventQueue_ = eventQueue;
}

int32_t Timer::createNewTimerId() {
    static int32_t nextTimerId = -1;
    int32_t newTimerId = -1;
    std::lock_guard<std::mutex> lock(timerMutex);

    if (timerTableMap.size() >= INT32_MAX - 1) {
        R_LOG(ERROR, "Timer ID space exhausted");
        return newTimerId;
    }

    if (nextTimerId == INT32_MAX) {
        nextTimerId = 1; // Wrap around
    }

    // Find next available Timer ID
    while (timerTableMap.find(nextTimerId) != timerTableMap.end()) {
        nextTimerId++;
        if (nextTimerId == INT32_MAX) {
            nextTimerId = 1; // Wrap around
        }
    }
    newTimerId = nextTimerId;
    return newTimerId;
}

std::shared_ptr<TimerElement> Timer::makeNewTimerElement(const uint32_t timeout_ms, std::shared_ptr<Event> event) {
    int32_t timerId = createNewTimerId();

    if (timerId == -1) {
        R_LOG(ERROR, "Failed to create new Timer ID");
        return nullptr;
    }

    std::shared_ptr<TimerElement> timerElement = std::make_shared<TimerElement>();

    timerElement->timerId = timerId;
    timerElement->timeout_ms = timeout_ms;
    timerElement->expireTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    timerElement->event = event;

    return timerElement;
}

int32_t Timer::startTimer(const uint32_t timeout_ms, std::shared_ptr<Event> event) {
    if (eventQueue_ == nullptr) {
        R_LOG(ERROR, "EventQueue is not set in Timer");
        return -1;
    }

    std::shared_ptr<TimerElement> timerElement = makeNewTimerElement(timeout_ms, event);

    if (timerElement == nullptr) {
        R_LOG(ERROR, "Failed to create Timer Element");
        return -1;
    }

    std::lock_guard<std::mutex> lock(timerMutex);
    timerTableMap.insert({timerElement->timerId, timerElement});
    R_LOG(DEBUG, "Started Timer: TimerID=%d, Timeout=%u ms", timerElement->timerId, timeout_ms);
    timerWakeupCondition.notify_all();

    return timerElement->timerId;
}

bool Timer::stopTimer(int32_t timerId) {
    bool ret = false;
    std::lock_guard<std::mutex> lock(timerMutex);

    auto iter = timerTableMap.find(timerId);
    if (iter != timerTableMap.end()) {
        timerTableMap.erase(iter);
        R_LOG(DEBUG, "Stopped Timer: TimerID=%d", timerId);
        ret = true;
        timerWakeupCondition.notify_all();
    } else {
        R_LOG(WARN, "Timer ID %d not found", timerId);
    }

    return ret;
}