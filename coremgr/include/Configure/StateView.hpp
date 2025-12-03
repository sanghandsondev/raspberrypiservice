#ifndef STATE_VIEW_HPP_
#define STATE_VIEW_HPP_

#include "Define.hpp"

#define STATE_VIEW_INSTANCE() StateView::getInstance()

enum class RecordState {
    STOPPED = 0,
    RECORDING,
    PROCESSING
};

enum class ScanningBTDeviceState {
    IDLE = 0,
    SCANNING
};

enum class BluetoothPowerState {
    OFF = 0,
    ON,
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
        inline static RecordState RECORD_STATE = RecordState::STOPPED;
        inline static int CURRENT_TEMPERATURE = 0;
        inline static ScanningBTDeviceState SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::IDLE;
        inline static BluetoothPowerState BLUETOOTH_POWER_STATE = BluetoothPowerState::OFF;

    private:
        StateView() = default;
        ~StateView() = default;
};

#endif // STATE_VIEW_HPP_