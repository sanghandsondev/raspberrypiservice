#ifndef SQLITE_DATABASE_HPP_
#define SQLITE_DATABASE_HPP_

#include <string>
#include <sqlite3.h>
#include <vector>
#include <mutex> // Thêm header mutex
#include "Schema.hpp"

class SQLiteDatabase {
    public:
        explicit SQLiteDatabase(const std::string &dbFilePath);
        ~SQLiteDatabase();

        bool open();
        void close();

        // Schema initialization
        bool initializeSchema();

        // QUERY operations
        bool insertAudioRecord(const AudioRecord &record);
        std::vector<AudioRecord> getAllRecords();

    private:
        std::string dbFilePath_;
        struct sqlite3 *db_; // Forward declaration of sqlite3
        mutable std::mutex dbMutex_; // Thêm mutex

        bool executeSQL(const std::string& sql);
        sqlite3_stmt* prepareStatement(const std::string& sql);
};

#endif // SQLITE_DATABASE_HPP_