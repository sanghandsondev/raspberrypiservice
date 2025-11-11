#ifndef STATE_VIEW_HPP_
#define STATE_VIEW_HPP_

#include "Define.hpp"

#define STATE_VIEW() StateView::getInstance()

enum class LEDState {
    OFF = 0,
    ON,
    PROCESSING
};

enum class RecordState {
    STOPPED = 0,
    RECORDING,
    PROCESSING
};

class StateView {
    public:
        static StateView *getInstance() {
            static StateView instance;
            return &instance;
        }
        StateView(const StateView &) = delete;
        StateView &operator=(const StateView &) = delete;

        // View Properties
        inline static LEDState LED_STATE = LEDState::OFF;
        inline static RecordState RECORD_STATE = RecordState::STOPPED;

    private:
        StateView() = default;
        ~StateView() = default;
};

#endif // STATE_VIEW_HPP_