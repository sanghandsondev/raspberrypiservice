# C++ Features Review — RaspberryPiService Project

> **Mục đích:** Tổng hợp toàn bộ tính năng C++ nổi bật đã sử dụng trong project để ôn phỏng vấn.
> Project gồm 3 service: **CoreManager**, **HardwareManager**, **RecordManager** + thư viện **common**.

---

## Mục Lục

1. [Design Patterns](#1-design-patterns)
   - 1.1 [Singleton Pattern (Meyer's Singleton)](#11-singleton-pattern-meyers-singleton)
   - 1.2 [Singleton Pattern (std::call_once)](#12-singleton-pattern-stdcall_once)
   - 1.3 [Factory Method Pattern](#13-factory-method-pattern)
   - 1.4 [Template Method Pattern](#14-template-method-pattern)
   - 1.5 [Observer / Pub-Sub Pattern](#15-observer--pub-sub-pattern)
   - 1.6 [Producer-Consumer Pattern](#16-producer-consumer-pattern)
   - 1.7 [Dependency Injection Pattern](#17-dependency-injection-pattern)
   - 1.8 [State Machine Pattern](#18-state-machine-pattern)
   - 1.9 [Thread Pool Pattern](#19-thread-pool-pattern)
2. [C++17 Features](#2-c17-features)
   - 2.1 [std::optional](#21-stdoptional)
   - 2.2 [std::filesystem](#22-stdfilesystem)
   - 2.3 [inline static member variables](#23-inline-static-member-variables)
   - 2.4 [Structured Bindings](#24-structured-bindings)
   - 2.5 [[[maybe_unused]] Attribute](#25-maybe_unused-attribute)
   - 2.6 [std::call_once / std::once_flag](#26-stdcall_once--stdonce_flag)
   - 2.7 [std::any (include)](#27-stdany-include)
3. [OOP — Lập Trình Hướng Đối Tượng](#3-oop--lập-trình-hướng-đối-tượng)
   - 3.1 [Abstract Class / Pure Virtual Function](#31-abstract-class--pure-virtual-function)
   - 3.2 [Virtual Destructor](#32-virtual-destructor)
   - 3.3 [override Specifier](#33-override-specifier)
   - 3.4 [= default / = delete](#34--default---delete)
   - 3.5 [explicit Constructor](#35-explicit-constructor)
   - 3.6 [Polymorphic Inheritance Hierarchy](#36-polymorphic-inheritance-hierarchy)
   - 3.7 [std::dynamic_pointer_cast (Downcasting)](#37-stddynamic_pointer_cast-downcasting)
   - 3.8 [Forward Declarations](#38-forward-declarations)
   - 3.9 [Operator Overloading](#39-operator-overloading)
4. [Smart Pointers & Memory Management](#4-smart-pointers--memory-management)
   - 4.1 [std::shared_ptr / std::make_shared](#41-stdshared_ptr--stdmake_shared)
   - 4.2 [std::unique_ptr / std::make_unique](#42-stdunique_ptr--stdmake_unique)
   - 4.3 [std::enable_shared_from_this](#43-stdenable_shared_from_this)
   - 4.4 [RAII (Resource Acquisition Is Initialization)](#44-raii-resource-acquisition-is-initialization)
5. [Modern C++ Features](#5-modern-c-features)
   - 5.1 [Lambda Expressions](#51-lambda-expressions)
   - 5.2 [Move Semantics (std::move)](#52-move-semantics-stdmove)
   - 5.3 [std::function (Callback)](#53-stdfunction-callback)
   - 5.4 [enum class (Scoped Enumerations)](#54-enum-class-scoped-enumerations)
   - 5.5 [Type Alias (using)](#55-type-alias-using)
   - 5.6 [Raw String Literals R"(...)"](#56-raw-string-literals-r)
   - 5.7 [std::future / std::packaged_task / std::promise](#57-stdfuture--stdpackaged_task--stdpromise)
   - 5.8 [std::chrono (Time Library)](#58-stdchrono-time-library)
   - 5.9 [Range-based for loop / auto](#59-range-based-for-loop--auto)
   - 5.10 [Initializer List { }](#510-initializer-list--)
6. [Preprocessor & Macro Techniques](#6-preprocessor--macro-techniques)
   - 6.1 [Variadic Macro with __VA_ARGS__](#61-variadic-macro-with-__va_args__)
   - 6.2 [Macro Constants as Instance Accessors](#62-macro-constants-as-instance-accessors)
   - 6.3 [Include Guard (#ifndef)](#63-include-guard-ifndef)
7. [STL Containers & Algorithms](#7-stl-containers--algorithms)
8. [Variadic Functions (C-style)](#8-variadic-functions-c-style)
9. [Đánh Giá Tổng Quan: Có Chuyên Nghiệp Không?](#9-đánh-giá-tổng-quan-có-chuyên-nghiệp-không)

---

## 1. Design Patterns

### 1.1 Singleton Pattern (Meyer's Singleton)

**Khái niệm:** Chỉ cho phép tạo duy nhất 1 instance của class trong suốt vòng đời chương trình. Sử dụng **static local variable** (thread-safe từ C++11) + **delete** copy constructor và assignment operator.

**Áp dụng trong project:** `Config`, `StateView`, `DBusSender`, `RLogger`, `JsonHelper`

**📁 coremgr/include/Configure/Config.hpp**
```cpp
class Config : public IConfig {
public:
    static Config *getInstance() {
        static Config instance;       // Meyer's Singleton - thread-safe (C++11)
        return &instance;
    }
    Config(const Config &) = delete;            // Chặn copy
    Config &operator=(const Config &) = delete; // Chặn assignment

private:
    Config() = default;     // Constructor private → không ai tạo được từ bên ngoài
    ~Config() = default;    // Destructor private
    
    inline static const std::string COREMGR_SERVICE_NAME = "com.example.coremanager";
    // ... more config values
};
```

**📁 coremgr/include/DBus/DBusSender.hpp**
```cpp
class DBusSender : public DBusSenderBase {
public:
    static DBusSender *getInstance() {
        static DBusSender instance;
        return &instance;
    }
    DBusSender(const DBusSender &) = delete;
    DBusSender &operator=(const DBusSender &) = delete;

private:
    DBusSender() : DBusSenderBase() {
        msgMaker = std::make_shared<CMSenderFactory>(); // Gán Factory khi khởi tạo
    };
    ~DBusSender() override = default;
};
```

**📁 coremgr/include/Log/RLogger.hpp**
```cpp
class RLogger : public Logger {
public:
    static RLogger *getInstance(){
        static RLogger instance;
        return &instance;
    }
    RLogger(const RLogger &) = delete;
    RLogger &operator=(const RLogger &) = delete;

private:
    RLogger() : Logger("CoreManager") {}    // Gọi constructor base class
    ~RLogger() override = default;
};
```

**Giải thích phỏng vấn:**
- `static Config instance;` — biến static local chỉ được khởi tạo 1 lần duy nhất, thread-safe theo chuẩn C++11.
- `= delete` — ngăn không cho copy hoặc gán object, đảm bảo chỉ có 1 instance.
- Constructor `private` → không ai có thể `new Config()` từ bên ngoài.

---

### 1.2 Singleton Pattern (std::call_once)

**Khái niệm:** Biến thể Singleton sử dụng `std::call_once` + `std::once_flag` để đảm bảo khởi tạo chỉ xảy ra 1 lần, kể cả trong multi-thread. Khác Meyer's Singleton ở chỗ dùng `std::unique_ptr` để quản lý lifetime.

**📁 coremgr/include/Thread/Timer.hpp**
```cpp
class Timer : public ThreadBase {
public:
    static std::unique_ptr<Timer> &getInstance();
    Timer(const Timer &) = delete;
    Timer(Timer &&) noexcept = delete;         // Delete cả move constructor!
    Timer &operator=(const Timer &) = delete;
    Timer &operator=(Timer &&) noexcept = delete; // Delete cả move assignment!

private:
    Timer();
    inline static std::once_flag onceFlag = {};
    // ...
};
```

**📁 coremgr/src/Thread/Timer.cpp**
```cpp
std::unique_ptr<Timer> &Timer::getInstance() {
    static std::unique_ptr<Timer> instance;
    std::call_once(onceFlag, []() {
        instance.reset(new Timer());   // Khởi tạo chỉ 1 lần
        instance->run();               // Tự động chạy thread khi tạo
    });
    return instance;
}
```

**Giải thích phỏng vấn:**
- `std::call_once` đảm bảo lambda chỉ được gọi đúng 1 lần dù nhiều thread gọi `getInstance()` đồng thời.
- Dùng `std::unique_ptr` thay vì raw pointer → tự động giải phóng memory.
- Delete cả **move constructor** và **move assignment** → Singleton không thể bị move đi nơi khác.

---

### 1.3 Factory Method Pattern

**Khái niệm:** Tách logic tạo object ra khỏi code sử dụng. Base class định nghĩa interface (abstract), mỗi service tạo concrete factory riêng.

**📁 common/include/DBus/ISenderFactory.hpp** (Abstract Factory)
```cpp
class ISenderFactory {
public:
    virtual ~ISenderFactory() = default;
    virtual DBusMessage* makeMsg(DBusCommand cmd) = 0;          // Pure virtual
    virtual DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, 
                                     const DBusDataInfo &msgInfo) = 0; // Pure virtual
protected:
    virtual DBusMessage* makeMsgInternal(const char *objectpath, const char *interface,
                                         const char* signal, DBusCommand cmd);
    virtual DBusMessage* makeMsgNotiInternal(const char *objectpath, const char *interface,
                                             const char* signal, DBusCommand cmd,
                                             bool isSuccess, const DBusDataInfo &msgInfo);
};
```

**📁 coremgr/include/DBus/CMSenderFactory.hpp** (Concrete Factory)
```cpp
class CMSenderFactory : public ISenderFactory {
public:
    CMSenderFactory() = default;
    ~CMSenderFactory() override = default;

    DBusMessage* makeMsg(DBusCommand cmd) override;         // override pure virtual
    DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, 
                             const DBusDataInfo &msgInfo) override;
    // ...
};
```

**📁 coremgr/include/DBus/DBusSender.hpp** (Sử dụng Factory)
```cpp
DBusSender() : DBusSenderBase() {
    msgMaker = std::make_shared<CMSenderFactory>();  // Inject concrete factory
};
```

**Giải thích phỏng vấn:**
- `ISenderFactory` là abstract interface với 2 pure virtual functions.
- Mỗi service (CoreMgr, HardwareMgr, RecordMgr) tạo factory riêng: `CMSenderFactory`, `HMSenderFactory`, `RMSenderFactory`.
- `DBusSenderBase` chỉ giữ `std::shared_ptr<ISenderFactory> msgMaker` → **polymorphism** — gọi `msgMaker->makeMsg(cmd)` mà không cần biết concrete type.

---

### 1.4 Template Method Pattern

**Khái niệm:** Base class định nghĩa "skeleton" của algorithm (template), subclass chỉ cần implement các bước cụ thể thông qua pure virtual function.

**📁 common/include/Thread/ThreadBase.hpp**
```cpp
class ThreadBase {
private:
    std::thread threadObj_;
    std::string threadName_;

protected:
    std::atomic<bool> runningFlag_;
    virtual void threadFunction() = 0;  // Pure virtual — subclass PHẢI implement
    virtual void onStop() {}            // Hook method — optional override

public:
    explicit ThreadBase(std::string threadName);
    virtual ~ThreadBase();

    void run();          // Template method — gọi threadFunction()
    void join();
    virtual void stop(); // Có thể override
};
```

**📁 common/src/Thread/ThreadBase.cpp** (Template method)
```cpp
void ThreadBase::run() {
    runningFlag_ = true;
    threadObj_ = std::thread(&ThreadBase::threadFunction, this); // Gọi virtual function
}

void ThreadBase::stop() {
    runningFlag_ = false;
    onStop();  // Hook — subclass có thể override
}
```

**Subclasses:** `MainWorker`, `DBusReceiver`, `DBThreadPool`, `Timer`, `BluetoothWorker`, `MonitorWorker`, `RecordWorker`

**Giải thích phỏng vấn:**
- `run()` là template method — nó gọi `threadFunction()` nhưng KHÔNG biết nội dung cụ thể.
- `threadFunction()` là pure virtual → mỗi subclass tự quyết nội dung thread.
- `onStop()` là **hook method** — mặc định không làm gì, nhưng `Timer` override để `notify_all()`.

---

### 1.5 Observer / Pub-Sub Pattern

**Khái niệm:** D-Bus signal và WebSocket broadcast tạo thành hệ thống Pub-Sub giữa các services.

**📁 coremgr/include/WS/WebSocketServer.hpp**
```cpp
class WebSocketServer {
public:
    void join(std::shared_ptr<WebSocketSession> session);    // Subscribe
    void leave(std::shared_ptr<WebSocketSession> session);   // Unsubscribe
    void updateStateAndBroadcast(...);                        // Notify all subscribers

private:
    std::set<std::shared_ptr<WebSocketSession>> sessions_;   // Observer list
    void broadcast(const std::string& message);              // Send to all
};
```

**Giải thích phỏng vấn:**
- Mỗi WebSocket client kết nối → `join()` → thêm vào `sessions_` set.
- Khi có event → `broadcast()` gửi message đến TẤT CẢ sessions đang connected.
- Khi disconnect → `leave()` → xóa khỏi set.
- D-Bus signals cũng hoạt động tương tự: service gửi signal → service khác lắng nghe.

---

### 1.6 Producer-Consumer Pattern

**Khái niệm:** Thread này tạo Event (producer), thread khác xử lý Event (consumer), sử dụng queue + mutex + condition_variable.

**📁 common/include/Event/EventQueue.hpp**
```cpp
class EventQueue {
public:
    void pushEvent(std::shared_ptr<Event> event);       // Producer
    std::shared_ptr<Event> waitAndPopEvent();            // Consumer — blocking
    void stop();

private:
    std::queue<std::shared_ptr<Event>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condVar;
    bool m_stopped = false;
};
```

**📁 common/src/Event/EventQueue.cpp**
```cpp
void EventQueue::pushEvent(std::shared_ptr<Event> event) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(event);
    }
    m_condVar.notify_one();  // Đánh thức consumer đang chờ
}

std::shared_ptr<Event> EventQueue::waitAndPopEvent() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condVar.wait(lock, [this]{ return !m_queue.empty() || m_stopped; }); // Block đến khi có event
    if (m_stopped) return nullptr;
    auto event = m_queue.front();
    m_queue.pop();
    return event;
}
```

**Giải thích phỏng vấn:**
- `pushEvent()`: dùng `lock_guard` (RAII lock) → push vào queue → `notify_one()`.
- `waitAndPopEvent()`: dùng `unique_lock` (vì `wait` cần unlock/relock) → `wait` với predicate → pop khi có data.
- **Predicate lambda** `[this]{ return !m_queue.empty() || m_stopped; }` → tránh **spurious wakeup**.

---

### 1.7 Dependency Injection Pattern

**Khái niệm:** Thay vì tự tạo dependency bên trong, object nhận dependency từ bên ngoài thông qua setter method.

**📁 coremgr/include/Controller/HardwareHandler.hpp**
```cpp
class HardwareHandler {
public:
    HardwareHandler() = default;
    ~HardwareHandler() = default;

    void setWebSocket(std::shared_ptr<WebSocket> webSocket) { webSocket_ = webSocket; }
    void setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) { dbThreadPool_ = dbThreadPool; }

private:
    std::shared_ptr<WebSocket> webSocket_;
    std::shared_ptr<DBThreadPool> dbThreadPool_;
};
```

**📁 coremgr/src/Thread/MainWorker.cpp** (Inject dependencies)
```cpp
// MainWorker tạo dependencies rồi inject vào handlers
hardwareHandler_.setWebSocket(webSocket_);
recordHandler_.setWebSocket(webSocket_);
sqliteDBHandler_.setDBThreadPool(dbThreadPool_);
sqliteDBHandler_.setWebSocket(webSocket_);
```

**Giải thích phỏng vấn:**
- Handler KHÔNG tự tạo `WebSocket` hay `DBThreadPool` → dễ test, dễ thay thế, loose coupling.
- `MainWorker` đóng vai trò **Composition Root** — tạo mọi thứ rồi inject vào.

---

### 1.8 State Machine Pattern

**Khái niệm:** Quản lý trạng thái của một tính năng thông qua enum class + switch-case, chặn hành vi không hợp lệ ở mỗi state.

**📁 coremgr/include/Configure/StateView.hpp**
```cpp
enum class RecordState {
    STOPPED = 0,
    RECORDING,
    PROCESSING
};

enum class BluetoothPowerState {
    OFF = 0,
    ON,
    PROCESSING
};

enum class CallState {
    IDLE = 0,
    PROCESSING,
    INCOMING,
    OUTCOMING,
    CALLING,
};
```

**📁 coremgr/src/Controller/HardwareHandler.cpp** (State transitions)
```cpp
void HardwareHandler::startScanBTDevice(){
    ScanningBTDeviceState currentState = STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE;
    switch (currentState) {
        case ScanningBTDeviceState::IDLE:
            STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::START_SCAN_BTDEVICE);
            break;
        case ScanningBTDeviceState::SCANNING:
            R_LOG(WARN, "Already SCANNING. No Action taken.");
            break;
        case ScanningBTDeviceState::PROCESSING:
            R_LOG(WARN, "In PROCESSING state. No Action taken.");
            break;
    }
}
```

**Giải thích phỏng vấn:**
- State transitions: `IDLE → PROCESSING → SCANNING → PROCESSING → IDLE`
- `PROCESSING` là trạng thái trung gian (pending) → ngăn double-click / duplicate request.
- Mỗi hàm **kiểm tra state hiện tại** trước khi thực thi → tránh race condition logic.

---

### 1.9 Thread Pool Pattern

**Khái niệm:** Tạo trước N threads, các task được đẩy vào queue, workers lấy task ra thực thi — tránh overhead tạo/hủy thread liên tục.

**📁 coremgr/include/Thread/DBThreadPool.hpp**
```cpp
class DBThreadPool : public ThreadBase {
public:
    DBThreadPool(std::shared_ptr<EventQueue> eventQueue, int numWorkers);
    
    void enqueueTask(std::function<void()> task);  // Đẩy task vào queue

    std::future<AudioRecord> insertAudioRecord(const std::string& filePath, int durationSec);
    std::future<std::vector<AudioRecord>> getAllAudioRecords();
    std::future<std::string> removeAudioRecord(int recordId);

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasksQueue_;
    std::mutex taskMutex_;
    std::condition_variable cv_;
};
```

**📁 coremgr/src/Thread/DBThreadPool.cpp**
```cpp
// Constructor — tạo N worker threads
for (int i = 0; i < numWorkers; i++) {
    workers_.emplace_back(&DBThreadPool::threadFunction, this);
}

// Worker thread — chờ task rồi thực thi
void DBThreadPool::threadFunction() {
    while (runningFlag_) {
        std::unique_lock<std::mutex> lock(taskMutex_);
        cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });
        
        auto task = std::move(tasksQueue_.front());
        tasksQueue_.pop();
        lock.unlock();
        
        task();  // Execute task
    }
}

// Enqueue — đẩy task + std::move
void DBThreadPool::enqueueTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(taskMutex_);
        tasksQueue_.push(std::move(task));   // Move semantics!
    }
    cv_.notify_one();
}
```

---

## 2. C++17 Features

### 2.1 std::optional

**Khái niệm:** Đại diện cho giá trị "có thể có hoặc không" — thay thế cho magic values (-1, nullptr) hoặc out-parameter.

**📁 coremgr/include/Util/JsonHelper.hpp**
```cpp
#include <optional>

class JsonHelper {
public:
    std::optional<int> getIntField(const json& data, const std::string& key);
    std::optional<std::string> getStringField(const json& data, const std::string& key);
};
```

**📁 coremgr/src/Util/JsonHelper.cpp**
```cpp
std::optional<int> JsonHelper::getIntField(const json& data, const std::string& key) {
    if (data.contains(key)) {
        const auto& value = data[key];
        if (value.is_number()) {
            return value.get<int>();      // Có giá trị → trả về std::optional chứa int
        }
    }
    return std::nullopt;                  // Không có → trả về empty optional
}
```

**Giải thích phỏng vấn:**
- `std::optional<int>` rõ ràng hơn `int` return -1 khi lỗi.
- Caller kiểm tra: `if (auto val = helper->getIntField(data, "key")) { use *val; }`
- `std::nullopt` — giá trị empty, tương tự `nullptr` cho optional.

---

### 2.2 std::filesystem

**Khái niệm:** API chuẩn C++17 để thao tác file/directory — thay thế POSIX `stat`, `mkdir`, `remove`.

**📁 coremgr/src/Controller/SQLiteDBHandler.cpp**
```cpp
#include <filesystem>

void SQLiteDBHandler::removeAudioRecord(std::shared_ptr<Payload> payload) {
    // ... get filePath from DB ...
    try {
        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::remove(filePath)) {
                R_LOG(INFO, "Successfully deleted file: %s", filePath.c_str());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        R_LOG(ERROR, "Filesystem error: %s", e.what());
    }
}
```

**📁 recordmgr (AudioFilter.cpp, SQLiteDatabase.cpp)** — cũng dùng `std::filesystem` cho:
- `std::filesystem::exists()` — kiểm tra file tồn tại
- `std::filesystem::remove()` — xóa file
- `std::filesystem::create_directories()` — tạo thư mục đệ quy

---

### 2.3 inline static member variables

**Khái niệm:** C++17 cho phép khởi tạo `static` member trực tiếp trong header file mà KHÔNG cần define trong `.cpp` — rất tiện cho config/constants.

**📁 coremgr/include/Configure/Config.hpp**
```cpp
class Config : public IConfig {
private:
    inline static const std::string COREMGR_SERVICE_NAME = "com.example.coremanager";
    inline static const std::string COREMGR_OBJECT_PATH = "/com/example/coremanager";
    inline static const unsigned short WEBSOCKET_PORT = 9000;
    inline static const unsigned int SQLiteDBWorkerThreads = 5;
    // ... 
};
```

**📁 coremgr/include/Configure/StateView.hpp**
```cpp
class StateView {
public:
    inline static RecordState RECORD_STATE = RecordState::STOPPED;
    inline static int CURRENT_TEMPERATURE = 0;
    inline static BluetoothPowerState BLUETOOTH_POWER_STATE = BluetoothPowerState::OFF;
    inline static CallState CALL_STATE = CallState::IDLE;
};
```

**📁 coremgr/include/Thread/Timer.hpp**
```cpp
inline static std::once_flag onceFlag = {};
```

**Giải thích phỏng vấn:**
- Trước C++17: phải khai báo `static const std::string X;` trong header, rồi define `const std::string Config::X = "..."` trong `.cpp`.
- C++17 `inline static`: khai báo + khởi tạo ngay trong header → đơn giản, ít file hơn.
- `inline` đảm bảo chỉ có 1 definition dù header được include ở nhiều nơi.

---

### 2.4 Structured Bindings

**Khái niệm:** C++17 cho phép "unpack" struct/pair/tuple thành biến riêng.

**📁 hardwaremgr/src/BT/BluetoothAgent.cpp**
```cpp
for (auto const& [address, msg] : pendingRequests_) {
    // address = key (std::string)
    // msg = value (DBusMessage*)
    dbus_message_unref(msg);
}
```

**Giải thích phỏng vấn:**
- `auto const& [address, msg]` — unpack `std::pair<const std::string, DBusMessage*>` thành 2 biến.
- Tương đương `pair.first` và `pair.second` nhưng dễ đọc hơn nhiều.

---

### 2.5 [[maybe_unused]] Attribute

**Khái niệm:** Báo cho compiler biết biến CÓ THỂ không dùng → tắt warning.

**📁 coremgr/src/WS/WebSocketServer.cpp**
```cpp
void WebSocketServer::doAccept() {
    acceptor_.async_accept([this]([[maybe_unused]] boost::system::error_code ec,
                                   boost::asio::ip::tcp::socket socket) {
        // ec có thể không dùng nhưng signature yêu cầu
    });
}
```

---

### 2.6 std::call_once / std::once_flag

**Đã mô tả chi tiết ở [1.2 Singleton Pattern (std::call_once)](#12-singleton-pattern-stdcall_once)**

```cpp
inline static std::once_flag onceFlag = {};

std::call_once(onceFlag, []() {
    instance.reset(new Timer());
    instance->run();
});
```

---

### 2.7 std::any (include)

**📁 common/include/DBus/DBusData.hpp**
```cpp
#include <any>  // C++17 type-safe container for any type
```

Được include nhưng cuối cùng project dùng `std::string data[]` thay vì `std::any` cho D-Bus data. Cho thấy awareness về C++17 type-erasure.

---

## 3. OOP — Lập Trình Hướng Đối Tượng

### 3.1 Abstract Class / Pure Virtual Function

**Khái niệm:** Class có ít nhất 1 pure virtual function (`= 0`) → không thể tạo instance → bắt buộc subclass implement.

**📁 common/include/Thread/ThreadBase.hpp**
```cpp
class ThreadBase {
protected:
    virtual void threadFunction() = 0;  // Pure virtual
    virtual void onStop() {}            // Virtual với default implementation (hook)
public:
    virtual ~ThreadBase();              // Virtual destructor
};
```

**📁 common/include/Configure/IConfig.hpp**
```cpp
class IConfig {
public:
    IConfig() = default;
    virtual ~IConfig() = default;

    virtual const std::string &getBinaryPath() const = 0;       // Pure virtual
    virtual const std::string &getServiceName() const = 0;      // Pure virtual
    virtual const std::string &getObjectPath() const = 0;       // Pure virtual
    virtual const std::string &getInterfaceName() const = 0;    // Pure virtual
    virtual const std::string &getSignalName() const = 0;       // Pure virtual
};
```

**📁 common/include/DBus/ISenderFactory.hpp**
```cpp
class ISenderFactory {
public:
    virtual ~ISenderFactory() = default;
    virtual DBusMessage* makeMsg(DBusCommand cmd) = 0;
    virtual DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) = 0;
};
```

**📁 common/include/Event/Event.hpp**
```cpp
class Payload {
public:
    Payload() {}
    virtual ~Payload() {}   // Virtual destructor — base class cho polymorphism
};
```

---

### 3.2 Virtual Destructor

**Khái niệm:** Khi xóa object thông qua base pointer, cần virtual destructor để gọi đúng destructor của derived class → tránh memory leak.

```cpp
// ✅ Đúng — Virtual destructor
class Payload {
    virtual ~Payload() {}   // Khi delete shared_ptr<Payload> → gọi ~NotiPayload()
};

class ThreadBase {
    virtual ~ThreadBase();  // Khi delete ThreadBase* → gọi ~MainWorker()
};

class ISenderFactory {
    virtual ~ISenderFactory() = default;
};

class IConfig {
    virtual ~IConfig() = default;
};
```

---

### 3.3 override Specifier

**Khái niệm:** Đánh dấu function là override virtual function từ base class → compiler báo lỗi nếu không match.

```cpp
// coremgr/include/DBus/CMSenderFactory.hpp
class CMSenderFactory : public ISenderFactory {
    DBusMessage* makeMsg(DBusCommand cmd) override;      // Override pure virtual
    ~CMSenderFactory() override = default;               // Override virtual dtor
};

// coremgr/include/Thread/Timer.hpp
class Timer : public ThreadBase {
    void threadFunction() override;   // Override pure virtual
    void onStop() override;           // Override hook method
};

// coremgr/include/Log/RLogger.hpp
class RLogger : public Logger {
    ~RLogger() override = default;    // Override virtual dtor
};
```

---

### 3.4 = default / = delete

**Khái niệm:**
- `= default`: yêu cầu compiler tự generate function (constructor, destructor, copy, move).
- `= delete`: xóa function, compiler báo lỗi nếu ai gọi.

```cpp
// = default
Config() = default;              // Default constructor do compiler generate
~Config() = default;             // Default destructor
IConfig() = default;
~IConfig() = default;
CMSenderFactory() = default;
~CMSenderFactory() override = default;

// = delete (Singleton)
Config(const Config &) = delete;            // Chặn copy constructor
Config &operator=(const Config &) = delete; // Chặn copy assignment

// Delete cả move (Timer)
Timer(Timer &&) noexcept = delete;              // Chặn move constructor
Timer &operator=(Timer &&) noexcept = delete;   // Chặn move assignment
```

---

### 3.5 explicit Constructor

**Khái niệm:** Ngăn **implicit conversion** — bắt buộc gọi constructor rõ ràng.

```cpp
// common/include/Event/Event.hpp
class NotiPayload : public Payload {
    explicit NotiPayload(bool isSuccess = false, const std::string &msgInfo = "")
        : isSuccess_(isSuccess), msgInfo_(msgInfo) {}
};

class BluetoothDevicePayload : public Payload {
    explicit BluetoothDevicePayload(const std::string &name, const std::string &address, 
                                     int rssi, bool isPaired, bool isConnected, const std::string &icon);
};

// common/include/Thread/ThreadBase.hpp
explicit ThreadBase(std::string threadName);

// common/include/Log/Logger.hpp
explicit Logger(std::string moduleName);

// WebSocketServer.hpp
explicit WebSocketSession(boost::asio::ip::tcp::socket socket, WebSocketServer& server);
```

**Giải thích phỏng vấn:**
- Không có `explicit`: `Payload p = true;` → compiler tự convert bool → NotiPayload (nguy hiểm!).
- Có `explicit`: bắt buộc `NotiPayload p(true, "msg");` → rõ ràng intention.

---

### 3.6 Polymorphic Inheritance Hierarchy

**Khái niệm:** Base class `Payload` + 11 derived classes tạo thành cây thừa kế, sử dụng `std::shared_ptr<Payload>` để truyền data mà không cần biết concrete type.

```
Payload (base, virtual dtor)
├── NotiPayload
├── NotiTemperaturePayload
├── NotiBTDeviceAddressPayload
├── ContactPayload
├── CallHistoryPayload
├── CallPayload
├── BluetoothDevicePayload
├── BluetoothDeviceAddressPayload
├── BluetoothDevicePasskeyPayload
├── WavPayload
└── RemoveRecordPayload
```

```cpp
// Tạo derived, lưu vào shared_ptr<Payload>
std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(address);
std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::..., payload);

// Truyền qua EventQueue dưới dạng shared_ptr<Payload>
eventQueue_->pushEvent(event);
```

**Tương tự:** `ISenderFactory` → `CMSenderFactory`, `HMSenderFactory`, `RMSenderFactory`

**Ngoài ra:** Project cũng sử dụng **plain structs** (non-polymorphic) cho data transfer:
```cpp
// hardwaremgr/include/BT/ObexPbapClient.hpp
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
```
→ Structs đơn giản cho PBAP data, không cần virtual — khác với Payload hierarchy.

---

### 3.7 std::dynamic_pointer_cast (Downcasting)

**Khái niệm:** An toàn cast từ `shared_ptr<Base>` xuống `shared_ptr<Derived>` — trả về `nullptr` nếu sai type.

**📁 coremgr/src/Controller/HardwareHandler.cpp** (sử dụng RẤT NHIỀU)
```cpp
void HardwareHandler::pairBTDevice(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = 
        std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    
    if (btPayload == nullptr) {
        R_LOG(ERROR, "PAIR_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    // ...
}
```

**Giải thích phỏng vấn:**
- `dynamic_pointer_cast` sử dụng RTTI (Run-Time Type Information) để kiểm tra type lúc runtime.
- Nếu `payload` thực sự là `BluetoothDeviceAddressPayload` → trả về shared_ptr hợp lệ.
- Nếu không → trả về `nullptr` → safe, không crash.
- Pattern: **cast → null check → use** — áp dụng nhất quán trong toàn bộ project.

---

### 3.8 Forward Declarations

**Khái niệm:** Khai báo class mà KHÔNG include header → giảm dependency, tăng tốc compile.

**📁 coremgr/include/Thread/MainWorker.hpp** (8 forward declarations!)
```cpp
class EventQueue;
class WebSocket;
class DBusSender;
class DBThreadPool;
class HardwareHandler;
class RecordHandler;
class SQLiteDBHandler;
class Payload;
```

**Giải thích phỏng vấn:**
- Header chỉ dùng pointer/reference đến class → không cần full definition → forward declare đủ.
- Giảm transitive includes → compile nhanh hơn, tránh circular dependency.
- `.cpp` file mới `#include` đầy đủ headers.

---

### 3.9 Operator Overloading

**Khái niệm:** Override operator `[]` để truy cập data bằng enum key.

**📁 common/include/DBus/DBusData.hpp**
```cpp
struct DBusDataInfo {
    std::string data[DBUS_DATA_MAX];

    std::string& operator[](DBusDataType type) {
        return data[type];
    }

    const std::string& operator[](DBusDataType type) const {
        return data[type];
    }
};

// Sử dụng:
DBusDataInfo data;
data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;  // Gán bằng [] operator
```

---

## 4. Smart Pointers & Memory Management

### 4.1 std::shared_ptr / std::make_shared

**Sử dụng khắp nơi trong project:**

```cpp
// Tạo shared objects
std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
std::shared_ptr<DBThreadPool> dbThreadPool = std::make_shared<DBThreadPool>(eventQueue, numWorkers);
std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::..., payload);
std::shared_ptr<TimerElement> timerElement = std::make_shared<TimerElement>();

// Factory injection
std::shared_ptr<ISenderFactory> msgMaker = std::make_shared<CMSenderFactory>();

// Payload polymorphism
std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(address);
```

**Giải thích phỏng vấn:**
- `std::make_shared` hiệu quả hơn `new` — 1 lần allocation (object + control block).
- Reference counting → tự giải phóng khi count = 0 → không memory leak.
- Truyền `shared_ptr` giữa threads an toàn (control block thread-safe).

---

### 4.2 std::unique_ptr / std::make_unique

**📁 coremgr/src/Thread/Timer.cpp**
```cpp
static std::unique_ptr<Timer> instance;
instance.reset(new Timer());  // unique_ptr quản lý Timer singleton
```

**Giải thích phỏng vấn:**
- `unique_ptr` — exclusive ownership, không copy được, chỉ move.
- Timer dùng `unique_ptr` vì singleton chỉ có 1 owner → phù hợp hơn `shared_ptr`.

---

### 4.3 std::enable_shared_from_this

**Khái niệm:** Cho phép object tạo `shared_ptr` trỏ đến chính nó từ bên trong method.

**📁 coremgr/include/WS/WebSocketServer.hpp**
```cpp
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    void start();
    void send(const std::string& message);
    // ...
};
```

**Giải thích phỏng vấn:**
- Boost.Asio async handlers cần giữ object alive → `shared_from_this()` tạo `shared_ptr` từ `this`.
- Nếu dùng raw `this` → object có thể bị destroy trước khi callback chạy → dangling pointer!
- `enable_shared_from_this` đảm bảo object sống đủ lâu cho tất cả async operations.

---

### 4.4 RAII (Resource Acquisition Is Initialization)

**Khái niệm:** Resource được acquire trong constructor, release trong destructor → đảm bảo không leak kể cả khi exception.

```cpp
// Lock mutex
{
    std::lock_guard<std::mutex> lock(m_mutex);   // Acquire lock
    m_queue.push(event);
}   // Release lock tự động — kể cả exception

// Unique lock cho condition_variable
std::unique_lock<std::mutex> lock(taskMutex_);
cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });

// Smart pointers = RAII cho memory
std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
// Tự giải phóng khi out of scope

// Thread join in destructor
ThreadBase::~ThreadBase() {
    // Ensure thread is joined
}

DBThreadPool::~DBThreadPool() {
    for (auto& worker : workers_) {
        if (worker.joinable()) worker.join();
    }
    if (database_) database_->close();
}
```

---

## 5. Modern C++ Features

### 5.1 Lambda Expressions

**Sử dụng rất nhiều trong project:**

```cpp
// 1. Condition variable predicate
cv_.wait(lock, [this]{ return !tasksQueue_.empty() || !runningFlag_; });

// 2. Timer predicate với capture
timerWakeupCondition.wait_until(lock, nextItemTimeout, [this, mapSize]() -> bool {
    return (!runningFlag_ || timerTableMap.size() != mapSize);
});

// 3. std::call_once initialization
std::call_once(onceFlag, []() {
    instance.reset(new Timer());
    instance->run();
});

// 4. Thread pool task — capture by value
auto task = std::make_shared<std::packaged_task<AudioRecord()>>(
    [this, filePath, durationSec]() {
        AudioRecord record;
        record.filePath = filePath;
        record.durationSec = durationSec;
        // ...
        return newRecord;
    }
);

// 5. Wrapper lambda
enqueueTask([task](){ (*task)(); });

// 6. Async accept callback
acceptor_.async_accept([this]([[maybe_unused]] boost::system::error_code ec,
                               boost::asio::ip::tcp::socket socket) {
    auto session = std::make_shared<WebSocketSession>(std::move(socket), *this);
    session->start();
    doAccept();
});
```

**Các loại capture sử dụng:**
- `[this]` — capture `this` pointer
- `[this, mapSize]` — capture `this` + copy of local variable
- `[this, filePath, durationSec]` — capture `this` + copy of strings
- `[task]` — capture shared_ptr by value (tăng ref count)
- `[]()` — no capture
- `[&]` — capture all by reference
- `[pbap, ofono, deviceAddress]` — capture multiple shared_ptr + string by value (PBAP sync thread)

```cpp
// 7. Detached thread for PBAP sync — capture shared_ptr by value to extend lifetime
// hardwaremgr/src/Thread/BluetoothWorker.cpp
auto pbap = pbapClient_;
auto ofono = ofonoDBus_;
std::thread([pbap, ofono, deviceAddress]() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    pbap->createSession(deviceAddress);
    auto contacts = pbap->pullPhonebook();
    // Feed contacts to OfonoDBus for caller ID
    if (!contacts.empty() && ofono) {
        std::unordered_map<std::string, std::string> phonebook;
        for (const auto& contact : contacts) {
            phonebook[contact.number] = contact.name;
        }
        ofono->setPhonebook(phonebook);
    }
    pbap->pullCallHistory();
}).detach();  // Detach: thread tự chạy, không block BluetoothWorker
```

**Giải thích phỏng vấn:**
- Capture `shared_ptr` **by value** → tăng reference count → đảm bảo `pbapClient_` và `ofonoDBus_` không bị destroy trước khi thread kết thúc.
- `.detach()` thay vì `.join()` → thread chạy nền, không chặn D-Bus message loop.
- `deviceAddress` capture by value (copy) vì biến local có thể bị destroy sau khi `triggerPbapSync()` return.

---

### 5.2 Move Semantics (std::move)

```cpp
// Move task vào queue (tránh copy std::function)
tasksQueue_.push(std::move(task));

// Move task ra khỏi queue
auto task = std::move(tasksQueue_.front());

// Move socket (non-copyable) vào WebSocketSession
auto session = std::make_shared<WebSocketSession>(std::move(socket), *this);

// emplace_back với move
workers_.emplace_back(&DBThreadPool::threadFunction, this);
```

**Giải thích phỏng vấn:**
- `std::move()` cast thành rvalue reference → cho phép "steal" resources thay vì copy.
- `boost::asio::ip::tcp::socket` là non-copyable → bắt buộc move.
- `std::function<void()>` có thể chứa captures lớn → move hiệu quả hơn copy.

---

### 5.3 std::function (Callback)

```cpp
// WebSocket message handler callback
using MessageHandler = std::function<void(const std::string&)>;

class WebSocketServer {
    MessageHandler messageHandler_;
    
public:
    WebSocketServer(const std::string& host, unsigned short port, MessageHandler handler);
};

// Thread pool task type
void enqueueTask(std::function<void()> task);
```

**Giải thích phỏng vấn:**
- `std::function` là type-erased callable wrapper → chứa function pointer, lambda, functor.
- `MessageHandler` cho phép WebSocketServer gọi bất kỳ function nào phù hợp signature.

---

### 5.4 enum class (Scoped Enumerations)

```cpp
// Scoped — cần prefix DBusCommand::
enum class DBusCommand {
    NONE = 0,
    INITIALIZE_BLUETOOTH,
    START_SCAN_BTDEVICE,
    // ...
    MAX
};

enum class RecordState { STOPPED = 0, RECORDING, PROCESSING };
enum class ScanningBTDeviceState { IDLE = 0, SCANNING, PROCESSING };
enum class BluetoothPowerState { OFF = 0, ON, PROCESSING };
enum class CallState { IDLE = 0, PROCESSING, INCOMING, OUTCOMING, CALLING };
enum class EventTypeID;

// Unscoped (legacy)
enum LogLevel { DEBUG = 0, INFO, WARN, ERROR, MAX };
enum DBusDataType { DBUS_DATA_MESSAGE = 0, /* ... */ };
```

**Giải thích phỏng vấn:**
- `enum class` → strong typing, PHẢI dùng `DBusCommand::START_SCAN_BTDEVICE`.
- `enum` (unscoped) → weak typing, `DEBUG` có thể conflict với tên khác.
- Project dùng `enum class` cho business logic, `enum` cho data/log level (ít conflict hơn).

---

### 5.5 Type Alias (using)

```cpp
// coremgr/include/Util/JsonHelper.hpp
using json = nlohmann::json;

// coremgr/include/WS/WebSocketServer.hpp
using MessageHandler = std::function<void(const std::string&)>;
```

**Giải thích phỏng vấn:**
- `using` thay thế `typedef` — dễ đọc hơn, hỗ trợ template alias.
- `using json = nlohmann::json` → viết `json data;` thay vì `nlohmann::json data;`.

---

### 5.6 Raw String Literals R"(...)"

**📁 coremgr/src/DB/SQLiteDatabase.cpp**
```cpp
const char* createTableSQL = R"(
    CREATE TABLE IF NOT EXISTS audio_records (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        file_path TEXT NOT NULL UNIQUE,
        duration_sec INTEGER DEFAULT 0
    );
)";
```

**Giải thích phỏng vấn:**
- `R"(...)"` — raw string, giữ nguyên newlines, tabs, không cần escape.
- Rất phù hợp cho SQL queries, JSON templates, regex patterns.

---

### 5.7 std::future / std::packaged_task / std::promise

**📁 coremgr/src/Thread/DBThreadPool.cpp**
```cpp
std::future<AudioRecord> DBThreadPool::insertAudioRecord(const std::string& filePath, int durationSec) {
    // packaged_task wrap lambda thành callable + future
    auto task = std::make_shared<std::packaged_task<AudioRecord()>>(
        [this, filePath, durationSec]() {
            AudioRecord record;
            record.filePath = filePath;
            record.durationSec = durationSec;
            std::lock_guard<std::mutex> dbLock(dbMutex_);
            return database_->insertAudioRecord(record);
        }
    );

    auto future = task->get_future();              // Lấy future TRƯỚC khi enqueue
    enqueueTask([task](){ (*task)(); });            // Enqueue task
    return future;                                  // Trả future cho caller
}

// Caller chờ kết quả
auto future = dbThreadPool_->insertAudioRecord(filePath, durationSec);
AudioRecord newRecord = future.get();  // Blocking — chờ worker thread hoàn thành
```

**Giải thích phỏng vấn:**
- `std::packaged_task` wrap callable → có thể lấy `future` của return value.
- `std::future::get()` block đến khi result sẵn sàng.
- Pattern: wrap lambda → shared_ptr<packaged_task> → get_future() → enqueue → return future.
- Dùng `shared_ptr<packaged_task>` vì `packaged_task` non-copyable, cần capture trong lambda.

---

### 5.8 std::chrono (Time Library)

**📁 coremgr/src/Thread/Timer.cpp**
```cpp
#include <chrono>

// Lấy thời gian hiện tại
std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

// Tính thời gian hết hạn
timerElement->expireTime = std::chrono::steady_clock::now() 
    + std::chrono::milliseconds(timeout_ms);

// So sánh thời gian
if (timerElement->expireTime <= currentTime) {
    // Timer đã hết hạn
}

// Sleep có timeout
const std::chrono::steady_clock::time_point defaultTimeout =
    currentTime + std::chrono::milliseconds(INTERNAL_SLEEP_TIMEOUT_MS);

timerWakeupCondition.wait_until(lock, nextItemTimeout, predicate);
```

**Giải thích phỏng vấn:**
- `steady_clock` — monotonic, không bị ảnh hưởng bởi system time change → phù hợp cho timer.
- `std::chrono::milliseconds` — type-safe time duration.
- `wait_until` — sleep đến thời điểm cụ thể hoặc khi predicate true.

---

### 5.9 Range-based for loop / auto

```cpp
// Range-based for
for (const auto& record : vec) {
    recordJson["id"] = record.id;
}

for (auto& worker : workers_) {
    if (worker.joinable()) worker.join();
}

for (const auto& timerElement : expiredTimerElementList) {
    eventQueue_->pushEvent(timerElement->event);
}

// Structured binding (C++17)
for (auto const& [address, msg] : pendingRequests_) {
    dbus_message_unref(msg);
}

// auto type deduction
auto future = task->get_future();
auto task = std::move(tasksQueue_.front());
const auto& value = data[key];
```

---

### 5.10 Initializer List { }

```cpp
// nlohmann::json initializer list
webSocket_->getServer()->updateStateAndBroadcast("success", 
    "Scanning Bluetooth device found.",
    "Settings", "scanning_btdevice_found_noti", {
        {"device_name", btPayload->getName()},
        {"device_address", btPayload->getAddress()},
        {"rssi", btPayload->getRssi()},
        {"is_paired", btPayload->isPaired()},
        {"is_connected", btPayload->isConnected()},
        {"icon", btPayload->getIcon()}
    });

// Empty initializer list  
webSocket_->getServer()->updateStateAndBroadcast("success", msg, "Record", "start_record_noti", {});
```

---

## 6. Preprocessor & Macro Techniques

### 6.1 Variadic Macro with __VA_ARGS__

```cpp
// common/include/Log/Logger.hpp
#define CMN_LOG(level, fmt, ...) \
    Logger("Common").printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);

// coremgr/include/Log/RLogger.hpp
#define R_LOG(level, fmt, ...) \
    RLogger::getInstance()->printLog(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);
```

**Giải thích phỏng vấn:**
- `__FILE__`, `__LINE__`, `__func__` — predefined macros cho debug info tự động.
- `##__VA_ARGS__` — GCC extension: nếu không có extra args, `##` xóa dấu `,` phía trước → tránh lỗi.
- Macro thay vì function → `__FILE__` và `__LINE__` sẽ refer đến **caller's location**, không phải Logger.

---

### 6.2 Macro Constants as Instance Accessors

```cpp
#define CONFIG_INSTANCE()       Config::getInstance()
#define STATE_VIEW_INSTANCE()   StateView::getInstance()
#define DBUS_SENDER()           DBusSender::getInstance()
#define TIMER_INSTANCE()        Timer::getInstance()
#define JSON_HELPER_INSTANCE()  JsonHelper::getInstance()
```

**Giải thích phỏng vấn:**
- Rút gọn code — `STATE_VIEW_INSTANCE()->CALL_STATE` thay vì `StateView::getInstance()->CALL_STATE`.
- Tất cả Singletons đều có macro tương ứng → coding convention nhất quán.

---

### 6.3 Include Guard (#ifndef)

```cpp
#ifndef CONFIG_HPP_
#define CONFIG_HPP_

// ... header content ...

#endif // CONFIG_HPP_
```

**Toàn bộ** header files đều sử dụng include guard → ngăn multiple inclusion.

---

## 7. STL Containers & Algorithms

| Container | Sử dụng | File |
|-----------|---------|------|
| `std::queue` | Event queue, message queue, task queue | EventQueue, WebSocketSession, DBThreadPool |
| `std::vector` | Worker threads, expired timers, DB results | DBThreadPool, Timer, SQLiteDatabase |
| `std::unordered_map` | Timer table, device state | Timer, HardwareHandler |
| `std::set` | WebSocket sessions (ordered) | WebSocketServer |
| `std::string` | Everywhere | All files |
| `std::array` (implied) | DBusDataInfo.data[MAX] | DBusData.hpp |
| `std::pair` (implicit) | map iteration, structured bindings | BluetoothAgent |
| `std::istringstream` | Parse VCF text data line-by-line | ObexPbapClient |
| `std::stringstream` | Read file contents into string | ObexPbapClient |

```cpp
// unordered_map — O(1) lookup
std::unordered_map<int32_t, std::shared_ptr<TimerElement>> timerTableMap;
std::unordered_map<std::string, int32_t> timerIdMap_;
std::unordered_map<std::string, std::string> phonebook_; // <Number, Name> for caller ID

// set — unique, ordered sessions
std::set<std::shared_ptr<WebSocketSession>> sessions_;

// vector — worker threads
std::vector<std::thread> workers_;
std::vector<VCardContact> contacts; // PBAP parsed contacts

// queue — FIFO tasks
std::queue<std::function<void()>> tasksQueue_;
std::queue<std::shared_ptr<Event>> m_queue;

// istringstream — VCF text parsing
std::istringstream stream(vcfData);
std::string line;
while (std::getline(stream, line)) {
    // Parse FN:, TEL:, N:, X-IRMC-CALL-DATETIME fields
}

// stringstream — read entire file
std::stringstream ss;
ss << file.rdbuf();
return ss.str();
```

---

## 8. Variadic Functions (C-style)

**📁 common/src/Log/Logger.cpp**
```cpp
#include <cstdarg>

void Logger::printLog(LogLevel level, const char* file, int line, 
                       const char* func, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list argsCopy;
    va_copy(argsCopy, args);      // Copy để tính size trước

    int size = vsnprintf(nullptr, 0, fmt, argsCopy);
    va_end(argsCopy);

    std::vector<char> buffer(size + 1);
    vsnprintf(buffer.data(), buffer.size(), fmt, args);
    va_end(args);
    
    // Print with timestamp, module, level, file:line:func
}
```

**Giải thích phỏng vấn:**
- `va_list`, `va_start`, `va_copy`, `va_end` — C-style variadic function.
- `vsnprintf(nullptr, 0, fmt, args)` — tính kích thước output trước khi allocate buffer.
- `va_copy` cần thiết vì `va_list` bị consume sau `vsnprintf`.
- Tại sao C-style? → Vì macro `R_LOG(level, fmt, ...)` dùng `__VA_ARGS__` (preprocessor) → cần C-style variadic.

---

## 9. Đánh Giá Tổng Quan: Có Chuyên Nghiệp Không?

### ✅ Điểm Mạnh (Professional)

| Tiêu chí | Đánh giá |
|-----------|----------|
| **Design Patterns** | 9 patterns: Singleton, Factory, Template Method, Observer, Producer-Consumer, DI, State Machine, Thread Pool, RAII |
| **C++17 Usage** | `std::optional`, `std::filesystem`, `inline static`, structured bindings, `[[maybe_unused]]`, `std::call_once` |
| **Memory Safety** | `shared_ptr` / `unique_ptr` everywhere, RAII locks, no raw `new`/`delete` (ngoại trừ `new Timer()` trong `call_once`) |
| **Thread Safety** | `mutex` + `lock_guard` / `unique_lock`, `condition_variable`, `std::atomic`, `std::call_once` |
| **Polymorphism** | Abstract classes, virtual dtors, `override`, `dynamic_pointer_cast` downcasting |
| **Code Organization** | Separation of concerns: common lib, 3 independent services, handler classes |
| **Build System** | CMake + cross-compilation toolchain → professional embedded development |
| **Coding Convention** | Include guards, forward declarations, `explicit`, `const`, macro accessors |
| **Error Handling** | `try-catch` for `std::filesystem`, `std::out_of_range`; null checks after `dynamic_pointer_cast` |

### ⚠️ Có Thể Cải Thiện

| Tiêu chí | Gợi ý |
|-----------|-------|
| `#pragma once` | Có thể dùng thay `#ifndef` include guards (hầu hết compiler hỗ trợ) |
| `constexpr` | Có thể dùng cho compile-time constants thay `#define TIMEOUT_MS` |
| `std::variant` | Có thể thay thế Payload hierarchy bằng `std::variant<Noti, BT, Call, ...>` (nếu set cố định) |
| Smart pointer cho DBus | `DBusConnection*`, `DBusMessage*` vẫn là raw pointer → có thể wrap bằng custom deleter `unique_ptr` |
| Unit Tests | Chưa thấy testing framework (Google Test, Catch2) |
| `noexcept` | Một số destructors và move operations có thể mark `noexcept` |
| `std::string_view` | Có thể dùng cho parameters `const std::string&` → tránh allocation |

### 📊 Tổng Kết

```
Design Patterns:     ★★★★★ (9 patterns thực tế)
C++17 Features:      ★★★★☆ (6/10 features phổ biến)  
OOP Principles:      ★★★★★ (polymorphism, abstraction, encapsulation)
Memory Management:   ★★★★★ (smart pointers, RAII)
Thread Safety:       ★★★★★ (mutex, condition_variable, atomic)
Code Organization:   ★★★★☆ (tốt, có thể thêm namespaces)
Error Handling:      ★★★★☆ (có try-catch, null checks)
Testing:             ★★☆☆☆ (chưa có unit tests)
```

**Kết luận:** Project thể hiện trình độ C++ **intermediate-to-advanced**, sử dụng đúng các design patterns và modern C++ features trong bối cảnh embedded Linux service development. Phù hợp để present trong phỏng vấn C++ system programming / embedded software engineer.
