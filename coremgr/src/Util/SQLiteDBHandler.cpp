#include "SQLiteDBHandler.hpp"
#include "DBThreadPool.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include <vector>
#include "Schema.hpp"

void SQLiteDBHandler::setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) {
    dbThreadPool_ = dbThreadPool;
}

void SQLiteDBHandler::insertAudioRecord(const std::string& filePath) {
    if (dbThreadPool_ == nullptr) {
        return;
    }

    dbThreadPool_->insertAudioRecord(filePath);
}

void SQLiteDBHandler::getAllAudioRecords() {
    if (dbThreadPool_ == nullptr) {
        return;
    }
    std::vector<AudioRecord> vec;
    dbThreadPool_->getAllAudioRecords(vec);
    R_LOG(INFO, "SQLiteDBHandler: Retrieved %zu audio records from database", vec.size());
    // webSocket_->getServer()->updateStateAndBroadcast(); // TODO
}