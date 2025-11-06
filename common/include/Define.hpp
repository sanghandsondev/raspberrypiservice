#ifndef DEFINE_HPP_
#define DEFINE_HPP_

enum class DBusCommand {
    NONE = 0,
    TURN_ON_LED = 1,
    TURN_OFF_LED,

    MAX
};

enum LogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,

    MAX
};

#endif // DEFINE_HPP_