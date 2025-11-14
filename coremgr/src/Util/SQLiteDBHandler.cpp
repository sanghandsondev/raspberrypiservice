#include "SQLiteDBHandler.hpp"
#include "DBThreadPool.hpp"

void SQLiteDBHandler::setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) {
    dbThreadPool_ = dbThreadPool;
}

void SQLiteDBHandler::insertAudioRecord(const std::string& filePath) {
    if (dbThreadPool_) {
        dbThreadPool_->insertAudioRecord(filePath);
    }
}