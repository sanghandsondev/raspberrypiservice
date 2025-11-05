#ifndef DBUS_COMMAND_HPP_
#define DBUS_COMMAND_HPP_

enum class DBusCommand {
    NONE = 0,
    TURN_ON_LED = 1,
    TURN_OFF_LED,

    MAX
};

#endif // DBUS_COMMAND_HPP_