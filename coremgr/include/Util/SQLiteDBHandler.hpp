#ifndef SQLITE_DB_HANDLER_HPP_
#define SQLITE_DB_HANDLER_HPP_

#include <memory>
#include <string>

class DBThreadPool;

class SQLiteDBHandler {
    public:
        explicit SQLiteDBHandler() = default;
        ~SQLiteDBHandler() = default;

        void setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool);

        // Additional database operations can be added here
        void insertAudioRecord(const std::string& filePath);

    private:
        std::shared_ptr<DBThreadPool> dbThreadPool_;
};


#endif // SQLITE_DB_HANDLER_HPP_