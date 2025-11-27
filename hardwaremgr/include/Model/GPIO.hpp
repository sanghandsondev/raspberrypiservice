#ifndef GPIO_HPP_
#define GPIO_HPP_

#include <string>

// https://pinout.xyz/pinout/1_wire

class GPIO {
    public:
        GPIO();
        ~GPIO() = default;

        bool readTemperatureSensor(float& temperature);

    private:
        void findSensorFile();
        std::string sensorFile_;
};

#endif // GPIO_HPP_