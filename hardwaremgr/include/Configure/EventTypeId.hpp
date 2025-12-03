#ifndef EVENT_TYPE_ID_HPP_
#define EVENT_TYPE_ID_HPP_

#include "Event.hpp"

enum class EventTypeID {
    NONE = 0,
    
    START_SCAN_BTDEVICE,
    STOP_SCAN_BTDEVICE,
    BLUETOOTH_POWER_ON,
    BLUETOOTH_POWER_OFF,

    MAX
};

#endif // EVENT_TYPE_ID_HPP_