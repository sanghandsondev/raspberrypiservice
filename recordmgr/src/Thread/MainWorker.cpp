#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "RecordWorker.hpp"
#include "DBusSender.hpp"
#include "DBusData.hpp"
#include "RLogger.hpp"
#include "AudioFilter.hpp"
#include <thread>

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<RecordWorker> recordWorker) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue), recordWorker_(recordWorker) {
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
        case EventTypeID::START_RECORD:
            R_LOG(INFO, "Processing START_RECORD event");
            processStartRecordEvent();
            break;
        case EventTypeID::STOP_RECORD:
            R_LOG(INFO, "Processing STOP_RECORD event");
            processStopRecordEvent();
            break;
        case EventTypeID::CANCEL_RECORD:
            R_LOG(INFO, "Processing CANCEL_RECORD event");
            processCancelRecordEvent();
            break;
        case EventTypeID::FILTER_WAV_FILE:
            R_LOG(INFO, "Processing FILTER_WAV_FILE event");
            processFilterWavFileEvent(event->getPayload());
            break;
        
        default:
            R_LOG(WARN, "MainWorker received unknown EventTypeID");
            break;
    }
}

void MainWorker::processStartRecordEvent() { recordWorker_->startRecording();}

void MainWorker::processStopRecordEvent() { recordWorker_->stopRecording(); }

void MainWorker::processCancelRecordEvent() { recordWorker_->cancelRecording(); }

void MainWorker::processFilterWavFileEvent(std::shared_ptr<Payload> payload) {
    std::shared_ptr<WavPayload> wavPayload = std::dynamic_pointer_cast<WavPayload>(payload);
    if (!wavPayload) {
        R_LOG(ERROR, "Invalid payload for FILTER_WAV_FILE event");
        return;
    }
    std::string wavFilePath = wavPayload->getFilePath();
    int durationSec = wavPayload->getDurationSec();
    R_LOG(INFO, "Received WAV file for filtering: %s", wavFilePath.c_str());

    if(wavFilePath == "") {
        R_LOG(ERROR, "WAV file path is empty, cannot process");
        return;
    }

    std::thread([wavFilePath, durationSec]() {
        R_LOG(INFO, "Starting filtering process for WAV file: %s", wavFilePath.c_str());

        AudioFilter filter;
        bool ret = filter.applyFilter(wavFilePath);
        DBusDataInfo dataInfo;
        if (!ret) {
            // Failed to apply filter
            R_LOG(ERROR, "Failed to applying filter on WAV file: %s", wavFilePath.c_str());
            dataInfo.data[DBUS_DATA_MESSAGE] = "FAILED to save WAV file.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::FILTER_WAV_FILE_NOTI, false, dataInfo);
        } else {
            R_LOG(INFO, "Successfully applied to applying filter on WAV file: %s", wavFilePath.c_str());
            dataInfo.data[DBUS_DATA_WAV_FILE_PATH] = filter.getFilteredFilePath();
            dataInfo.data[DBUS_DATA_WAV_FILE_DURATION_SEC] = std::to_string(durationSec);
            DBUS_SENDER()->sendMessageNoti(DBusCommand::FILTER_WAV_FILE_NOTI, true, dataInfo);
        }

        R_LOG(INFO, "Completed filtering process for WAV file: %s", wavFilePath.c_str());
    }).detach();
}