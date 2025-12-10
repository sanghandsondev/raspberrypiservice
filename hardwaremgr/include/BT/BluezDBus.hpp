#ifndef BLUEZ_DBUS_HPP_
#define BLUEZ_DBUS_HPP_

#include <string>
#include <memory>
#include <dbus/dbus.h>
#include "DBusData.hpp"
#include <unordered_map>

class BluezDBus {
public:
    BluezDBus();
    ~BluezDBus();

    void initializeAdapter();
    void startDiscovery();
    void stopDiscovery();
    void powerOnAdapter();
    void powerOffAdapter();
    void pairDevice(const std::string& address);
    void unpairDevice(const std::string& address);
    void registerAgent(const std::string& capability);
    void unregisterAgent();
    void connectDevice(const std::string& address);
    void disconnectDevice(const std::string& address);
    void trustDevice(const std::string& address);

    bool isAdapterFound() const;
    DBusConnection* getConnection();
    const std::string& getAdapterPath() const;
    void addMatchRule(const std::string& rule);
    DBusDataInfo parseAdapterProperties(DBusMessageIter *properties_iter);
    DBusDataInfo parseDeviceProperties(DBusMessageIter *properties_iter);
    DBusDataInfo getAllDeviceProperties(const std::string& objectPath);
    DBusDataInfo getAllAdapterProperties(const std::string& objectPath);
    void setDiscoverable(bool on);

private:
    DBusConnection* conn_;
    std::string adapterPath_;
    bool isInitialized_;
    std::unordered_map<std::string, std::string> phonebook_; // <Number, Name>

    bool parseManagedObjects(DBusMessageIter *iter);
    void setPower(bool on);
    std::string deviceAddressToObjectPath(const std::string& address) const;
};

#endif // BLUEZ_DBUS_HPP_
