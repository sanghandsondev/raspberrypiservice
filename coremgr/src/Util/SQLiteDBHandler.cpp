#include "SQLiteDBHandler.hpp"
#include "DBThreadPool.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include <vector>
#include <future>
#include "Schema.hpp"
#include "json.hpp"     // nlohmann::json
#include "Event.hpp"

void SQLiteDBHandler::setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) {
    dbThreadPool_ = dbThreadPool;
}

void SQLiteDBHandler::getAllAudioRecords() {
    if (dbThreadPool_ == nullptr) {
        R_LOG(ERROR, "DBThreadPool is not set in SQLiteDBHandler");
        return;
    }

    // Retrieve updated list of audio records
    auto future = dbThreadPool_->getAllAudioRecords();
    std::vector<AudioRecord> vec = future.get(); // Blocking call
    R_LOG(INFO, "SQLiteDBHandler: Retrieved %zu audio records from database", vec.size());

    // Broadcast updated record list
    nlohmann::json jsonVec = nlohmann::json::array();
    for (const auto& record : vec) {
        nlohmann::json recordJson;
        recordJson["id"] = record.id;
        recordJson["file_path"] = record.filePath;
        recordJson["duration_sec"] = record.durationSec;
        jsonVec.push_back(recordJson);
    }
    webSocket_->getServer()->updateStateAndBroadcast("success", "Fetched audio records", "Record", "get_all_record_noti", {{"records", jsonVec}});
}

void SQLiteDBHandler::insertAudioRecord(std::shared_ptr<Payload> payload){
    std::shared_ptr<WavPayload> insertPayload = std::dynamic_pointer_cast<WavPayload>(payload);
    if (insertPayload == nullptr) {
        R_LOG(ERROR, "No valid payload for inserting audio record");
        return;
    }

    std::string filePath = insertPayload->getFilePath();
    int durationSec = insertPayload->getDurationSec();

    if (dbThreadPool_ == nullptr) {
        R_LOG(ERROR, "DBThreadPool is not set in SQLiteDBHandler");
        return;
    }

    auto future = dbThreadPool_->insertAudioRecord(filePath, durationSec);
    AudioRecord newRecord = future.get(); // Blocking call

    if (newRecord.id != -1) {
        R_LOG(INFO, "Successfully inserted audio record with id %d.", newRecord.id);
        nlohmann::json recordJson;
        recordJson["id"] = newRecord.id;
        recordJson["file_path"] = newRecord.filePath;
        recordJson["duration_sec"] = newRecord.durationSec;
        webSocket_->getServer()->updateStateAndBroadcast("success", "Record inserted successfully", "Record", "insert_record_noti", {{"record", recordJson}});
    } else {
        R_LOG(ERROR, "Failed to insert audio record.");
        webSocket_->getServer()->updateStateAndBroadcast("fail", "Failed to insert record to DB", "Record", "insert_record_noti", {});
    }
}

void SQLiteDBHandler::removeAudioRecord(std::shared_ptr<Payload> payload) {
    std::shared_ptr<RemoveRecordPayload> removePayload = std::dynamic_pointer_cast<RemoveRecordPayload>(payload);
    if (removePayload == nullptr) {
        R_LOG(ERROR, "No valid payload for removing audio record");
        return;
    }

    int recordId = removePayload->getRecordId();

    if (dbThreadPool_ == nullptr) {
        R_LOG(ERROR, "DBThreadPool is not set in SQLiteDBHandler");
        return;
    }

    auto future = dbThreadPool_->removeAudioRecord(recordId);
    bool success = future.get(); // Blocking call

    if (success) {
        R_LOG(INFO, "Successfully removed audio record with id %d, now fetching updated list.", recordId);
        webSocket_->getServer()->updateStateAndBroadcast("success", "Record removed successfully", "Record", "remove_record_noti", {{"id", recordId}});
    } else {
        R_LOG(ERROR, "Failed to remove audio record with id %d.", recordId);
        // Optionally, notify client about the failure
        webSocket_->getServer()->updateStateAndBroadcast("fail", "Failed to remove record", "Record", "remove_record_noti", {{"id", recordId}});
    }
}