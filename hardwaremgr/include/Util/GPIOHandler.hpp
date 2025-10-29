#ifndef GPIO_HANDLER_HPP_
#define GPIO_HANDLER_HPP_

enum class SwitchState {
    // RELEASED = 0,
    // PRESSED
};

enum class LEDState {
    OFF = 0,
    ON,
    // BLINKING
};

class GPIOHandler {
    public:
        explicit GPIOHandler() {};
        virtual ~GPIOHandler() {};

        // bool OnOffLED(std::shared_ptr<Payload> payload);
        // bool blinkLED();

    private:
        LEDState ledState_;
        SwitchState switchState_;
};

#endif // GPIO_HANDLER_HPP_