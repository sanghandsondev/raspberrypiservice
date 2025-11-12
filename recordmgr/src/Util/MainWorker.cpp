#include "MainWorker.hpp"
#include "RecordWorker.hpp"
#include "RMLogger.hpp"

void MainWorker::startRecord(){
    if (recordWorker_ && recordWorker_->isRunning()) {
        RM_LOG(WARN, "RecordWorker is already running. Ignoring start request.");
        return;
    }
    RM_LOG(INFO, "MainWorker creating and starting a new RecordWorker.");
    recordWorker_ = std::make_shared<RecordWorker>();
    recordWorker_->run();
}

void MainWorker::stopRecord(){
    if (recordWorker_) {
        RM_LOG(INFO, "MainWorker stopping RecordWorker.");
        recordWorker_->stop();
        recordWorker_->join();
        // Sau khi dừng, giải phóng con trỏ để lần sau có thể tạo mới
        recordWorker_.reset();  // nullptr
    } else {
        RM_LOG(WARN, "No active RecordWorker to stop.");
    }
}