#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <memory>
#include "EventTypeId.hpp"

enum class EventTypeID;

class Payload {
    public:
        Payload() {}
        virtual ~Payload() {}   // Virtual destructor for proper cleanup of derived classes
};

class LEDPayload : public Payload {
    public:
        explicit LEDPayload(bool state) : state_(state) {}

        bool getState() const {
            return state_;
        }

    private:
        bool state_;
};

class Event {
    public:
        explicit Event() : eventTypeId_(EventTypeID::NONE), payload_(nullptr) {}

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
