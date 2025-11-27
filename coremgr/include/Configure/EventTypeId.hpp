#ifndef EVENT_TYPE_ID_HPP_
#define EVENT_TYPE_ID_HPP_

#include "Event.hpp"

enum class EventTypeID {
    NONE = 0,
    STARTUP,

    // Hardware
    UPDATE_TEMPERATURE_NOTI,

    // Record
    START_RECORD,
    STOP_RECORD,
    CANCEL_RECORD,
    REMOVE_RECORD,
    GET_ALL_RECORD,

    START_RECORD_NOTI,
    STOP_RECORD_NOTI,
    CANCEL_RECORD_NOTI,
    FILTER_WAV_FILE_NOTI,
    INSERT_WAV_FILE,

    MAX
};

#endif // EVENT_TYPE_ID_HPP_