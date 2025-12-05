#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "RLogger.hpp"
#include "DBusSender.hpp"
#include "BluezDBus.hpp"
#include "BluetoothAgent.hpp"
#include <thread>

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus,
     std::shared_ptr<BluetoothAgent> agent) : ThreadBase("MainWorker"), 
    eventQueue_(eventQueue), bluezDBus_(bluezDBus), agent_(agent) {
}

void MainWorker::threadFunction() {
    R_LOG(INFO, "MainWorker Thread function started");

    while (runningFlag_) {
        if (eventQueue_->hasEvent()) {
            std::shared_ptr<Event> event = eventQueue_->popEvent();
            if (event != nullptr) {
                processEvent(event);
            }
        } else {
            // Wait for new events with a timeout
            eventQueue_->waitForEvent((uint32_t)INTERNAL_EVENTQUEUE_TIMEOUT_MS);
        }
    }

    R_LOG(INFO, "MainWorker Thread function exiting");
}

void MainWorker::processEvent(const std::shared_ptr<Event> event) {
    if (event == nullptr) {
        return;
    }

    // Process the event based on its type
    switch (event->getEventTypeId()) {
        case EventTypeID::INITIALIZE_BLUETOOTH:
            R_LOG(INFO, "Processing INITIALIZE_BLUETOOTH event");
            processInitializeBluetoothEvent();
            break;
        case EventTypeID::START_SCAN_BTDEVICE:
            R_LOG(INFO, "Processing START_SCAN_BTDEVICE event");
            processStartScanBTDeviceEvent();
            break;
        case EventTypeID::STOP_SCAN_BTDEVICE:
            R_LOG(INFO, "Processing STOP_SCAN_BTDEVICE event");
            processStopScanBTDeviceEvent();
            break;
        case EventTypeID::BLUETOOTH_POWER_ON:
            R_LOG(INFO, "Processing BLUETOOTH_POWER_ON event");
            processBluetoothPowerOnEvent();
            break;
        case EventTypeID::BLUETOOTH_POWER_OFF:
            R_LOG(INFO, "Processing BLUETOOTH_POWER_OFF event");
            processBluetoothPowerOffEvent();
            break;
        case EventTypeID::PAIR_BTDEVICE:
            R_LOG(INFO, "Processing PAIR_BTDEVICE event");
            processPairBTDeviceEvent(event->getPayload());
            break;
        case EventTypeID::UNPAIR_BTDEVICE:
            R_LOG(INFO, "Processing UNPAIR_BTDEVICE event");
            processUnpairBTDeviceEvent(event->getPayload());
            break;
        case EventTypeID::CONNECT_BTDEVICE:
            R_LOG(INFO, "Processing CONNECT_BTDEVICE event");
            procsesConnectBTDeviceEvent(event->getPayload());
            break;
        case EventTypeID::DISCONNECT_BTDEVICE:
            R_LOG(INFO, "Processing DISCONNECT_BTDEVICE event");
            processDisconnectBTDeviceEvent(event->getPayload());
            break;
        case EventTypeID::REJECT_REQUEST_CONFIRMATION:
            R_LOG(INFO, "Processing REJECT_REQUEST_CONFIRMATION event");
            processRejectRequestConfirmationEvent(event->getPayload());
            break;
        case EventTypeID::ACCEPT_REQUEST_CONFIRMATION:
            R_LOG(INFO, "Processing ACCEPT_REQUEST_CONFIRMATION event");
            processAcceptRequestConfirmationEvent(event->getPayload());
            break;
        default:
            R_LOG(WARN, "MainWorker received unknown EventTypeID");
            break;
    }
}

void MainWorker::processInitializeBluetoothEvent() {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    bluezDBus_->initializeAdapter();
}

void MainWorker::processStartScanBTDeviceEvent() {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    bluezDBus_->startDiscovery();
}

void MainWorker::processStopScanBTDeviceEvent() {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    bluezDBus_->stopDiscovery();
}

void MainWorker::processBluetoothPowerOnEvent() {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    bluezDBus_->powerOnAdapter();
}

void MainWorker::processBluetoothPowerOffEvent() {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    bluezDBus_->powerOffAdapter();
}

void MainWorker::processPairBTDeviceEvent(std::shared_ptr<Payload> payload) {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "PAIR_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    bluezDBus_->pairDevice(btPayload->getAddress());
}

void MainWorker::processUnpairBTDeviceEvent(std::shared_ptr<Payload> payload) {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "UNPAIR_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    bluezDBus_->unpairDevice(btPayload->getAddress());
}

void MainWorker::procsesConnectBTDeviceEvent(std::shared_ptr<Payload> payload) {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "CONNECT_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    bluezDBus_->connectDevice(btPayload->getAddress());
}

void MainWorker::processDisconnectBTDeviceEvent(std::shared_ptr<Payload> payload) {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "DISCONNECT_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    bluezDBus_->disconnectDevice(btPayload->getAddress());
}

void MainWorker::processRejectRequestConfirmationEvent(std::shared_ptr<Payload> payload) {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "REJECT_REQUEST_CONFIRMATION payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    agent_->confirmRequest(btPayload->getAddress(), false);
}

void MainWorker::processAcceptRequestConfirmationEvent(std::shared_ptr<Payload> payload) {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "ACCEPT_REQUEST_CONFIRMATION payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    agent_->confirmRequest(btPayload->getAddress(), true);
}