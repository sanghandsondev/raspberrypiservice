#ifndef RECORD_HANDLER_HPP_
#define RECORD_HANDLER_HPP_

#include <memory>

class WebSocket;
class Payload;

class RecordHandler {
    public:
        explicit RecordHandler() = default;
        ~RecordHandler() = default;

        void setWebSocket(std::shared_ptr<WebSocket> ws){
            webSocket_ = ws;
        };

        void startRecord();
        void stopRecord();

        void startRecordNOTI(std::shared_ptr<Payload>);
        void stopRecordNOTI(std::shared_ptr<Payload>);
        void filterWavFileNOTI(std::shared_ptr<Payload>);
    
    private:
        std::shared_ptr<WebSocket> webSocket_;
};

#endif // RECORD_HANDLER_HPP_