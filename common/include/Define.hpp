#ifndef DEFINE_HPP_
#define DEFINE_HPP_

enum class DBusCommand {
    NONE = 0,

    // Hardware
    TURN_ON_LED = 1,
    TURN_OFF_LED,

    // Record
    START_RECORD,
    STOP_RECORD,

    START_RECORD_NOTI,
    STOP_RECORD_NOTI,

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