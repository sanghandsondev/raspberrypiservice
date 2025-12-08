#ifndef BLUETOOTH_WORKER_HPP_
#define BLUETOOTH_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"
#include <dbus/dbus.h>

class EventQueue;
class BluezDBus;
class BluetoothAgent;

class BluetoothWorker : public ThreadBase {
    public:
        explicit BluetoothWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus, std::shared_ptr<BluetoothAgent> agent);
        ~BluetoothWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<BluezDBus> bluezDBus_;
        std::shared_ptr<BluetoothAgent> agent_;

        void threadFunction() override;
        void dispatchMessage(DBusMessage* msg);
        void handleInterfacesAdded(DBusMessage* msg);
        void handleInterfacesRemoved(DBusMessage* msg);
        void handlePropertiesChanged(DBusMessage* msg);

        // oFono signal handlers
        void handleModemAdded(DBusMessage* msg);
        void handleModemRemoved(DBusMessage* msg);
        void handleOfonoPropertyChanged(DBusMessage* msg);
};

#endif // BLUETOOTH_WORKER_HPP_