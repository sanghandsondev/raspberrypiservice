#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <memory>
#include <string>

enum class EventTypeID;

class Payload {
    public:
        Payload() {}
        virtual ~Payload() {}   // Virtual destructor for proper cleanup of derived classes
};

class NotiPayload : public Payload {
    public:
        explicit NotiPayload(bool isSuccess = false, const std::string &msgInfo = "")
            : isSuccess_(isSuccess), msgInfo_(msgInfo) {}

        bool isSuccess() const { return isSuccess_; }
        std::string getMsgInfo() const { return msgInfo_; }

    private:
        bool isSuccess_;
        std::string msgInfo_;
};

// Hardware
class NotiTemperaturePayload : public Payload {
    public:
        explicit NotiTemperaturePayload(bool isSuccess, float temperatureValue) 
            : isSuccess_(isSuccess), temperatureValue_(temperatureValue) {}

        bool isSuccess() const { return isSuccess_; }
        float getTemperatureValue() const { return temperatureValue_; }

    private:
        bool isSuccess_;
        float temperatureValue_;
};

class NotiBTDeviceAddressPayload : public Payload {
    public:
        explicit NotiBTDeviceAddressPayload(bool isSuccess, const std::string &msgInfo, const std::string &address) 
            : isSuccess_(isSuccess), msgInfo_(msgInfo), address_(address) {}

        bool isSuccess() const { return isSuccess_; }
        std::string getMsgInfo() const { return msgInfo_; }
        std::string getAddress() const { return address_; }

    private:
        bool isSuccess_;
        std::string msgInfo_;
        std::string address_;
};

class BluetoothDevicePayload : public Payload {
    public:
        explicit BluetoothDevicePayload(const std::string &name, const std::string &address, int rssi, bool isPaired, bool isConnected, const std::string &icon)
            : name_(name), address_(address), rssi_(rssi), isPaired_(isPaired), isConnected_(isConnected), icon_(icon) {}

        std::string getName() const { return name_; }
        std::string getAddress() const { return address_; }
        int getRssi() const { return rssi_; }
        bool isPaired() const { return isPaired_; }
        bool isConnected() const { return isConnected_; }
        std::string getIcon() const {return icon_;}

    private:
        std::string name_;
        std::string address_;
        int rssi_;
        bool isPaired_;
        bool isConnected_;
        std::string icon_;
};

class BluetoothDeviceAddressPayload : public Payload {
    public:
        explicit BluetoothDeviceAddressPayload(const std::string &address)
            : address_(address) {}

        std::string getAddress() const { return address_; }

    private:
        std::string address_;
};

// Record
class WavPayload : public Payload {
    public:
        explicit WavPayload(const std::string &filePath, int durationSec = 0) : filePath_(filePath), durationSec_(durationSec) {}

        std::string getFilePath() const { return filePath_; }
        int getDurationSec() const { return durationSec_; }

    private:
        std::string filePath_;
        int durationSec_;
};

class RemoveRecordPayload : public Payload {
    public:
        explicit RemoveRecordPayload(int recordId) : recordId_(recordId) {}

        int getRecordId() const { return recordId_ ; }
        
    private:
        int recordId_;
};

class Event {
    public:
        explicit Event() : eventTypeId_(static_cast<EventTypeID>(0)), payload_(nullptr) {}

        Event(EventTypeID eventTypeId) : eventTypeId_(eventTypeId), payload_(nullptr) {}

        Event(EventTypeID eventTypeId, std::shared_ptr<Payload> payload) : eventTypeId_(eventTypeId), payload_(payload) {}
            
        EventTypeID getEventTypeId() const {
            return eventTypeId_;
        }

        std::shared_ptr<Payload> getPayload() const {
            return payload_;
        }

    private:
        EventTypeID eventTypeId_;
        std::shared_ptr<Payload> payload_;
};

#endif // EVENT_HPP_
