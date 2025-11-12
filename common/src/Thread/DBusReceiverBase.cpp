#include "DBusReceiverBase.hpp"
#include "DBusClient.hpp"
#include "Logger.hpp"
#include "Define.hpp"
#include <poll.h>
#include <cerrno>
#include <cstring>

DBusReceiverBase::DBusReceiverBase(const std::string& serviceName, const std::string& objectPath, const std::string& interfaceName, const std::string& signalName)
    : ThreadBase("DBusReceiver"),
      interfaceName_(interfaceName),
      signalName_(signalName)
{
    dbusClient_ = std::make_shared<DBusClient>(
        serviceName,
        objectPath,
        interfaceName
    );
    dbusClient_->addMatchRule(signalName);
}

void DBusReceiverBase::threadFunction() {
    CMN_LOG(INFO, "DBusReceiverBase Thread function started using poll()");

    DBusConnection* conn = dbusClient_->getConnection();
    if (!conn) {
        CMN_LOG(ERROR, "Failed to get D-Bus connection for polling.");
        return;
    }

    int fd;
    if (!dbus_connection_get_unix_fd(conn, &fd)) {
        CMN_LOG(ERROR, "Failed to get D-Bus file descriptor.");
        return;
    }

    while (runningFlag_) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN; // Chỉ quan tâm sự kiện "có dữ liệu để đọc"
        pfd.revents = 0;

        // Gọi poll() để chờ sự kiện.
        // Luồng sẽ bị block ở đây cho đến khi có dữ liệu hoặc hết timeout.
        // Timeout 500ms để vòng lặp có thể kiểm tra runningFlag_ định kỳ.
        int ret = poll(&pfd, 1, 500);

        if (ret < 0) {
            CMN_LOG(ERROR, "poll() error: %s", strerror(errno));
            continue;
        }

        if (ret == 0) { // Timeout, không có sự kiện nào
            continue;
        }

        // Nếu có sự kiện (ret > 0) và đó là sự kiện đọc (POLLIN)
        if (pfd.revents & POLLIN) {
            // Yêu cầu libdbus đọc dữ liệu từ buffer nội bộ
            dbus_connection_read_write(conn, 0);

            DBusMessage* msg = nullptr;
            // Lấy các message đã được xử lý ra
            while ((msg = dbus_connection_pop_message(conn)) != nullptr) {
                if (dbus_message_is_signal(msg, interfaceName_.c_str(), 
                                                signalName_.c_str())) {
                    CMN_LOG(INFO, "Received Signal via poll()");
                    dispatchMessage(msg);
                }
                dbus_message_unref(msg);
            }
        }
    }
    CMN_LOG(INFO, "DBusReceiverBase Thread function exiting.");
}

void DBusReceiverBase::dispatchMessage(DBusMessage* msg) {
    const char* signature = dbus_message_get_signature(msg);
    if (signature == nullptr) {
        CMN_LOG(WARN, "Received D-Bus message with no signature.");
        return;
    }

    DBusError err;
    dbus_error_init(&err);

    // So sánh chữ ký để xác định loại message
    if (strcmp(signature, "i") == 0) {
        // Đây là message thường (chỉ có command)
        int32_t received_cmd = 0;
        if (dbus_message_get_args(msg, &err, DBUS_TYPE_INT32, &received_cmd, DBUS_TYPE_INVALID)) {
            CMN_LOG(INFO, "Dispatching command: %d", received_cmd);
            handleMessage(static_cast<DBusCommand>(received_cmd));
        } else {
            CMN_LOG(ERROR, "Failed to parse command message: %s", err.message);
        }
    } else if (strcmp(signature, "ibs") == 0) {
        // Đây là message notification (command, success, info)
        int32_t received_cmd = 0;
        dbus_bool_t isSuccess = false;
        const char* msgInfo_cstr = nullptr; // Dùng con trỏ char* để nhận dữ liệu

        if (dbus_message_get_args(msg, &err, 
                                     DBUS_TYPE_INT32, &received_cmd, 
                                     DBUS_TYPE_BOOLEAN, &isSuccess, 
                                     DBUS_TYPE_STRING, &msgInfo_cstr,  // Truyền địa chỉ của con trỏ
                                     DBUS_TYPE_INVALID)) {
            // Khởi tạo std::string từ con trỏ char* một cách an toàn
            std::string msgInfo = (msgInfo_cstr) ? msgInfo_cstr : "";
            
            CMN_LOG(INFO, "Dispatching notification: cmd=%d, success=%d, msg=%s",
                  received_cmd, isSuccess, msgInfo.c_str());
            handleMessageNoti(static_cast<DBusCommand>(received_cmd), isSuccess, msgInfo);
        } else {
            CMN_LOG(ERROR, "Failed to parse notification message: %s", err.message);
        }
    } else {
        CMN_LOG(WARN, "Received D-Bus message with unknown signature: %s", signature);
    }

    // Luôn giải phóng tài nguyên của DBusError ở cuối hàm
    dbus_error_free(&err);
}
