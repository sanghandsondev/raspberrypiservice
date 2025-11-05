#ifndef EVENT_TYPE_ID_HPP_
#define EVENT_TYPE_ID_HPP_

enum class EventTypeID {
    NONE = 0,
    STARTUP,
    ONOFF_LED,
    WS_MESSAGE_RECEIVED,
    WS_SEND_MESSAGE,

    MAX
};

#endif // EVENT_TYPE_ID_HPP_