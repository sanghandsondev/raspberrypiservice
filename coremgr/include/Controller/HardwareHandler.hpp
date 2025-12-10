#ifndef HARDWARE_HANDLER_HPP_
#define HARDWARE_HANDLER_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#define TIMEOUT_REQUEST_CONFIRMATION_MS 30000 // 30 seconds

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
        void acceptBTDeviceRequestConfirmation(std::shared_ptr<Payload>);
        void rejectBTDeviceRequestConfirmation(std::shared_ptr<Payload>);
        void dialCall(std::shared_ptr<Payload>);
        void hangupCall();
        void answerCall();

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
        void btDeviceRequestConfirmationNOTI(std::shared_ptr<Payload>);
        void handleBTDeviceRequestConfirmationTimeout(std::shared_ptr<Payload>);

        void pbapSessionEndNOTI(std::shared_ptr<Payload>);
        void pbapPhonebookPullStartNOTI(std::shared_ptr<Payload>);
        void pbapPhonebookPullNOTI(std::shared_ptr<Payload>);
        void pbapPhonebookPullEndNOTI(std::shared_ptr<Payload>);
        void callHistoryPullStartNOTI(std::shared_ptr<Payload>);
        void callHistoryPullNOTI(std::shared_ptr<Payload>);
        void callHistoryPullEndNOTI(std::shared_ptr<Payload>);

        void incomingCallNOTI(std::shared_ptr<Payload>);
        void outgoingCallNOTI(std::shared_ptr<Payload>);
        void callStateChangedNOTI(std::shared_ptr<Payload>);
        void callEndedNOTI(std::shared_ptr<Payload>);
        void dialCallNOTI(std::shared_ptr<Payload>);
        void hangupCallNOTI(std::shared_ptr<Payload>);
        void answerCallNOTI(std::shared_ptr<Payload>);
    
    private:
        std::unordered_map<std::string, int32_t> timerIdMap_;    // <deviceAddress, timerId>
        void removeTimerOnTimerIdMap(const std::string& key);

        std::shared_ptr<WebSocket> webSocket_;
};

#endif // HARDWARE_HANDLER_HPP_