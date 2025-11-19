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

        bool isSuccess() const {
            return isSuccess_;
        }

        std::string getMsgInfo() const {
            return msgInfo_;
        }

    private:
        bool isSuccess_;
        std::string msgInfo_;
};

// TODO
class WavPayload : public Payload {
    public:
        explicit WavPayload(const std::string &filePath = "") : filePath_(filePath) {}

        std::string getFilePath() const {
            return filePath_;
        }

    private:
        std::string filePath_;
};

// TODO
class RemoveRecordPayload : public Payload {
    
}

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
