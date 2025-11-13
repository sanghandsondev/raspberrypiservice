#ifndef DEFINE_HPP_
#define DEFINE_HPP_

enum class DBusCommand {
    NONE = 0,

    // Hardware
    TURN_ON_LED = 1,
    TURN_OFF_LED,

    TURN_ON_LED_NOTI,
    TURN_OFF_LED_NOTI,

    // Record
    START_RECORD,
    STOP_RECORD,

    START_RECORD_NOTI,
    STOP_RECORD_NOTI,
    FILTER_WAV_FILE_NOTI,

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