#include "SQLiteDBHandler.hpp"
#include "DBThreadPool.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include <vector>
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
    std::vector<AudioRecord> vec;
    // TODO: Currently not blocking; in real implementation, consider using futures or callbacks
    // to handle async results properly.
    dbThreadPool_->getAllAudioRecords(vec);
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
    webSocket_->getServer()->updateStateAndBroadcast("success", "Fetched audio records", "Record", "update_list_record", {{"records", jsonVec}});
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

    // TODO: How to handle async result properly should be considered in real implementation ?
    // For now, we just enqueue the task.
    dbThreadPool_->insertAudioRecord(filePath, durationSec);
    getAllAudioRecords();
}