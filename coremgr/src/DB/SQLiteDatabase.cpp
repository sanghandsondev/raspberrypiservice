#include "SQLiteDatabase.hpp"
#include "RLogger.hpp"
#include <filesystem>

namespace fs = std::filesystem;

SQLiteDatabase::SQLiteDatabase(const std::string &dbFilePath) : dbFilePath_(dbFilePath), db_(nullptr) {}

SQLiteDatabase::~SQLiteDatabase() {
    close();
}

bool SQLiteDatabase::open() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    try{
        fs::path dbPath(dbFilePath_);
        fs::create_directories(dbPath.parent_path());
        R_LOG(INFO, "SQLiteDatabase: ensured directory exists: %s", dbPath.parent_path().c_str());
    } catch (const fs::filesystem_error& e) {
        R_LOG(ERROR, "SQLiteDatabase: Failed to create database directory: %s", e.what());
        return false;
    }

    int rc = sqlite3_open(dbFilePath_.c_str(), &db_);

    if (rc != SQLITE_OK) {
        R_LOG(ERROR, "SQLiteDatabase: Failed to open database: %s", sqlite3_errmsg(db_));
        return false;
    }

    R_LOG(INFO, "SQLiteDatabase: Database opened successfully");
    return initializeSchema();
}

void SQLiteDatabase::close() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
        R_LOG(INFO, "SQLiteDatabase: Database closed");
    }
}

bool SQLiteDatabase::initializeSchema() {
    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS audio_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_path TEXT NOT NULL UNIQUE,
            duration_sec INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    return executeSQL(createTableSQL);
}

bool SQLiteDatabase::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;

    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        R_LOG(ERROR, "SQLiteDatabase: SQL execution error: %s", errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

sqlite3_stmt* SQLiteDatabase::prepareStatement(const std::string& sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        R_LOG(ERROR, "SQLiteDatabase: Failed to prepare statement: %s", sqlite3_errmsg(db_));
        return nullptr;
    }

    return stmt;
}

AudioRecord SQLiteDatabase::insertAudioRecord(const AudioRecord &record) {
    std::lock_guard<std::mutex> lock(dbMutex_);

    const std::string insertSQL = R"(
        INSERT INTO audio_records (file_path, duration_sec)
        VALUES (?, ?);
    )";

    sqlite3_stmt* stmt = prepareStatement(insertSQL);
    if (!stmt) {
        return {-1, "", 0};
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, record.filePath.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, record.durationSec);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        R_LOG(ERROR, "SQLiteDatabase: Failed to insert audio record: %s", sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        return {-1, "", 0};
    }

    sqlite3_int64 lastId = sqlite3_last_insert_rowid(db_);
    R_LOG(INFO, "SQLiteDatabase: Audio record inserted successfully with file path: %s", record.filePath.c_str());
    sqlite3_finalize(stmt);

    AudioRecord newRecord = record;
    newRecord.id = static_cast<int>(lastId);
    return newRecord;
}

std::vector<AudioRecord> SQLiteDatabase::getAllRecords() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    
    std::vector<AudioRecord> records;
    const std::string querySQL = R"(
        SELECT id, file_path, duration_sec FROM audio_records ORDER BY created_at DESC LIMIT 100;
    )";

    sqlite3_stmt* stmt = prepareStatement(querySQL);
    if (!stmt) {
        return records; // Return empty vector on failure
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AudioRecord record;
        record.id = sqlite3_column_int(stmt, 0);
        record.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.durationSec = sqlite3_column_int(stmt, 2);
        
        records.push_back(record);
    }

    sqlite3_finalize(stmt);
    R_LOG(INFO, "SQLiteDatabase: Retrieved %zu audio records from database", records.size());
    return records;
}

std::string SQLiteDatabase::removeAudioRecord(int recordId) {
    std::lock_guard<std::mutex> lock(dbMutex_);

    const std::string deleteSQL = "DELETE FROM audio_records WHERE id = ? RETURNING file_path;";
    sqlite3_stmt* stmt = prepareStatement(deleteSQL);
    if (!stmt) {
        return "";
    }

    sqlite3_bind_int(stmt, 1, recordId);

    std::string filePath = "";
    int rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // A row was deleted and its file_path is returned
        filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        R_LOG(INFO, "SQLiteDatabase: Audio record with id %d deleted successfully.", recordId);
    } else if (rc == SQLITE_DONE) {
        // No row was deleted (recordId not found)
        R_LOG(WARN, "SQLiteDatabase: No record found with id %d to delete.", recordId);
    } else {
        // An error occurred
        R_LOG(ERROR, "SQLiteDatabase: Failed to delete audio record with id %d: %s", recordId, sqlite3_errmsg(db_));
    }

    sqlite3_finalize(stmt);
    return filePath;
}