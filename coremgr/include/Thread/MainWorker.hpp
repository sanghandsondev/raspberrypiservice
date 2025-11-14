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
class DBThreadPool;
class SQLiteDBHandler;

class MainWorker : public ThreadBase {
    public:
        explicit MainWorker(std::shared_ptr<EventQueue> eventQueue);
        ~MainWorker();

        void setWebSocket(std::shared_ptr<WebSocket> ws);
        void setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool);

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<HardwareHandler> hardwareHandler_;
        std::shared_ptr<RecordHandler> recordHandler_;
        std::shared_ptr<SQLiteDBHandler> sqliteDBHandler_;

        std::shared_ptr<WebSocket> webSocket_;
        std::shared_ptr<DBThreadPool> dbThreadPool_;

        void threadFunction() override;
        void processEvent(const std::shared_ptr<Event> event);
};

#endif // MAIN_WORKER_HPP_