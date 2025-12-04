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
        void bluetoothPowerOn();
        void bluetoothPowerOff();
        void pairBTDevice(std::shared_ptr<Payload>);
        void unpairBTDevice(std::shared_ptr<Payload>);
        void connectBTDevice(std::shared_ptr<Payload>);
        void disconnectBTDevice(std::shared_ptr<Payload>);

        void updateTemperatureNOTI(std::shared_ptr<Payload>);
        void startScanBTDeviceNOTI(std::shared_ptr<Payload>);
        void stopScanBTDeviceNOTI(std::shared_ptr<Payload>);
        void scanningBTDeviceFoundNOTI(std::shared_ptr<Payload>);
        void scanningBTDeviceDeleteNOTI(std::shared_ptr<Payload>);
        void bluetoothPowerOnNOTI(std::shared_ptr<Payload>);
        void bluetoothPowerOffNOTI(std::shared_ptr<Payload>);
        void btDevicePropertyChangeNOTI(std::shared_ptr<Payload>);
        void pairBTDeviceNOTI(std::shared_ptr<Payload>);
        void unpairBTDeviceNOTI(std::shared_ptr<Payload>);
        void connectBTDeviceNOTI(std::shared_ptr<Payload>);
        void disconnectBTDeviceNOTI(std::shared_ptr<Payload>);
    
    private:
        std::shared_ptr<WebSocket> webSocket_;
};

#endif // HARDWARE_HANDLER_HPP_