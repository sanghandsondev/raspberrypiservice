#ifndef DB_THREAD_POOL_HPP_
#define DB_THREAD_POOL_HPP_

#include "ThreadBase.hpp"
#include "Schema.hpp"
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

class EventQueue;
class SQLiteDatabase;

class DBThreadPool : public ThreadBase {
public:
    DBThreadPool(std::shared_ptr<EventQueue> eventQueue, int numWorkers);
    ~DBThreadPool();

    void stop() override;
    void enqueueTask(std::function<void()> task);

    std::future<AudioRecord> insertAudioRecord(const std::string& filePath, int durationSec);
    std::future<std::vector<AudioRecord>> getAllAudioRecords();
    std::future<bool> removeAudioRecord(int recordId);

protected:
    void threadFunction() override;

private:
    std::shared_ptr<EventQueue> eventQueue_;
    std::shared_ptr<SQLiteDatabase> database_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasksQueue_;
    std::mutex taskMutex_;
    std::mutex dbMutex_;
    std::condition_variable cv_;
};

#endif // DB_THREAD_POOL_HPP_