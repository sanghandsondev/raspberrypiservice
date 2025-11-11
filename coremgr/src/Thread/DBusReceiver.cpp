#include "DBusReceiver.hpp"
#include "DBusClient.hpp"
#include "Config.hpp"
#include "CMLogger.hpp"
#include <poll.h>       // poll()
#include <cstring>      // strerror
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("DBusReceiver"), eventQueue_(eventQueue) {

    dbusClient_ = std::make_shared<DBusClient>(
        CONFIG_INSTANCE()->getCoreMgrServiceName(),
        CONFIG_INSTANCE()->getCoreMgrObjectPath(),
        CONFIG_INSTANCE()->getCoreMgrInterfaceName()
    );
    dbusClient_->addMatchRule(CONFIG_INSTANCE()->getCoreMgrSignalName());
}

DBusReceiver::~DBusReceiver() {}

void DBusReceiver::threadFunction() {
    CM_LOG(INFO, "DBusReceiver Thread function started using poll()");

    DBusConnection* conn = dbusClient_->getConnection();
    if (!conn) {
        CM_LOG(ERROR, "Failed to get D-Bus connection for polling.");
        return;
    }

    int fd;
    if (!dbus_connection_get_unix_fd(conn, &fd)) {
        CM_LOG(ERROR, "Failed to get D-Bus file descriptor.");
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
            CM_LOG(ERROR, "poll() error: %s", strerror(errno));
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
                if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getCoreMgrInterfaceName().c_str(), 
                                                CONFIG_INSTANCE()->getCoreMgrSignalName().c_str())) {
                    CM_LOG(INFO, "Received CoreManagerSignal via poll()");
                    dispatchMessage(msg);
                }
                dbus_message_unref(msg);
            }
        }
    }
}

void DBusReceiver::dispatchMessage(DBusMessage *msg){
    // TODO
    CM_LOG(INFO, "Dispatching message...");
    DBusError err;
    int32_t received_cmd = 0;

    // Khởi tạo lỗi
    dbus_error_init(&err);

    if (!dbus_message_get_args(msg, &err, DBUS_TYPE_INT32, &received_cmd, DBUS_TYPE_INVALID)) {
        CM_LOG(ERROR, "DBusReceiver Error getting arguments: %s", err.message);
        dbus_error_free(&err);
        return;
    }
    
    if(!received_cmd){
        CM_LOG(ERROR, "DBusReceiver Received signal but no command data found.");
        return;
    }

    CM_LOG(INFO, "DBusReceiver Received command: %d", static_cast<DBusCommand>(received_cmd));

    switch (static_cast<DBusCommand>(received_cmd)) {
        case DBusCommand::START_RECORD_NOTI: {
            CM_LOG(INFO, "Dispatching START_RECORD_NOTI from DBus");
            auto event = std::make_shared<Event>(EventTypeID::START_RECORD_NOTI);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::STOP_RECORD_NOTI: {
            CM_LOG(INFO, "Dispatching STOP_RECORD_NOTI from DBus");
            auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD_NOTI);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::TURN_ON_LED_NOTI: {
            CM_LOG(INFO, "Dispatching TURN_ON_LED_NOTI from DBus");
            auto event = std::make_shared<Event>(EventTypeID::TURN_ON_LED_NOTI);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::TURN_OFF_LED_NOTI: {
            CM_LOG(INFO, "Dispatching TURN_OFF_LED_NOTI from DBus");
            auto event = std::make_shared<Event>(EventTypeID::TURN_OFF_LED_NOTI);
            eventQueue_->pushEvent(event);
            break;
        }
            
        default:
            CM_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}