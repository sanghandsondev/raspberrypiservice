#ifndef EVENT_TYPE_ID_HPP_
#define EVENT_TYPE_ID_HPP_

#include "Event.hpp"

enum class EventTypeID {
    NONE = 0,
    
    START_SCAN_BTDEVICE,
    STOP_SCAN_BTDEVICE,

    MAX
};

#endif // EVENT_TYPE_ID_HPP_