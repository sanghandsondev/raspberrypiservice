#ifndef MAIN_WORKER_HPP_
#define MAIN_WORKER_HPP_

#include <memory>
#include "RecordWorker.hpp"

#define MAIN_WORKER_INSTANCE() MainWorker::getInstance()

class MainWorker {
    public:
        static MainWorker* getInstance() {
            static MainWorker instance;
            return &instance;
        }
        MainWorker(const MainWorker&) = delete;
        MainWorker& operator=(const MainWorker&) = delete;

        void startRecord();
        void stopRecord();

    private:
        MainWorker() : recordWorker_(nullptr) {};
        ~MainWorker() = default;

        std::shared_ptr<RecordWorker> recordWorker_;
};

#endif // MAIN_WORKER_HPP_