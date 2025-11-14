#ifndef DB_THREAD_POOL_HPP_
#define DB_THREAD_POOL_HPP_

#include "ThreadBase.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>
#include <thread>
#include <queue>
#include <functional>

class SQLiteDatabase;
class EventQueue;

class DBThreadPool : public ThreadBase {
    public:
        explicit DBThreadPool(std::shared_ptr<EventQueue> eventQueue, int numWorkers = 4);
        ~DBThreadPool();

        void stop() override;
        void enqueueTask(std::function<void()> task);
        
        void insertAudioRecord(const std::string& filePath);

    private:
        void threadFunction() override;
        
        std::vector<std::thread> workers_;
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<SQLiteDatabase> database_;

        // Threadpool control
        std::mutex taskMutex_;
        std::mutex dbMutex_;
        std::condition_variable cv_;
        std::queue<std::function<void()>> tasksQueue_;

};

#endif // DB_THREAD_POOL_HPP_