#include "MainWorker.hpp"
#include "RecordWorker.hpp"
#include "RMLogger.hpp"

void MainWorker::startRecord(){
    RM_LOG(INFO, "MainWorker starting RecordWorker");
    recordWorker_->run();
}

void MainWorker::stopRecord(){
    RM_LOG(INFO, "MainWorker stopping RecordWorker");
    recordWorker_->stop();
    recordWorker_->join();
}