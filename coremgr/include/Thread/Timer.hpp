#ifndef TIMER_HPP_
#define TIMER_HPP_

#include "ThreadBase.hpp"
#include <memory>
#include <chrono>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#define INTERNAL_SLEEP_TIMEOUT_MS   (2500)
#define TIMER_INSTANCE() Timer::getInstance()

class TimerElement{
    public:
        TimerElement(){};
        ~TimerElement(){};
        int32_t timerId = -1;
        uint32_t timeout_ms = 0;
        std::chrono::steady_clock::time_point expireTime;
        std::shared_ptr<class Event> event = nullptr;
};

class Timer : public ThreadBase {
    public:
        static std::unique_ptr<Timer> &getInstance();
        Timer(const Timer &) = delete;
        Timer(Timer &&) noexcept = delete;
        Timer &operator=(const Timer &) = delete;
        Timer &operator=(Timer &&) noexcept = delete;

        int32_t startTimer(const uint32_t timeout_ms, std::shared_ptr<class Event> event);
        bool stopTimer(int32_t timerId);

        void SetEventQueue(std::shared_ptr<class EventQueue> eventQueue);
    private:
        Timer();
        std::shared_ptr<class EventQueue> eventQueue_;
        inline static std::once_flag onceFlag = {};
        std::unordered_map<int32_t, std::shared_ptr<TimerElement>> timerTableMap; // <timerId, TimerInfo>
        std::condition_variable timerWakeupCondition;
        std::mutex timerMutex;
        std::vector<std::shared_ptr<TimerElement>> expiredTimerElementList;
        
        void threadFunction() override;
        void onStop() override;

        void excuteIfExpiredExistAndUpdateTimerElements(std::chrono::steady_clock::time_point &currentTime);
        void sleepForNextItem(std::chrono::steady_clock::time_point &currentTime);

        int32_t createNewTimerId();
        std::shared_ptr<TimerElement> makeNewTimerElement(const uint32_t timeout_ms, std::shared_ptr<class Event> event);

};


#endif // TIMER_HPP_