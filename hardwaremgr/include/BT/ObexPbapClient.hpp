#ifndef OBEX_PBAP_CLIENT_HPP_
#define OBEX_PBAP_CLIENT_HPP_

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <dbus/dbus.h>
#include "DBusData.hpp"

/**
 * ObexPbapClient — PBAP (Phone Book Access Profile) client using obexd (BlueZ OBEX daemon).
 * 
 * This class connects to the obexd session D-Bus service (org.bluez.obex) 
 * to pull phonebook contacts and call history from a connected Bluetooth phone.
 *
 * Required: obexd must be running (usually started by BlueZ automatically).
 *   - Install: sudo apt install obex-data-server   (or bluez-obex)
 *   - Service: /usr/lib/bluetooth/obexd --session (auto-started on session bus)
 *
 * PBAP standard paths (on the phone):
 *   - "internal/telecom/pb.vcf"    → Phone contacts
 *   - "internal/telecom/ich.vcf"   → Incoming call history
 *   - "internal/telecom/och.vcf"   → Outgoing call history
 *   - "internal/telecom/mch.vcf"   → Missed call history
 *   - "internal/telecom/cch.vcf"   → Combined call history
 *   - "SIM1/telecom/pb.vcf"        → SIM contacts
 */

struct VCardContact {
    std::string name;
    std::string number;
};

struct VCardCallHistory {
    std::string name;
    std::string number;
    std::string type;       // "received", "dialed", "missed"
    std::string datetime;
};

class ObexPbapClient {
public:
    explicit ObexPbapClient(std::recursive_mutex& mutex);
    ~ObexPbapClient();

    /**
     * Create a PBAP session to a connected Bluetooth device.
     * @param deviceAddress The BT address of the phone (e.g., "AA:BB:CC:DD:EE:FF")
     * @return true if session created successfully
     */
    bool createSession(const std::string& deviceAddress);

    /**
     * Close the current PBAP session.
     */
    void removeSession();

    /**
     * Pull phone contacts (pb.vcf). 
     * Sends PBAP_PHONEBOOK_PULL_NOTI for each contact and start/end notifications.
     * @return The parsed contacts list (for caller ID lookup).
     */
    std::vector<VCardContact> pullPhonebook();

    /**
     * Pull call history (incoming, outgoing, missed).
     * Sends CALL_HISTORY_PULL_NOTI for each entry and start/end notifications.
     */
    void pullCallHistory();

    /**
     * Check if a PBAP session is currently active.
     */
    bool hasSession() const;

    /**
     * Get the device address of the current session.
     */
    const std::string& getSessionDeviceAddress() const;

private:
    DBusConnection* sessionConn_;   // Session bus connection (obexd runs on session bus)
    std::recursive_mutex& mutex_;
    std::string sessionPath_;       // Current OBEX session object path
    std::string deviceAddress_;     // Current connected device address

    /**
     * Pull a phonebook object (VCF) from the phone at the given path.
     * @param pbPath The PBAP path, e.g., "internal/telecom/pb.vcf"
     * @return The VCF data as a string, or empty string on failure.
     */
    std::string pullPhonebookObject(const std::string& pbPath);

    /**
     * Select the phonebook folder before pulling.
     * @param folder The folder path, e.g., "internal" or "SIM1"
     * @param subfolder The subfolder, e.g., "telecom"
     */
    bool selectPhonebook(const std::string& folder, const std::string& subfolder);

    /**
     * Wait for a transfer to complete and return the transferred file path.
     * @param transferPath The D-Bus object path of the transfer
     * @return The file path of the transferred data, or empty string on failure.
     */
    std::string waitForTransfer(const std::string& transferPath);

    /**
     * Read a file's contents into a string.
     */
    std::string readFileContents(const std::string& filePath);

    /**
     * Parse VCF (vCard) data into contacts.
     */
    std::vector<VCardContact> parseVcfContacts(const std::string& vcfData);

    /**
     * Parse VCF call history data.
     * @param vcfData The raw VCF text
     * @param type The call type label ("received", "dialed", "missed")
     */
    std::vector<VCardCallHistory> parseVcfCallHistory(const std::string& vcfData, const std::string& type);
};

#endif // OBEX_PBAP_CLIENT_HPP_
