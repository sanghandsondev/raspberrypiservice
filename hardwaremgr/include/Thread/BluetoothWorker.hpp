#ifndef BLUETOOTH_WORKER_HPP_
#define BLUETOOTH_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"
#include <dbus/dbus.h>

class EventQueue;
class Event;
class Payload;
class BluezDBus;

class BluetoothWorker : public ThreadBase {
    public:
        explicit BluetoothWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus);
        ~BluetoothWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<BluezDBus> bluezDBus_;

        void threadFunction() override;
        void dispatchMessage(DBusMessage* msg);
        void handleInterfacesAdded(DBusMessage* msg);
        void handleInterfacesRemoved(DBusMessage* msg);
        void handlePropertiesChanged(DBusMessage* msg);
        std::string parseVariant(DBusMessageIter *iter);
};

#endif // BLUETOOTH_WORKER_HPP_