#include "DBusReceiver.hpp"
#include "DBusClient.hpp"
#include "Config.hpp"
#include "HMLogger.hpp"
#include <poll.h>       // poll()
#include <cstring>      // strerror
#include "DBusSender.hpp"

DBusReceiver::DBusReceiver() : ThreadBase("DBusReceiver") {
    dbusClient_ = std::make_shared<DBusClient>(
        CONFIG_INSTANCE()->getServiceName(),
        CONFIG_INSTANCE()->getObjectPath(),
        CONFIG_INSTANCE()->getInterfaceName()
    );
    dbusClient_->addMatchRule(CONFIG_INSTANCE()->getSignalName());
}

DBusReceiver::~DBusReceiver() {}

void DBusReceiver::threadFunction() {
    HM_LOG(INFO, "DBusReceiver Thread function started using poll()");

    DBusConnection* conn = dbusClient_->getConnection();
    if (!conn) {
        HM_LOG(ERROR, "Failed to get D-Bus connection for polling.");
        return;
    }

    int fd;
    if (!dbus_connection_get_unix_fd(conn, &fd)) {
        HM_LOG(ERROR, "Failed to get D-Bus file descriptor.");
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
            HM_LOG(ERROR, "poll() error: %s", strerror(errno));
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
                if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getInterfaceName().c_str(), 
                                                CONFIG_INSTANCE()->getSignalName().c_str())) {
                    HM_LOG(INFO, "Received RecordManagerSignal via poll()");
                    dispatchMessage(msg);
                }
                dbus_message_unref(msg);
            }
        }
    }
}

void DBusReceiver::dispatchMessage(DBusMessage *msg){
    HM_LOG(INFO, "Dispatching message...");
    DBusError err;
    int32_t received_cmd = 0;

    dbus_error_init(&err);

    if (!dbus_message_get_args(msg, &err, DBUS_TYPE_INT32, &received_cmd, DBUS_TYPE_INVALID)) {
        HM_LOG(ERROR, "DBusReceiver Error getting arguments: %s", err.message);
        dbus_error_free(&err);
        return;
    }
    
    if(!received_cmd){
        HM_LOG(ERROR, "DBusReceiver Received signal but no command data found.");
        return;
    }

    HM_LOG(INFO, "DBusReceiver Received command: %d", static_cast<DBusCommand>(received_cmd));

    switch (static_cast<DBusCommand>(received_cmd)) {
        case DBusCommand::TURN_ON_LED:
            // TODO : Xử lý sự kiện START_RECORD
            DBUS_SENDER()->sendMessage(DBusCommand::TURN_ON_LED_NOTI);
            break;
        case DBusCommand::TURN_OFF_LED:
            // TODO : Xử lý sự kiện STOP_RECORD
            DBUS_SENDER()->sendMessage(DBusCommand::TURN_OFF_LED_NOTI);
            break;
        default:
            HM_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}