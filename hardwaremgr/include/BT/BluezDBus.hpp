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
    void powerOnAdapter();
    void powerOffAdapter();

    DBusConnection* getConnection();
    void addMatchRule(const std::string& rule);
    DBusDataInfo parseDeviceProperties(DBusMessageIter *properties_iter);
    DBusDataInfo parseAdapterProperties(DBusMessageIter *properties_iter);
    const std::string& getAdapterPath() const;

private:
    DBusConnection* conn_;
    std::string adapterPath_;

    void findAdapter();
    bool parseManagedObjects(DBusMessageIter *iter);
    void setPower(bool on);
};

#endif // BLUEZ_DBUS_HPP_
