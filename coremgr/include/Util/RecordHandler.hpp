#ifndef RECORD_HANDLER_HPP_
#define RECORD_HANDLER_HPP_

#include <memory>

class WebSocket;

class RecordHandler {
    public:
        explicit RecordHandler() = default;
        ~RecordHandler() = default;

        void setWebSocket(std::shared_ptr<WebSocket> ws){
            webSocket_ = ws;
        };

        void startRecord();
        void stopRecord();

        void startRecordNOTI();
        void stopRecordNOTI();
    
    private:
        std::shared_ptr<WebSocket> webSocket_;
};

#endif // RECORD_HANDLER_HPP_