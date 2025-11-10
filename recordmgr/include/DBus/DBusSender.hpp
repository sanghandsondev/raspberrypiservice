#ifndef DBUS_SENDER_HPP_
#define DBUS_SENDER_HPP_

#include "DBusSenderBase.hpp"
#include "RMSenderFactory.hpp"

#define DBUS_SENDER() DBusSender::getInstance()

class DBusSender : public DBusSenderBase {
    public:
        static DBusSender *getInstance() {
            static DBusSender instance;
            return &instance;
        }
        DBusSender(const DBusSender &) = delete;
        DBusSender &operator=(const DBusSender &) = delete;

    private:
        DBusSender() : DBusSenderBase() {
            msgMaker = std::make_shared<RMSenderFactory>();
        };
        ~DBusSender() override = default;
};

#endif // DBUS_SENDER_HPP_