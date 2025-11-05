#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For sleep

int main() {
    DBusConnection* conn;
    DBusError err;
    DBusMessage* msg;
    dbus_uint32_t serial = 0;

    // Khởi tạo lỗi
    dbus_error_init(&err);

    // 1. Kết nối tới SYSTEM bus, giống như CoreManager
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        return 1;
    }
    if (conn == NULL) {
        fprintf(stderr, "Connection Null\n");
        return 1;
    }

    // 2. Tạo một signal với đúng các thông số mà CoreManager đang lắng nghe
    msg = dbus_message_new_signal(
        "/com/example/coremanager",          // object path
        "com.example.coremanager.interface",  // interface name
        "CoreManagerSignal"                  // signal name
    );
    if (msg == NULL) {
        fprintf(stderr, "Message Null\n");
        return 1;
    }

    // (Tùy chọn) Thêm tham số vào signal nếu muốn.
    // Ví dụ, gửi một chuỗi "Hello from testmgr"
    const char* my_string = "Hello from testmgr";
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &my_string, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "Out of Memory!\n");
        return 1;
    }


    // 3. Gửi signal
    if (!dbus_connection_send(conn, msg, &serial)) {
        fprintf(stderr, "Out Of Memory!\n");
        return 1;
    }
    dbus_connection_flush(conn);

    printf("Signal 'CoreManagerSignal' sent!\n");

    // 4. Giải phóng tài nguyên
    dbus_message_unref(msg);
    dbus_connection_unref(conn);

    return 0;
}