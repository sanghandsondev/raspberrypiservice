#ifndef SQLITE_DB_HANDLER_HPP_
#define SQLITE_DB_HANDLER_HPP_

#include <memory>
#include <string>

class WebSocket;
class DBThreadPool;

class SQLiteDBHandler {
    public:
        explicit SQLiteDBHandler() = default;
        ~SQLiteDBHandler() = default;

        void setWebSocket(std::shared_ptr<WebSocket> ws){ webSocket_ = ws; };
        void setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool);

        // Additional database operations can be added here
        void insertAudioRecord(const std::string& filePath);
        void getAllAudioRecords();

    private:
        std::shared_ptr<WebSocket> webSocket_;
        std::shared_ptr<DBThreadPool> dbThreadPool_;
};


#endif // SQLITE_DB_HANDLER_HPP_