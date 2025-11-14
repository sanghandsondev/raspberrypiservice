#include "DBThreadPool.hpp"
#include "SQLiteDatabase.hpp"
#include "EventQueue.hpp"
#include "RLogger.hpp"
#include "Config.hpp"

DBThreadPool::DBThreadPool(std::shared_ptr<EventQueue> eventQueue, int numWorkers) 
    : ThreadBase("DBThreadPool"), eventQueue_(eventQueue) {
    // Initialize database connection
    database_ = std::make_shared<SQLiteDatabase>(CONFIG_INSTANCE()->getSQLiteDBFilePath());
    if (!database_->open()) {
        R_LOG(ERROR, "DBThreadPool: Failed to open database connection");
    } else {
        R_LOG(INFO, "DBThreadPool: Database connection opened successfully");
    }

    // Start worker threads
    for (int i = 0; i < numWorkers; i++) {
        // Explicitly bind threadFunction to this instance
        workers_.emplace_back(&DBThreadPool::threadFunction, this);
    }
    R_LOG(INFO, "DBThreadPool: Started %d worker threads", numWorkers);
}

DBThreadPool::~DBThreadPool() {
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    if (database_) {
        database_->close();
    }
    R_LOG(INFO, "DBThreadPool: Shutdown complete");
}

void DBThreadPool::stop() {
    ThreadBase::stop();
    cv_.notify_all(); // Wake up all worker threads to exit
}

void DBThreadPool::enqueueTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        if (!runningFlag_) {
            R_LOG(WARN, "DBThreadPool: Attempted to enqueue task after stop was called");
            return;
        }
        tasksQueue_.push(std::move(task));
    }
    cv_.notify_one();
}

void DBThreadPool::insertAudioRecord(const std::string& filePath) {
    auto task = [this, filePath]() {
        AudioRecord record;
        record.filePath = filePath;

        std::lock_guard<std::mutex> dbLock(dbMutex_);
        if (!database_->insertAudioRecord(record)) {
            R_LOG(ERROR, "DBThreadPool: Failed to insert audio record into database");
        } else {
            R_LOG(INFO, "DBThreadPool: Audio record inserted successfully: %s", filePath.c_str());
        }
    };

    enqueueTask(task);
}

void DBThreadPool::threadFunction() {
    R_LOG(INFO, "DBThreadPool worker thread started");

    while (runningFlag_) {
        std::unique_lock<std::mutex> lock(taskMutex_);
        cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });

        if (!runningFlag_) {
            break;
        }

        if(tasksQueue_.empty()) {
            continue;
        }

        auto task = std::move(tasksQueue_.front());
        tasksQueue_.pop();
        lock.unlock();

        // Execute the task
        try {
            task();
        } catch (const std::exception& e) {
            R_LOG(ERROR, "DBThreadPool: Exception in worker thread: %s", e.what());
        }
    }

    R_LOG(INFO, "DBThreadPool worker thread exiting");
}







