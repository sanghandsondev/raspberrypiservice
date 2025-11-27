#ifndef GPIO_HPP_
#define GPIO_HPP_

class GPIO {
    public:
        GPIO() = default;
        ~GPIO() = default;

        bool readTemperatureSensor(float& temperature);

    private:
        
};

#endif // GPIO_HPP_