#ifndef HARDWARE_HANDLER_HPP_
#define HARDWARE_HANDLER_HPP_

#include <memory>

class WebSocket;
class Payload;

class HardwareHandler {
    public:
        explicit HardwareHandler() = default;
        ~HardwareHandler() = default;

        void setWebSocket(std::shared_ptr<WebSocket> ws){ webSocket_ = ws; }

        void startScanBTDevice();
        void stopScanBTDevice();

        void updateTemperatureNOTI(std::shared_ptr<Payload>);
        void startScanBTDeviceNOTI(std::shared_ptr<Payload>);
        void stopScanBTDeviceNOTI(std::shared_ptr<Payload>);
        void pairedBTDeviceFoundNOTI(std::shared_ptr<Payload>);
        void scanningBTDeviceFoundNOTI(std::shared_ptr<Payload>);
    
    private:
        std::shared_ptr<WebSocket> webSocket_;
};

#endif // HARDWARE_HANDLER_HPP_