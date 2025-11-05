#include <sdbus-c++/sdbus-c++.h>
#include <chrono>
#include <thread>
#include <iostream>

int main()
{
    // DBus service and object path for coremanager
    const char* serviceName = "com.example.coremanager";
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager";
    const char* methodName = "Ping"; // Thay bằng method thực tế nếu khác

    while (true)
    {
        try
        {
            // Create a connection to the system bus
            auto connection = sdbus::createSystemBusConnection();

            // Create a proxy object for the coremanager service
            auto proxy = sdbus::createProxy(*connection, serviceName, objectPath);

            // Call the method (không có tham số, không nhận giá trị trả về)
            proxy->callMethod(methodName).onInterface(interfaceName);

            std::cout << "Sent DBus message to coremanager." << std::endl;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "DBus call failed: " << ex.what() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}