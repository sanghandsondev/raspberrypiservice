#include "SQLiteDatabase.hpp"
#include "RLogger.hpp"
#include <filesystem>

namespace fs = std::filesystem;

SQLiteDatabase::SQLiteDatabase(const std::string &dbFilePath) : dbFilePath_(dbFilePath), db_(nullptr) {}

SQLiteDatabase::~SQLiteDatabase() {
    close();
}

bool SQLiteDatabase::open() {
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

bool SQLiteDatabase::insertAudioRecord(const AudioRecord &record) {
    const std::string insertSQL = R"(
        INSERT INTO audio_records (file_path)
        VALUES (?);
    )";

    sqlite3_stmt* stmt = prepareStatement(insertSQL);
    if (!stmt) {
        return false;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, record.filePath.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        R_LOG(ERROR, "SQLiteDatabase: Failed to insert audio record: %s", sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        return false;
    }

    R_LOG(INFO, "SQLiteDatabase: Audio record inserted successfully with file path: %s", record.filePath.c_str());
    sqlite3_finalize(stmt);
    return true;
}

std::vector<AudioRecord> SQLiteDatabase::getAllRecords() {
    std::vector<AudioRecord> records;
    const std::string querySQL = R"(
        SELECT id, file_path, created_at FROM audio_records ORDER BY created_at DESC LIMIT 100;
    )";

    sqlite3_stmt* stmt = prepareStatement(querySQL);
    if (!stmt) {
        return records; // Return empty vector on failure
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AudioRecord record;
        record.id = sqlite3_column_int(stmt, 0);
        record.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        records.push_back(record);
    }

    sqlite3_finalize(stmt);
    R_LOG(INFO, "SQLiteDatabase: Retrieved %zu audio records from database", records.size());
    return records;
}