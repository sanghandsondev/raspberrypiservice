#include "DBusReceiver.hpp"
#include "DBusClient.hpp"
#include "Config.hpp"
#include "RLogger.hpp"
#include <poll.h>       // poll()
#include <cstring>      // strerror

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
    const char* received_string = nullptr;

    // Khởi tạo lỗi
    dbus_error_init(&err);

    // 1. Đọc các tham số từ message
    //    - DBUS_TYPE_STRING: Chúng ta mong đợi nhận được một chuỗi.
    //    - &received_string: Con trỏ để lưu địa chỉ của chuỗi đọc được.
    //    - DBUS_TYPE_INVALID: Kết thúc danh sách các tham số cần đọc.
    if (!dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &received_string, DBUS_TYPE_INVALID)) {
        CM_LOG(ERROR, "DBusReceiver Error getting arguments: %s", err.message);
        dbus_error_free(&err);
        return;
    }

    // 2. Kiểm tra xem chuỗi có hợp lệ không và sử dụng nó
    if (received_string) {
        CM_LOG(INFO, "DBusReceiver Received string data: \"%s\"", received_string);

        // 3. (Quan trọng) Tạo một Event và đẩy vào hàng đợi
        //    Tạo một bản sao của chuỗi vì message gốc sẽ sớm bị giải phóng.
        // std::string payload = received_string;
        // auto event = std::make_shared<Event>(EventTypeID::DBUS_SIGNAL_RECEIVED, std::make_shared<std::string>(payload));
        // eventQueue_->pushEvent(event);

    } else {
        CM_LOG(WARN, "DBusReceiver Received signal but no string data found.");
    }
    
}