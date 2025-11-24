#include "SQLiteDBHandler.hpp"
#include "DBThreadPool.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include <vector>
#include "Schema.hpp"
#include "json.hpp"     // nlohmann::json

void SQLiteDBHandler::setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) {
    dbThreadPool_ = dbThreadPool;
}

void SQLiteDBHandler::insertAudioRecord() {
    if (dbThreadPool_ == nullptr) {
        return;
    }

    dbThreadPool_->insertAudioRecord(filePath);
}

void SQLiteDBHandler::getAllAudioRecords() {
    // if (dbThreadPool_ == nullptr) {
    //     return;
    // }

    // // Retrieve updated list of audio records
    // std::vector<AudioRecord> vec;
    // dbThreadPool_->getAllAudioRecords(vec);
    // R_LOG(INFO, "SQLiteDBHandler: Retrieved %zu audio records from database", vec.size());

    // // Broadcast updated record list
    // nlohmann::json jsonVec = nlohmann::json::array();
    // for (const auto& record : vec) {
    //     nlohmann::json recordJson;
    //     recordJson["id"] = record.id;
    //     recordJson["file_path"] = record.filePath;
    //     jsonVec.push_back(recordJson);
    // }
    // webSocket_->getServer()->updateStateAndBroadcast();
}

void SQLiteDBHandler::insertAudioRecord(std::shared_ptr<Payload> payload){
    // TODO
}