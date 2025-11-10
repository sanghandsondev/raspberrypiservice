#ifndef EVENT_TYPE_ID_HPP_
#define EVENT_TYPE_ID_HPP_

enum class EventTypeID {
    NONE = 0,
    STARTUP,


    ONOFF_LED,


    START_RECORD,
    STOP_RECORD,

    START_RECORD_NOTI,
    STOP_RECORD_NOTI,

    MAX
};

#endif // EVENT_TYPE_ID_HPP_