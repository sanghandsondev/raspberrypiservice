#ifndef OFONO_DBUS_HPP_
#define OFONO_DBUS_HPP_

#include <string>
#include <memory>
#include <mutex>
#include <dbus/dbus.h>
#include "DBusData.hpp"
#include <unordered_map>
#include <set>

/**
 * OfonoDBus — handles oFono HFP (Hands-Free Profile) voice call functionality.
 * 
 * oFono manages cellular telephony over Bluetooth HFP. When a phone connects
 * via Bluetooth, oFono detects the modem and allows dialing/answering/hanging up calls.
 *
 * NOTE: oFono does NOT support PBAP. Phonebook and call history are pulled
 * via ObexPbapClient (obexd PBAP) instead.
 */
class OfonoDBus {
public:
    explicit OfonoDBus(DBusConnection* conn, std::recursive_mutex& mutex);
    ~OfonoDBus() = default;

    // Voice call methods
    void dialCall(const std::string& number);
    void answerCall();
    void hangupCall();
    DBusDataInfo getVoiceCallProperties(const std::string& callPath);
    void setOfonoModemProperty(const std::string& modemPath, const std::string& property, bool value);

    // Contact name lookup (populated by ObexPbapClient)
    std::string findNameByNumber(const std::string& number);
    void setPhonebook(const std::unordered_map<std::string, std::string>& phonebook);
    void clearPhonebook();

    // State management
    void setActiveModemPath(const std::string& modemPath);
    void clearActiveModemPath();
    const std::string& getActiveModemPath() const;
    void addActiveCallPath(const std::string& callPath);
    void removeActiveCallPath(const std::string& callPath);
    const std::set<std::string>& getActiveCallPaths() const;

private:
    DBusConnection* conn_;
    std::recursive_mutex& mutex_;
    std::string modemPath_;
    std::set<std::string> activeCallPaths_;
    std::unordered_map<std::string, std::string> phonebook_; // <Number, Name> — populated by PBAP
};

#endif // OFONO_DBUS_HPP_
