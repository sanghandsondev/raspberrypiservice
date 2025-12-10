#ifndef OFONO_DBUS_HPP_
#define OFONO_DBUS_HPP_

#include <string>
#include <memory>
#include <mutex>
#include <dbus/dbus.h>
#include "DBusData.hpp"
#include <unordered_map>

class OfonoDBus {
public:
    explicit OfonoDBus(DBusConnection* conn, std::recursive_mutex& mutex);
    ~OfonoDBus() = default;

    // oFono related methods
    void dial(const std::string& modemPath, const std::string& number);
    void answer(const std::string& callPath);
    void hangupAll(const std::string& modemPath);
    DBusDataInfo getVoiceCallProperties(const std::string& callPath);
    void setOfonoModemProperty(const std::string& modemPath, const std::string& property, bool value);
    void setOfonoPhonebookStorage(const std::string& modemPath, const std::string& storage);
    void getOfonoContacts(const std::string& modemPath);
    void syncAllOfonoContacts(const std::string& modemPath);
    void syncAllOfonoCallHistory(const std::string& modemPath);
    std::string findNameByNumber(const std::string& number);

private:
    DBusConnection* conn_;
    std::recursive_mutex& mutex_;
    std::unordered_map<std::string, std::string> phonebook_; // <Number, Name>

    void getOfonoContactDetails(const std::string& contactPath);
    void getOfonoCallHistory(const std::string& modemPath, const std::string& type);
    void getOfonoCallDetails(const std::string& callPath, const std::string& type);
};

#endif // OFONO_DBUS_HPP_
