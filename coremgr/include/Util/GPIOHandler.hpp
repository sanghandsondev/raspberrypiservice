#ifndef GPIO_HANDLER_HPP_
#define GPIO_HANDLER_HPP_

enum class LEDState {
    OFF = 0,
    ON,
    BLINKING,
    PROCESSING
};

class GPIOHandler {
    public:
        explicit GPIOHandler() {};
        virtual ~GPIOHandler() {};

        // bool OnOffLED(std::shared_ptr<Payload> payload);
        // bool blinkLED();

    private:
        LEDState ledState_;
};

#endif // GPIO_HANDLER_HPP_