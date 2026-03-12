# 🐧 Linux System Features - RaspberryPiService Project
## Tổng hợp các tính năng Linux System Programming để ôn phỏng vấn

> **Project Architecture:** 3 Linux services (daemons) giao tiếp qua D-Bus IPC  
> `CoreManager` ↔ `HardwareManager` ↔ `RecordManager`

---

## 📑 MỤC LỤC
1. [D-Bus IPC (Inter-Process Communication)](#1-d-bus-ipc-inter-process-communication)
2. [poll() - I/O Multiplexing](#2-poll---io-multiplexing)
3. [Multithreading (std::thread)](#3-multithreading-stdthread)
4. [Mutex & Lock (std::mutex, std::recursive_mutex, std::lock_guard, std::unique_lock)](#4-mutex--lock)
5. [Condition Variable (std::condition_variable)](#5-condition-variable-stdcondition_variable)
6. [Atomic Operations (std::atomic)](#6-atomic-operations-stdatomic)
7. [Signal Handling (SIGINT, SIGTERM)](#7-signal-handling-sigint-sigterm)
8. [systemd Service (Daemon)](#8-systemd-service-daemon)
9. [systemd Watchdog](#9-systemd-watchdog)
10. [systemd sd_notify](#10-systemd-sd_notify)
11. [D-Bus Security Policy (busconfig)](#11-d-bus-security-policy-busconfig)
12. [Socket / WebSocket (TCP)](#12-socket--websocket-tcp)
13. [Boost.Asio - Asynchronous I/O](#13-boostasio---asynchronous-io)
14. [ALSA (Advanced Linux Sound Architecture)](#14-alsa---advanced-linux-sound-architecture)
15. [GPIO / sysfs - Temperature Sensor (1-Wire)](#15-gpio--sysfs---temperature-sensor-1-wire)
16. [SQLite Database (File-based)](#16-sqlite-database-file-based)
17. [Thread Pool Pattern](#17-thread-pool-pattern)
18. [std::future / std::packaged_task (Asynchronous Tasks)](#18-stdfuture--stdpackaged_task)
19. [Producer-Consumer Pattern (Event Queue)](#19-producer-consumer-pattern-event-queue)
20. [File I/O (std::ifstream, std::ofstream)](#20-file-io-stdifstream-stdofstream)
21. [std::filesystem (C++17)](#21-stdfilesystem-c17)
22. [Directory Operations (opendir/readdir/closedir)](#22-directory-operations-opendirreaddirclosedir)
23. [Process ID (getpid)](#23-process-id-getpid)
24. [system() - Spawning Child Process](#24-system---spawning-child-process)
25. [Cross-Compilation (aarch64 Toolchain)](#25-cross-compilation-aarch64-toolchain)
26. [Singleton Pattern (std::call_once, std::once_flag)](#26-singleton-pattern-stdcall_once-stdonce_flag)
27. [Variadic Functions (va_list, va_start, va_end)](#27-variadic-functions-va_list-va_start-va_end)
28. [std::thread::detach() - Detached Thread](#28-stdthreaddetach---detached-thread)
29. [Bluetooth Stack (BlueZ D-Bus API)](#29-bluetooth-stack-bluez-d-bus-api)
30. [oFono Telephony Stack (HFP Voice Calls via D-Bus)](#30-ofono-telephony-stack)
31. [BlueZ Agent (Bluetooth Pairing Agent)](#31-bluez-agent-bluetooth-pairing-agent)
32. [D-Bus Message Iterator (Serialization/Deserialization)](#32-d-bus-message-iterator)
33. [Timer Mechanism (Software Timer)](#33-timer-mechanism-software-timer)
34. [fflush(stdout) - Immediate Output](#34-fflushstdout---immediate-output)
35. [OBEX PBAP (Phone Book Access Profile via obexd)](#35-obex-pbap-phone-book-access-profile-via-obexd)

---

## 1. D-Bus IPC (Inter-Process Communication)

**Khái niệm:** D-Bus là hệ thống message bus cho phép các process giao tiếp với nhau trên Linux. Project sử dụng **System Bus** để giao tiếp giữa 3 services.

**Nơi sử dụng:** `common/src/DBus/DBusClient.cpp`, `DBusSenderBase.cpp`, `DBusReceiverBase.cpp`

### 1a. Kết nối tới System Bus
```cpp
// DBusClient.cpp
conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
```

### 1b. Request Well-Known Name (đăng ký tên service)
```cpp
// DBusClient.cpp
int ret = dbus_bus_request_name(conn_, serviceName_.c_str(), 
                                 DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
```

### 1c. Gửi Signal (broadcast message)
```cpp
// ISenderFactory.cpp
DBusMessage* msg = dbus_message_new_signal(objectpath, interface, signal);
dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID);
```

### 1d. Gửi message qua connection
```cpp
// DBusSenderBase.cpp
dbus_connection_send(conn_, msg, &serial);
dbus_connection_flush(conn_);
```

### 1e. Add Match Rule (đăng ký lắng nghe signal)
```cpp
// DBusClient.cpp
std::string rule = "type='signal',interface='" + interfaceName_ + "',member='" + signalName + "'";
dbus_bus_add_match(conn_, rule.c_str(), &err);
```

### 1f. D-Bus Method Call (gọi hàm từ xa - RPC)
```cpp
// BluezDBus.cpp
DBusMessage* msg = dbus_message_new_method_call(
    "org.bluez", adapterPath_.c_str(),
    "org.bluez.Adapter1", "StartDiscovery");
DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
```

### 1g. Private Connection (không chia sẻ với process khác)
```cpp
// BluezDBus.cpp
conn_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
```

---

## 2. poll() - I/O Multiplexing

**Khái niệm:** `poll()` là system call cho phép thread chờ I/O event trên file descriptor mà không busy-wait. Hiệu quả hơn busy-loop, tương tự `epoll` nhưng đơn giản hơn.

**Nơi sử dụng:** `common/src/Thread/DBusReceiverBase.cpp`, `hardwaremgr/src/Thread/BluetoothWorker.cpp`

```cpp
// DBusReceiverBase.cpp
int fd;
dbus_connection_get_unix_fd(conn, &fd); // Lấy file descriptor từ D-Bus connection

struct pollfd pfd;
pfd.fd = fd;
pfd.events = POLLIN;  // Chỉ quan tâm sự kiện "có dữ liệu để đọc"
pfd.revents = 0;

int ret = poll(&pfd, 1, 500); // Block tối đa 500ms

if (ret > 0 && (pfd.revents & POLLIN)) {
    dbus_connection_read_write(conn, 0);
    DBusMessage* msg = dbus_connection_pop_message(conn);
    // Xử lý message...
}
```

**Điểm phỏng vấn:** So sánh `poll()` vs `select()` vs `epoll()`. `poll()` không giới hạn số fd như `select()`, nhưng `epoll()` hiệu quả hơn với số lượng fd lớn (O(1) vs O(n)).

---

## 3. Multithreading (std::thread)

**Khái niệm:** Tạo và quản lý nhiều thread chạy song song. Project sử dụng pattern **ThreadBase** (abstract class) để chuẩn hóa lifecycle của thread.

**Nơi sử dụng:** `common/src/Thread/ThreadBase.cpp` - base class cho tất cả thread

```cpp
// ThreadBase.cpp
void ThreadBase::run() {
    if (!runningFlag_) {
        runningFlag_ = true;
        threadObj_ = std::thread(&ThreadBase::threadFunction, this);
    }
}

void ThreadBase::join() {
    if (threadObj_.joinable()) {
        threadObj_.join();
    }
}
```

**Các thread trong project:**
| Service | Thread | Chức năng |
|---------|--------|-----------|
| CoreManager | MainWorker | Xử lý event chính |
| CoreManager | DBusReceiver | Nhận D-Bus signal |
| CoreManager | WebSocket | WebSocket server |
| CoreManager | DBThreadPool (5 workers) | DB operations |
| CoreManager | Timer | Software timer |
| HardwareManager | MainWorker | Xử lý event chính |
| HardwareManager | DBusReceiver | Nhận D-Bus signal |
| HardwareManager | MonitorWorker | Đọc nhiệt độ sensor |
| HardwareManager | BluetoothWorker | Nhận BlueZ/oFono signal |
| RecordManager | MainWorker | Xử lý event chính |
| RecordManager | DBusReceiver | Nhận D-Bus signal |
| RecordManager | RecordWorker | Capture audio ALSA |

---

## 4. Mutex & Lock

**Khái niệm:** Mutex đảm bảo chỉ 1 thread truy cập critical section tại một thời điểm. Project sử dụng nhiều loại lock khác nhau.

### 4a. std::mutex + std::lock_guard (RAII auto-unlock)
```cpp
// EventQueue.cpp
bool EventQueue::pushEvent(const std::shared_ptr<Event> event) {
    std::lock_guard<std::mutex> lk(queueMutex_);  // Auto unlock khi ra khỏi scope
    eventList_.push_back(event);
    notifyEvent();
    return true;
}
```

### 4b. std::unique_lock (có thể unlock trước khi ra scope)
```cpp
// DBThreadPool.cpp
void DBThreadPool::threadFunction() {
    while (runningFlag_) {
        std::unique_lock<std::mutex> lock(taskMutex_);
        cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });
        auto task = std::move(tasksQueue_.front());
        tasksQueue_.pop();
        lock.unlock();  // Unlock sớm trước khi execute task
        task();
    }
}
```

### 4c. std::recursive_mutex (cho phép cùng 1 thread lock nhiều lần)
```cpp
// BluezDBus.cpp - Bluetooth operations có thể gọi nested
void BluezDBus::startDiscovery() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // ... code gọi các hàm khác cũng cần lock cùng mutex
}
```

### 4d. Mutex bảo vệ Logger (thread-safe logging)
```cpp
// Logger.cpp
void Logger::printLog(...) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    printf("[%s][%d]...", ...);
    fflush(stdout);
}
```

### 4e. Mutex bảo vệ SQLite Database
```cpp
// SQLiteDatabase.cpp
bool SQLiteDatabase::open() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    sqlite3_open(dbFilePath_.c_str(), &db_);
    return initializeSchema();
}
```

### 4f. Mutex bảo vệ WebSocket sessions
```cpp
// WebSocketServer.cpp
void WebSocketServer::join(std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
}
```

### 4g. Mutex bảo vệ message queue (write serialization)
```cpp
// WebSocketServer.cpp - WebSocketSession
void WebSocketSession::send(const std::string& message) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    message_queue_.push(message);
    if (!writing_) {
        writing_ = true;
        asio::post(ws_.get_executor(), std::bind(&WebSocketSession::doWrite, shared_from_this()));
    }
}
```

---

## 5. Condition Variable (std::condition_variable)

**Khái niệm:** Cho phép thread chờ (block) cho đến khi một điều kiện được thỏa mãn, tránh busy-waiting. Luôn dùng cùng `std::mutex`.

### 5a. wait_for() - chờ event với timeout
```cpp
// EventQueue.cpp
void EventQueue::waitForEvent(const uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lk_cv(wakeupMutex_);
    wakeupCondition_.wait_for(lk_cv, std::chrono::milliseconds(timeout_ms));
}
```

### 5b. wait() với predicate - chờ cho đến khi condition = true
```cpp
// DBThreadPool.cpp - Worker thread chờ task
cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });
```

### 5c. wait() trong RecordWorker - chờ lệnh start recording
```cpp
// RecordWorker.cpp
std::unique_lock<std::mutex> lock(mtx_);
cv_.wait(lock, [this]{ return state_ == State::RECORDING || !runningFlag_; });
```

### 5d. wait_until() trong Timer - chờ đến thời điểm expire
```cpp
// Timer.cpp
timerWakeupCondition.wait_until(lock, nextItemTimeout, [this, mapSize]() -> bool {
    return (!runningFlag_ || timerTableMap.size() != mapSize);
});
```

### 5e. notify_one() / notify_all()
```cpp
// EventQueue.cpp
wakeupCondition_.notify_all();  // Đánh thức tất cả thread đang chờ

// main.cpp (signal handler)
g_cv.notify_one();  // Đánh thức main thread để graceful shutdown
```

---

## 6. Atomic Operations (std::atomic)

**Khái niệm:** Biến atomic đảm bảo read/write là indivisible, không cần mutex cho các thao tác đơn giản.

### 6a. atomic<bool> cho running flag
```cpp
// ThreadBase.hpp
std::atomic<bool> runningFlag_;

// Trong threadFunction, đọc không cần lock:
while (runningFlag_) { ... }
```

### 6b. Global running flag cho main()
```cpp
// main.cpp (tất cả services)
std::atomic<bool> g_runningFlag;

void signalHandler(int signum) {
    g_runningFlag = false;     // atomic write, thread-safe
    g_cv.notify_one();
}
```

### 6c. Atomic state trong RecordWorker
```cpp
// RecordWorker.hpp
std::atomic<State> state_;
std::atomic<bool> cancelRequested_;

// Check state mà không cần lock:
while (runningFlag_ && state_ == State::RECORDING) { ... }
```

---

## 7. Signal Handling (SIGINT, SIGTERM)

**Khái niệm:** Bắt tín hiệu từ OS để graceful shutdown. `SIGINT` = Ctrl+C, `SIGTERM` = `kill` command / systemd stop.

**Nơi sử dụng:** `main.cpp` của cả 3 services

```cpp
// coremgr/src/main.cpp
void signalHandler(int signum) {
    R_LOG(WARN, "Interrupt Signal (%d).", signum);
    g_runningFlag = false;
    g_cv.notify_one();  // Đánh thức main thread
}

int main(){
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // systemctl stop / kill
    // ...
}
```

**Graceful Shutdown Flow:**
```
SIGTERM → signalHandler() → g_runningFlag = false → notify main thread
→ stop all threads → join all threads → exit
```

---

## 8. systemd Service (Daemon)

**Khái niệm:** Các service chạy dưới dạng Linux daemon, quản lý bởi systemd. Tự động start khi boot, restart khi crash.

**Nơi sử dụng:** `.service` files

```ini
# coremanager.service
[Unit]
Description=Core Manager Service for Raspberry Pi
After=network-online.target dbus.socket
Requires=dbus.socket

[Service]
ExecStart=/usr/local/bin/coremanager
User=pi
Restart=on-failure
RestartSec=5s
StartLimitBurst=5
Type=notify          # Service phải gọi sd_notify(READY=1)

[Install]
WantedBy=multi-user.target
```

**Service dependency:**
```
coremanager.service  ← (After) ←  hardwaremanager.service
                     ← (After) ←  recordmanager.service
```

---

## 9. systemd Watchdog

**Khái niệm:** systemd giám sát service bằng watchdog. Service phải gửi heartbeat (`WATCHDOG=1`) định kỳ, nếu không systemd sẽ restart service.

**Nơi sử dụng:** `main.cpp` của cả 3 services

```ini
# Trong .service file:
WatchdogSec=10    # Service phải ping trong vòng 10 giây
```

```cpp
// main.cpp
#define INTERNAL_WATCHDOG_STANDARD_MS  10000

while(g_runningFlag) {
    std::unique_lock<std::mutex> lk(g_mutex);
    g_cv.wait_for(lk, std::chrono::milliseconds(INTERNAL_WATCHDOG_STANDARD_MS));
    
    if (!g_runningFlag) break;
    
    sd_notify(0, "WATCHDOG=1");  // Gửi heartbeat cho systemd
}
```

---

## 10. systemd sd_notify

**Khái niệm:** `sd_notify()` thông báo cho systemd rằng service đã sẵn sàng (`READY=1`). Dùng với `Type=notify` trong service file.

**Nơi sử dụng:** `main.cpp` của cả 3 services

```cpp
#include <systemd/sd-daemon.h>

int main(){
    // ... initialization ...
    sd_notify(0, "READY=1");   // Thông báo systemd service đã ready
    // ... main loop with WATCHDOG=1 ...
}
```

---

## 11. D-Bus Security Policy (busconfig)

**Khái niệm:** File cấu hình D-Bus quyết định user nào được phép own (đăng ký) và gửi message đến service.

**Nơi sử dụng:** `.conf` files (ví dụ `com.example.coremanager.conf`)

```xml
<busconfig>
    <policy user="pi">
        <allow own="com.example.coremanager"/>
        <allow send_destination="com.example.coremanager"/>
    </policy>
    <policy context="default">
        <allow send_destination="com.example.coremanager"/>
    </policy>
</busconfig>
```

**Đặt tại:** `/etc/dbus-1/system.d/` trên target

---

## 12. Socket / WebSocket (TCP)

**Khái niệm:** WebSocket server chạy trên TCP socket, cho phép client (web app) giao tiếp real-time với CoreManager.

**Nơi sử dụng:** `coremgr/src/WS/WebSocketServer.cpp`

```cpp
// WebSocketServer.cpp - Lắng nghe kết nối TCP
WebSocketServer::WebSocketServer(const std::string& host, unsigned short port, MessageHandler handler)
    : acceptor_(io_, tcp::endpoint(asio::ip::make_address(host), port)), 
      messageHandler_(handler) {}

// Config: host = "0.0.0.0", port = 9000 (listen on all interfaces)
```

**Session management:**
```cpp
std::set<std::shared_ptr<WebSocketSession>> sessions_;  // Track connected clients

void WebSocketServer::broadcast(const std::string& message) {
    for (auto& session : sessions_) {
        session->send(message);
    }
}
```

---

## 13. Boost.Asio - Asynchronous I/O

**Khái niệm:** Framework asynchronous I/O. Sử dụng event loop (`io_context`) và callback pattern để xử lý I/O không đồng bộ.

**Nơi sử dụng:** `coremgr/src/WS/WebSocketServer.cpp`

### 13a. Async Accept
```cpp
void WebSocketServer::doAccept(){
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if(!ec) {
                std::make_shared<WebSocketSession>(std::move(socket), *this)->start();
            }
            doAccept();  // Accept tiếp client mới (recursive async)
        });
}
```

### 13b. Async Read
```cpp
void WebSocketSession::doRead(){
    ws_.async_read(buffer_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes) {
            auto msg = beast::buffers_to_string(self->buffer_.data());
            self->buffer_.consume(self->buffer_.size());
            self->server_.handleMessageFromSession(msg);
            self->doRead();  // Tiếp tục đọc
        });
}
```

### 13c. Async Write
```cpp
void WebSocketSession::doWrite() {
    ws_.async_write(asio::buffer(message_queue_.front()),
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            std::lock_guard<std::mutex> lock(self->queue_mutex_);
            self->message_queue_.pop();
            if (!self->message_queue_.empty()) {
                asio::post(self->ws_.get_executor(), 
                           std::bind(&WebSocketSession::doWrite, self));
            } else {
                self->writing_ = false;
            }
        });
}
```

### 13d. io_context event loop
```cpp
void WebSocketServer::run(){
    doAccept();
    io_.run();  // Block here, processing async events
}
```

---

## 14. ALSA (Advanced Linux Sound Architecture)

**Khái niệm:** API cấp thấp để capture/playback audio trên Linux. Project dùng ALSA để record âm thanh từ microphone.

**Nơi sử dụng:** `recordmgr/src/Util/AlsaHelper.cpp`

### 14a. Mở PCM device
```cpp
snd_pcm_open(&pcmHandle_, device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
```

### 14b. Cấu hình hardware parameters
```cpp
snd_pcm_hw_params_set_access(pcmHandle_, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
snd_pcm_hw_params_set_format(pcmHandle_, hwParams, SND_PCM_FORMAT_S16_LE);
snd_pcm_hw_params_set_rate_near(pcmHandle_, hwParams, &rate, nullptr);
snd_pcm_hw_params_set_channels(pcmHandle_, hwParams, 1);  // Mono
snd_pcm_hw_params(pcmHandle_, hwParams);
```

### 14c. Capture audio frames
```cpp
bool AlsaHelper::captureOnce() {
    std::vector<int16_t> buffer(frames);
    snd_pcm_sframes_t r = snd_pcm_readi(pcmHandle_, buffer.data(), frames);
    if (r == -EPIPE) {
        snd_pcm_prepare(pcmHandle_);  // Recover from overrun
        return true;
    }
    audioBuffer_.insert(audioBuffer_.end(), buffer.begin(), buffer.begin() + r);
    return true;
}
```

### 14d. Auto-detect capture device
```cpp
std::string AlsaHelper::findCaptureDevice() {
    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        // Enumerate PCM devices on each card
        snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_CAPTURE);
        if (snd_ctl_pcm_info(ctl_handle, pcm_info) >= 0) {
            return "plughw:" + std::to_string(card) + "," + std::to_string(dev);
        }
    }
}
```

### 14e. Cleanup
```cpp
void AlsaHelper::cleanupAlsa() {
    if (pcmHandle_) {
        snd_pcm_drain(pcmHandle_);   // Đợi buffer còn lại được xử lý
        snd_pcm_close(pcmHandle_);
    }
}
```

---

## 15. GPIO / sysfs - Temperature Sensor (1-Wire)

**Khái niệm:** Đọc cảm biến nhiệt độ DS18B20 thông qua **sysfs** interface (`/sys/bus/w1/devices/`). Đây là cách Linux kernel expose hardware interface cho userspace.

**Nơi sử dụng:** `hardwaremgr/src/Model/GPIO.cpp`

```cpp
// Tìm file sensor bằng cách scan thư mục
void GPIO::findSensorFile() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("/sys/bus/w1/devices/")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (entryName.rfind("28-", 0) == 0) {  // DS18B20 prefix
                sensorFile_ = devicesPath + entryName + "/w1_slave";
            }
        }
        closedir(dir);
    }
}

// Đọc nhiệt độ từ sysfs
bool GPIO::readTemperatureSensor(float& temperature) {
    std::ifstream sensorStream(sensorFile_);
    // Line 1: "xx xx xx xx : crc=xx YES"
    std::getline(sensorStream, line);
    if (line.find("YES") == std::string::npos) return false;  // CRC check
    // Line 2: "xx xx xx xx t=25625"
    std::getline(sensorStream, line);
    int temp_mC = std::stoi(line.substr(tempPos + 2));
    temperature = static_cast<float>(temp_mC) / 1000.0f;  // Convert millidegree to degree
}
```

---

## 16. SQLite Database (File-based)

**Khái niệm:** Embedded database cho lưu trữ local, không cần server riêng. Thread-safe thông qua mutex.

**Nơi sử dụng:** `coremgr/src/DB/SQLiteDatabase.cpp`

```cpp
// Mở database
sqlite3_open(dbFilePath_.c_str(), &db_);

// Tạo schema
executeSQL(R"(
    CREATE TABLE IF NOT EXISTS audio_records (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        file_path TEXT NOT NULL UNIQUE,
        duration_sec INTEGER NOT NULL,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)");

// Prepared Statement (chống SQL injection)
sqlite3_stmt* stmt = prepareStatement("INSERT INTO audio_records (file_path, duration_sec) VALUES (?, ?);");
sqlite3_bind_text(stmt, 1, record.filePath.c_str(), -1, SQLITE_STATIC);
sqlite3_bind_int(stmt, 2, record.durationSec);
sqlite3_step(stmt);
sqlite3_finalize(stmt);
```

---

## 17. Thread Pool Pattern

**Khái niệm:** Pool gồm N worker threads lấy task từ shared queue để xử lý. Tránh overhead tạo/hủy thread liên tục.

**Nơi sử dụng:** `coremgr/src/Thread/DBThreadPool.cpp`

```cpp
// Tạo N worker threads
DBThreadPool::DBThreadPool(..., int numWorkers) {
    for (int i = 0; i < numWorkers; i++) {
        workers_.emplace_back(&DBThreadPool::threadFunction, this);
    }
}

// Worker thread function
void DBThreadPool::threadFunction() {
    while (runningFlag_) {
        std::unique_lock<std::mutex> lock(taskMutex_);
        cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });
        
        auto task = std::move(tasksQueue_.front());
        tasksQueue_.pop();
        lock.unlock();
        task();  // Execute task outside of lock
    }
}

// Enqueue task
void DBThreadPool::enqueueTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        tasksQueue_.push(std::move(task));
    }
    cv_.notify_one();  // Đánh thức 1 worker
}
```

---

## 18. std::future / std::packaged_task

**Khái niệm:** `std::packaged_task` wrap một callable thành async task. `std::future` cho phép lấy kết quả từ task chạy trên thread khác.

**Nơi sử dụng:** `coremgr/src/Thread/DBThreadPool.cpp`

```cpp
std::future<AudioRecord> DBThreadPool::insertAudioRecord(const std::string& filePath, int durationSec) {
    auto task = std::make_shared<std::packaged_task<AudioRecord()>>(
        [this, filePath, durationSec]() {
            std::lock_guard<std::mutex> dbLock(dbMutex_);
            return database_->insertAudioRecord(record);
        });

    auto future = task->get_future();
    enqueueTask([task](){ (*task)(); });  // Submit to thread pool
    return future;  // Caller có thể future.get() để đợi kết quả
}
```

---

## 19. Producer-Consumer Pattern (Event Queue)

**Khái niệm:** Thread-safe event queue dùng mutex + condition_variable. Producer push event, Consumer pop và xử lý.

**Nơi sử dụng:** `common/src/Event/EventQueue.cpp`

```cpp
// Producer (DBusReceiver, WebSocket, ...)
bool EventQueue::pushEvent(const std::shared_ptr<Event> event) {
    std::lock_guard<std::mutex> lk(queueMutex_);
    eventList_.push_back(event);
    notifyEvent();  // Đánh thức consumer
    return true;
}

// Consumer (MainWorker)
void MainWorker::threadFunction() {
    while (runningFlag_) {
        if (eventQueue_->hasEvent()) {
            auto event = eventQueue_->popEvent();
            processEvent(event);
        } else {
            eventQueue_->waitForEvent(500);  // Block chờ event mới
        }
    }
}
```

---

## 20. File I/O (std::ifstream, std::ofstream)

**Khái niệm:** Đọc/ghi file trong Linux filesystem.

### 20a. Đọc sensor file (GPIO.cpp)
```cpp
std::ifstream sensorStream(sensorFile_);
std::getline(sensorStream, line);
```

### 20b. Ghi WAV file (AlsaHelper.cpp)
```cpp
std::ofstream out(outputFilePath_, std::ios::binary);
out.write("RIFF", 4);
out.write(reinterpret_cast<const char*>(&chunkSize), 4);
out.write("WAVE", 4);
// ... write fmt chunk, data chunk
out.write(reinterpret_cast<const char*>(audioBuffer_.data()), subchunk2Size);
out.close();
```

---

## 21. std::filesystem (C++17)

**Khái niệm:** API chuẩn C++17 cho các thao tác filesystem: tạo thư mục, kiểm tra file tồn tại, copy, xóa, ...

**Nơi sử dụng:** `recordmgr/src/Util/AudioFilter.cpp`, `coremgr/src/DB/SQLiteDatabase.cpp`

```cpp
namespace fs = std::filesystem;

// Tạo thư mục đệ quy
fs::create_directories(outputDir);

// Copy file giữa các filesystem khác nhau (thay vì rename)
fs::copy_file(tempFilteredPath, filteredFilePath_, fs::copy_options::overwrite_existing);

// Xóa file
fs::remove(wavFilePath);

// Kiểm tra tồn tại
if (fs::exists(wavFilePath)) { ... }
```

---

## 22. Directory Operations (opendir/readdir/closedir)

**Khái niệm:** POSIX API đọc nội dung thư mục. Dùng cho scan sensor device.

**Nơi sử dụng:** `hardwaremgr/src/Model/GPIO.cpp`

```cpp
#include <dirent.h>

DIR *dir;
struct dirent *ent;
if ((dir = opendir(devicesPath.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
        std::string entryName = ent->d_name;
        if (entryName.rfind(sensorPrefix, 0) == 0) {
            sensorFile_ = devicesPath + entryName + "/w1_slave";
        }
    }
    closedir(dir);
}
```

---

## 23. Process ID (getpid)

**Khái niệm:** Lấy PID của process hiện tại. Dùng trong Logger để phân biệt log giữa các process.

**Nơi sử dụng:** `common/src/Log/Logger.cpp`

```cpp
#include <unistd.h>

printf("[%s][%d][%s][%s:%d][%s] [%s]%s\n",
    m_moduleName.c_str(),
    getpid(),               // PID
    timeStream.str().c_str(),
    short_file, line, func,
    levelToString(level),
    message.c_str());
```

---

## 24. system() - Spawning Child Process

**Khái niệm:** `system()` tạo child process chạy shell command. Dùng để gọi `sox` (audio processing tool).

**Nơi sử dụng:** `recordmgr/src/Util/AudioFilter.cpp`

```cpp
#include <cstdlib>

// Bước 1: Tạo noise profile
std::string cmd = "sox \"" + wavFilePath + "\" -n trim 0 0.4 noiseprof \"" + noiseProfilePath + "\"";
int result = std::system(cmd.c_str());

// Bước 2: Áp dụng filter
cmd = "sox \"" + wavFilePath + "\" \"" + outputPath + "\" highpass 100 lowpass 3000 noisered \"" 
      + noiseProfilePath + "\" 0.21";
result = std::system(cmd.c_str());
```

**Lưu ý phỏng vấn:** `system()` có security risk (shell injection). Trong production nên dùng `fork()` + `exec()` hoặc `posix_spawn()`.

---

## 25. Cross-Compilation (aarch64 Toolchain)

**Khái niệm:** Build code trên x86 host nhưng chạy trên ARM64 (Raspberry Pi 4).

**Nơi sử dụng:** `rpi4_toolchain.cmake`

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSROOT $ENV{HOME}/raspberrypi/sysroot)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
add_compile_definitions(RASPBERRY_PI)
```

---

## 26. Singleton Pattern (std::call_once, std::once_flag)

**Khái niệm:** Đảm bảo chỉ tạo duy nhất 1 instance, thread-safe.

### 26a. std::call_once (Timer)
```cpp
// Timer.hpp
std::unique_ptr<Timer>& Timer::getInstance() {
    static std::unique_ptr<Timer> instance;
    std::call_once(onceFlag, []() {
        instance.reset(new Timer());
        instance->run();
    });
    return instance;
}
```

### 26b. Meyer's Singleton (Config, StateView)
```cpp
// Config.hpp
static Config* getInstance() {
    static Config instance;  // Thread-safe từ C++11 (Magic Statics)
    return &instance;
}
```

---

## 27. Variadic Functions (va_list, va_start, va_end)

**Khái niệm:** Hàm nhận số lượng argument không xác định. Dùng cho logger giống `printf`.

**Nơi sử dụng:** `common/src/Log/Logger.cpp`

```cpp
void Logger::printLog(..., const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, fmt, args_copy);  // Tính size cần thiết
    va_end(args_copy);

    std::vector<char> buffer(len + 1);
    vsnprintf(buffer.data(), buffer.size(), fmt, args);
    va_end(args);
}
```

---

## 28. std::thread::detach() - Detached Thread

**Khái niệm:** Thread chạy độc lập, không cần join. Khi thread kết thúc, tài nguyên tự động được giải phóng.

**Nơi sử dụng:** `recordmgr/src/Thread/MainWorker.cpp`

```cpp
void MainWorker::processFilterWavFileEvent(std::shared_ptr<Payload> payload) {
    std::string wavFilePath = wavPayload->getFilePath();
    int durationSec = wavPayload->getDurationSec();

    std::thread([wavFilePath, durationSec]() {
        AudioFilter filter;
        filter.applyFilter(wavFilePath);
        // Gửi kết quả qua D-Bus
        DBUS_SENDER()->sendMessageNoti(DBusCommand::FILTER_WAV_FILE_NOTI, ...);
        // Xóa file tạm
        std::filesystem::remove(wavFilePath);
    }).detach();  // Fire-and-forget
}
```

**Lưu ý phỏng vấn:** `detach()` phù hợp cho fire-and-forget task, nhưng khó kiểm soát lifecycle. Dùng `join()` hoặc thread pool nếu cần đảm bảo completion.

---

## 29. Bluetooth Stack (BlueZ D-Bus API)

**Khái niệm:** Giao tiếp với BlueZ daemon qua D-Bus để quản lý Bluetooth.

**Nơi sử dụng:** `hardwaremgr/src/BT/BluezDBus.cpp`

```cpp
// GetManagedObjects - lấy tất cả BT devices
dbus_message_new_method_call("org.bluez", "/",
    "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");

// Set Property qua org.freedesktop.DBus.Properties
dbus_message_new_method_call("org.bluez", adapterPath_.c_str(),
    "org.freedesktop.DBus.Properties", "Set");

// Pair/Connect/Disconnect device
dbus_message_new_method_call("org.bluez", objectPath.c_str(),
    "org.bluez.Device1", "Pair");
```

**D-Bus Signals được lắng nghe:**
- `InterfacesAdded` - Device mới được phát hiện
- `InterfacesRemoved` - Device bị xóa
- `PropertiesChanged` - Properties thay đổi (Paired, Connected, RSSI...)

---

## 30. oFono Telephony Stack

**Khái niệm:** oFono là telephony daemon trên Linux. Trong project này, oFono chỉ dùng cho **HFP (Hands-Free Profile)** — quản lý modem và voice call qua D-Bus. **Không** dùng oFono cho phonebook/call history (sử dụng OBEX PBAP thay thế — xem mục 35).

**Nơi sử dụng:** `hardwaremgr/src/BT/OfonoDBus.cpp`, `BluetoothWorker.cpp`

**oFono Interfaces sử dụng:**
- `org.ofono.Manager` — Phát hiện modem mới (ModemAdded/ModemRemoved)
- `org.ofono.Modem` — Set Powered, Online
- `org.ofono.VoiceCallManager` — Dial, HangupAll, CallAdded/CallRemoved
- `org.ofono.VoiceCall` — Answer, PropertyChanged (State)
- `org.ofono.Handsfree` — HFP audio routing

```cpp
// Dial a call via VoiceCallManager
dbus_message_new_method_call("org.ofono", modemPath_.c_str(),
    "org.ofono.VoiceCallManager", "Dial");
dbus_message_append_args(msg, DBUS_TYPE_STRING, &num_cstr, 
                          DBUS_TYPE_STRING, &hide_callerid, DBUS_TYPE_INVALID);

// Set modem property (Powered, Online)
dbus_message_new_method_call("org.ofono", modemPath.c_str(),
    "org.ofono.Modem", "SetProperty");

// Answer incoming call
dbus_message_new_method_call("org.ofono", incomingCallPath.c_str(),
    "org.ofono.VoiceCall", "Answer");

// Hang up all calls
dbus_message_new_method_call("org.ofono", modemPath_.c_str(),
    "org.ofono.VoiceCallManager", "HangupAll");
```

**⚠️ Lưu ý quan trọng:**
- oFono **KHÔNG** có interface `org.ofono.Phonebook` hay `org.ofono.CallHistory` — đây là lỗi phổ biến khi tham khảo tài liệu cũ.
- Phonebook/Call History phải dùng **OBEX PBAP** (BlueZ obexd) trên **session bus** — xem mục 35.

---

## 31. BlueZ Agent (Bluetooth Pairing Agent)

**Khái niệm:** Agent xử lý pairing requests từ BlueZ. Khi device muốn pair, BlueZ gọi method trên agent qua D-Bus.

**Nơi sử dụng:** `hardwaremgr/src/BT/BluetoothAgent.cpp`

```cpp
// Register agent với BlueZ
dbus_message_new_method_call("org.bluez", "/org/bluez",
    "org.bluez.AgentManager1", "RegisterAgent");

// Xử lý RequestConfirmation (user confirm passkey)
void BluetoothAgent::handleRequestConfirmation(DBusMessage* message) {
    dbus_message_get_args(message, NULL, 
        DBUS_TYPE_OBJECT_PATH, &device_path,
        DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_INVALID);
    
    pendingConfirmations_[deviceAddress] = dbus_message_ref(message); // Lưu pending request
    // Gửi noti cho UI hiển thị passkey
}

// User confirm hoặc reject
void BluetoothAgent::confirmRequest(const std::string& deviceAddress, bool confirmed) {
    if (confirmed) {
        sendSimpleReply(request, DBUS_MESSAGE_TYPE_METHOD_RETURN); // Accept
    } else {
        sendSimpleReply(request, DBUS_MESSAGE_TYPE_ERROR);         // Reject
    }
}
```

---

## 32. D-Bus Message Iterator

**Khái niệm:** Parse D-Bus messages phức tạp (nested dict, array, variant) bằng iterator API.

**Nơi sử dụng:** `BluezDBus.cpp`, `OfonoDBus.cpp`, `DBusReceiverBase.cpp`

```cpp
// Parse a{sv} (dict of string → variant)
DBusMessageIter iter, dict_entry_iter, variant_iter;
dbus_message_iter_init(msg, &iter);
dbus_message_iter_recurse(&iter, &dict_entry_iter);

while (dbus_message_iter_get_arg_type(&dict_entry_iter) == DBUS_TYPE_DICT_ENTRY) {
    DBusMessageIter prop_iter;
    dbus_message_iter_recurse(&dict_entry_iter, &prop_iter);
    
    const char* key = nullptr;
    dbus_message_iter_get_basic(&prop_iter, &key);       // Get key
    dbus_message_iter_next(&prop_iter);
    dbus_message_iter_recurse(&prop_iter, &variant_iter); // Get variant value
    
    if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
        dbus_bool_t value;
        dbus_message_iter_get_basic(&variant_iter, &value);
    }
    dbus_message_iter_next(&dict_entry_iter);
}
```

**Serialize (gửi) array of string:**
```cpp
// ISenderFactory.cpp
DBusMessageIter iter, array_iter;
dbus_message_iter_init_append(msg, &iter);
dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array_iter);
for (int i = 0; i < DBUS_DATA_MAX; ++i) {
    const char* str = msgInfo.data[i].c_str();
    dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_STRING, &str);
}
dbus_message_iter_close_container(&iter, &array_iter);
```

---

## 33. Timer Mechanism (Software Timer)

**Khái niệm:** Software timer tự implement. Dùng `std::chrono::steady_clock` để track expire time. Timer thread kiểm tra expire và push event vào EventQueue.

**Nơi sử dụng:** `coremgr/src/Thread/Timer.cpp`

```cpp
int32_t Timer::startTimer(const uint32_t timeout_ms, std::shared_ptr<Event> event) {
    auto timerElement = makeNewTimerElement(timeout_ms, event);
    timerElement->expireTime = std::chrono::steady_clock::now() 
                             + std::chrono::milliseconds(timeout_ms);
    
    std::lock_guard<std::mutex> lock(timerMutex);
    timerTableMap.insert({timerElement->timerId, timerElement});
    timerWakeupCondition.notify_all();
    return timerElement->timerId;
}

// Trong thread function, check expired:
for (auto iter = timerTableMap.begin(); iter != timerTableMap.end(); ) {
    if (timerElement->expireTime <= currentTime) {
        eventQueue_->pushEvent(timerElement->event);  // Fire!
        iter = timerTableMap.erase(iter);
    } else {
        ++iter;
    }
}
```

---

## 34. fflush(stdout) - Immediate Output

**Khái niệm:** Đảm bảo output buffer được ghi ra ngay lập tức. Quan trọng khi redirect stdout (như journalctl).

**Nơi sử dụng:** `common/src/Log/Logger.cpp`

```cpp
printf("[%s][%d]...", ...);
fflush(stdout);  // Force flush ngay lập tức
```

**Tại sao cần:** `stdout` mặc định là line-buffered khi ghi ra terminal, nhưng full-buffered khi redirect (pipe/file). `fflush()` đảm bảo log xuất hiện ngay trong `journalctl -f`.

---

## 35. OBEX PBAP (Phone Book Access Profile via obexd)

**Khái niệm:** OBEX PBAP là Bluetooth profile chuẩn để truy cập danh bạ và lịch sử cuộc gọi từ điện thoại. Sử dụng `obexd` daemon (BlueZ OBEX) chạy trên **session D-Bus** (không phải system bus).

**Nơi sử dụng:** `hardwaremgr/src/BT/ObexPbapClient.cpp`, `BluetoothWorker.cpp`

**Tại sao dùng OBEX PBAP thay vì oFono:**
- oFono **không có** interface `org.ofono.Phonebook` hay `org.ofono.CallHistory` — đây là API không tồn tại.
- oFono chỉ quản lý HFP (voice call): Dial, Answer, Hangup.
- Danh bạ và lịch sử cuộc gọi phải lấy qua PBAP — đây là chuẩn Bluetooth SIG chính thức.

**Kiến trúc:**
```
┌─────────────┐         Session D-Bus         ┌─────────────┐
│ ObexPbapClient│ ───── org.bluez.obex ──────→│   obexd      │
│ (HardwareMgr)│                              │ (BlueZ OBEX) │
└──────┬──────┘                              └──────┬──────┘
       │                                           │ OBEX/PBAP
       │          ┌────────────────────┐           │
       └──────────│ Phone (Android/iOS)│←──────────┘
                  └────────────────────┘
```

**PBAP Standard Paths (trên điện thoại):**
| Path | Nội dung |
|------|----------|
| `int/pb` | Phone contacts (danh bạ) |
| `int/ich` | Incoming call history (cuộc gọi đến) |
| `int/och` | Outgoing call history (cuộc gọi đi) |
| `int/mch` | Missed call history (cuộc gọi nhỡ) |
| `int/cch` | Combined call history (tổng hợp) |
| `SIM1/telecom/pb.vcf` | SIM contacts |

**Code — Tạo PBAP session:**
```cpp
// Kết nối obexd trên session bus (không phải system bus!)
sessionConn_ = dbus_bus_get(DBUS_BUS_SESSION, &err);

// Tạo PBAP session tới điện thoại
// org.bluez.obex.Client1.CreateSession(destination, {Target: "pbap"})
DBusMessage* msg = dbus_message_new_method_call(
    "org.bluez.obex", "/org/bluez/obex",
    "org.bluez.obex.Client1", "CreateSession");

// Truyền device address + dict a{sv} với Target = "pbap"
dbus_message_iter_append_basic(&args_iter, DBUS_TYPE_STRING, &dest);
// ... build dict {"Target": "pbap"} ...

DBusMessage* reply = dbus_connection_send_with_reply_and_block(sessionConn_, msg, 30000, &err);
// Reply chứa session object path, e.g., "/org/bluez/obex/client/session0"
```

**Code — Pull danh bạ:**
```cpp
// 1. Select phonebook: org.bluez.obex.PhonebookAccess1.Select("int", "pb")
DBusMessage* msg = dbus_message_new_method_call(
    "org.bluez.obex", sessionPath_.c_str(),
    "org.bluez.obex.PhonebookAccess1", "Select");

// 2. Pull all contacts: org.bluez.obex.PhonebookAccess1.PullAll("", {Format: "vcard30"})
// Returns: (object_path transfer, a{sv} properties)
// obexd downloads VCF file → parse vCard format

// 3. Poll transfer status: org.bluez.obex.Transfer1.Status
// Status = "queued" → "active" → "complete" (or "error")
while (status != "complete" && status != "error") {
    dbus_message_new_method_call("org.bluez.obex", transferPath.c_str(),
        "org.freedesktop.DBus.Properties", "GetAll");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

// 4. Read VCF file → parse contacts (FN, TEL, N fields)
// 5. Feed contacts to OfonoDBus::setPhonebook() cho caller ID lookup
```

**Code — Parse vCard (VCF):**
```cpp
// vCard 3.0 format:
// BEGIN:VCARD
// VERSION:3.0
// FN:Nguyen Van A          ← Full Name
// N:A;Van;Nguyen;;         ← Structured Name (Last;First;Middle;Prefix;Suffix)
// TEL;TYPE=CELL:0912345678 ← Phone number
// X-IRMC-CALL-DATETIME;TYPE=RECEIVED:20250101T120000 ← Call history datetime
// END:VCARD

if (line.substr(0, 3) == "FN:") {
    current.name = line.substr(colonPos + 1);
} else if (line.substr(0, 4) == "TEL:") {
    current.number = line.substr(colonPos + 1);
} else if (line.find("X-IRMC-CALL-DATETIME") != std::string::npos) {
    current.datetime = line.substr(colonPos + 1);
}
```

**Flow tổng thể (trigger khi device connect):**
```
BluetoothWorker::handlePropertiesChanged()
  → Device Connected=true && Paired=true
  → triggerPbapSync(address) [detached thread]
      → sleep(2s)  // chờ kết nối ổn định
      → ObexPbapClient::createSession(address)
      → ObexPbapClient::pullPhonebook()
          → Select("int", "pb") → PullAll → parse VCF → send NOTI per contact
      → Feed contacts → OfonoDBus::setPhonebook() [cho caller ID]
      → ObexPbapClient::pullCallHistory()
          → Select("int", "ich") → PullAll [incoming]
          → Select("int", "och") → PullAll [outgoing]
          → Select("int", "mch") → PullAll [missed]
```

**Yêu cầu trên Raspberry Pi:**
```bash
sudo apt install bluez-obex        # Cài obexd
# obexd tự khởi động trên session bus khi có D-Bus session
# Headless Pi cần đảm bảo có D-Bus session (dbus-run-session hoặc systemd --user)
```

**Giải thích phỏng vấn:**
- PBAP dùng **session bus** (khác với BlueZ/oFono dùng system bus) → cần `dbus_bus_get(DBUS_BUS_SESSION)`.
- Transfer là async: PullAll trả về transfer object path → poll Status cho đến khi "complete".
- vCard (VCF) là format chuẩn cho contacts — parse text-based, mỗi entry bắt đầu `BEGIN:VCARD`.
- `X-IRMC-CALL-DATETIME` là field riêng của PBAP cho thời gian cuộc gọi.
- Trigger PBAP trong detached thread để không block D-Bus message loop.

---

## 📊 Tổng kết Architecture

```
┌─────────────────────┐    ┌────────────────────────┐    ┌─────────────────────┐
│   HardwareManager   │    │     CoreManager         │    │   RecordManager     │
│                     │    │                          │    │                     │
│ ┌─MainWorker       │    │ ┌─MainWorker            │    │ ┌─MainWorker       │
│ ├─DBusReceiver     │    │ ├─DBusReceiver          │    │ ├─DBusReceiver     │
│ ├─MonitorWorker    │    │ ├─WebSocket             │    │ └─RecordWorker     │
│ └─BluetoothWorker  │    │ ├─DBThreadPool(5)       │    │   (ALSA capture)   │
│   (BlueZ + oFono)  │    │ └─Timer                 │    │                     │
│   (OBEX PBAP)      │    │   (SQLite + Boost.Asio) │    │                     │
│   (poll() + D-Bus) │    │                          │    │                     │
└────────┬────────────┘    └─────────┬────────────────┘    └────────┬────────────┘
         │                           │                               │
         ├───── D-Bus System Bus ────┴───────────────────────────────┘
         │           (IPC: Signal + Method Call)
         └───── D-Bus Session Bus (obexd PBAP)
```

**Key patterns used:**
- **Producer-Consumer** (EventQueue)
- **Thread Pool** (DBThreadPool)
- **Observer/Pub-Sub** (D-Bus signals + WebSocket broadcast)
- **Factory Method** (ISenderFactory → CMSenderFactory, HMSenderFactory, RMSenderFactory)
- **Template Method** (ThreadBase → MainWorker, DBusReceiver, etc.)
- **Singleton** (Config, StateView, Timer)
- **Dependency Injection** (WebSocket, DBThreadPool injected into MainWorker)
- **State Machine** (RecordWorker: IDLE → RECORDING → IDLE)

---

*Generated from RaspberryPiService source code analysis - March 2026*
