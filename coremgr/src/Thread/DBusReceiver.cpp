#include "DBusReceiver.hpp"
#include "DBusClient.hpp"
#include "Config.hpp"

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("DBusReceiver"), eventQueue_(eventQueue) {
    dbusClient_ = std::make_shared<DBusClient>(
        CONFIG_INSTANCE()->getCoreMgrServiceName(),
        CONFIG_INSTANCE()->getCoreMgrObjectPath(),
        CONFIG_INSTANCE()->getCoreMgrInterfaceName()
    );

    dbusClient_->addMatchRule("CoreManagerSignal");
}

DBusReceiver::~DBusReceiver() {}

void DBusReceiver::threadFunction() {
    printf("[DBusReceiver] Thread function started\n");

    while (runningFlag_) {
        DBusMessage* msg = dbusClient_->waitForAndProcessSignal();
        if (msg) {
            // Check message type and dispatch accordingly
            if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getCoreMgrInterfaceName().c_str(), "CoreManagerSignal")) {
                printf("[DBusReceiver] Received CoreManagerSignal\n");
                dispatchMessage(msg);
            } else {
                printf("[DBusReceiver] Received unknown signal\n");
            }

            // Free the message after processing
            dbus_message_unref(msg);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));    // Polling delay
        }
    }

    printf("[DBusReceiver] Thread function exiting\n");
}

void DBusReceiver::dispatchMessage(DBusMessage *msg){
    // TODO
    printf("[DBusReceiver] Dispatching message...\n");
    DBusError err;
    const char* received_string = nullptr;

    // Khởi tạo lỗi
    dbus_error_init(&err);

    // 1. Đọc các tham số từ message
    //    - DBUS_TYPE_STRING: Chúng ta mong đợi nhận được một chuỗi.
    //    - &received_string: Con trỏ để lưu địa chỉ của chuỗi đọc được.
    //    - DBUS_TYPE_INVALID: Kết thúc danh sách các tham số cần đọc.
    if (!dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &received_string, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "[DBusReceiver] Error getting arguments: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    // 2. Kiểm tra xem chuỗi có hợp lệ không và sử dụng nó
    if (received_string) {
        printf("[DBusReceiver] Received string data: \"%s\"\n", received_string);

        // 3. (Quan trọng) Tạo một Event và đẩy vào hàng đợi
        //    Tạo một bản sao của chuỗi vì message gốc sẽ sớm bị giải phóng.
        // std::string payload = received_string;
        // auto event = std::make_shared<Event>(EventTypeID::DBUS_SIGNAL_RECEIVED, std::make_shared<std::string>(payload));
        // eventQueue_->pushEvent(event);

    } else {
        printf("[DBusReceiver] Received signal but no string data found.\n");
    }
    
}