#ifndef BLUEZ_DBUS_HPP_
#define BLUEZ_DBUS_HPP_

#include <string>
#include <memory>
#include <dbus/dbus.h>
#include "DBusData.hpp"

class BluezDBus {
public:
    BluezDBus();
    ~BluezDBus();

    void startDiscovery();
    void stopDiscovery();

    DBusConnection* getConnection();
    void addMatchRule(const std::string& rule);
    DBusDataInfo parseDeviceProperties(DBusMessageIter *properties_iter);

private:
    DBusConnection* conn_;
    std::string adapterPath_;

    void findAdapter();
    bool parseManagedObjects(DBusMessageIter *iter);
};

#endif // BLUEZ_DBUS_HPP_
