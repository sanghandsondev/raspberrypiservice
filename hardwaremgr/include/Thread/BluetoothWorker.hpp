#ifndef BLUETOOTH_WORKER_HPP_
#define BLUETOOTH_WORKER_HPP_

#include <memory>
#include "ThreadBase.hpp"
#include <dbus/dbus.h>
#include <set>
#include <string>

class EventQueue;
class BluezDBus;
class BluetoothAgent;
class OfonoDBus;

class BluetoothWorker : public ThreadBase {
    public:
        explicit BluetoothWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus, std::shared_ptr<OfonoDBus> ofonoDBus, std::shared_ptr<BluetoothAgent> agent);
        ~BluetoothWorker() = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<BluezDBus> bluezDBus_;
        std::shared_ptr<OfonoDBus> ofonoDBus_;
        std::shared_ptr<BluetoothAgent> agent_;
        std::set<std::string> activeCallPaths_;

        void threadFunction() override;
        void dispatchMessage(DBusMessage* msg);
        void handleInterfacesAdded(DBusMessage* msg);
        void handleInterfacesRemoved(DBusMessage* msg);
        void handlePropertiesChanged(DBusMessage* msg);

        // oFono signal handlers
        void handleModemAdded(DBusMessage* msg);
        void handleModemRemoved(DBusMessage* msg);
        void handleOfonoPropertyChanged(DBusMessage* msg);
        void handleOfonoPropertiesChanged(DBusMessage* msg);
        void handleCallAdded(DBusMessage* msg);
        void handleVoiceCallPropertyChanged(DBusMessage* msg);
        void handleCallRemoved(DBusMessage* msg);
};

#endif // BLUETOOTH_WORKER_HPP_