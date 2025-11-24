#include "RecordWorker.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "AlsaHelper.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "DBusData.hpp"

RecordWorker::RecordWorker(std::shared_ptr<EventQueue> eventQueue) : ThreadBase("RecordWorker"),
	 eventQueue_(eventQueue), alsaHelper_(std::make_unique<AlsaHelper>()), state_(State::IDLE), cancelRequested_(false) {}

RecordWorker::~RecordWorker() {}

void RecordWorker::stop() {
    ThreadBase::stop();
    state_.store(State::IDLE); // Change state to allow thread to exit if in capture loop
    cv_.notify_one(); // Wake up thread if it's waiting
}

void RecordWorker::startRecording() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (state_ == State::IDLE) {
        cancelRequested_ = false; // Reset cancel flag for new session
        state_ = State::RECORDING;
        cv_.notify_one();
    } else {
        R_LOG(WARN, "Record worker is already recording. Ignoring start request.");
        DBusDataInfo info;
        info[DBUS_DATA_MESSAGE] = "Recording is already in progress.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, false, info);
    }
}

void RecordWorker::stopRecording() {
    if (state_ == State::RECORDING) {
        state_ = State::IDLE;
        // The capture loop in threadFunction will see the state change and stop.
        R_LOG(INFO, "Recording stop requested.");
    } else {
        R_LOG(WARN, "No active recording to stop. Ignoring stop request.");
        DBusDataInfo info;
        info[DBUS_DATA_MESSAGE] = "No recording is in progress.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, info);
    }
}

void RecordWorker::cancelRecording() {
    if (state_ == State::RECORDING) {
        cancelRequested_ = true;
        state_ = State::IDLE;
        // The capture loop in threadFunction will see the state change and stop.
        R_LOG(INFO, "Recording cancel requested.");
    } else {
        R_LOG(WARN, "No active recording to cancel. Ignoring cancel request.");
        DBusDataInfo info;
        info[DBUS_DATA_MESSAGE] = "No recording is in progress.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::CANCEL_RECORD_NOTI, false, info);
    }
}

void RecordWorker::threadFunction() {
	R_LOG(INFO, "RecordWorker thread started, waiting for recording tasks.");

    while (runningFlag_) {
        // Wait until startRecording() is called
		// Wait until state is RECORDING or runningFlag_ is false, so we can exit wait
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{ return state_ == State::RECORDING || !runningFlag_; });
        }

		// Check runningFlag_ again after waking up
        if (!runningFlag_) {
            break; // Exit if shutdown was requested
        }

        // -- Start Recording Session --
        R_LOG(INFO, "RecordWorker woken up, starting recording session.");
		alsaHelper_->clearAudioBuffer();

        if (!alsaHelper_->initAlsa()) {
            R_LOG(ERROR, "Failed to initialize ALSA, aborting recording session.");
            alsaHelper_->cleanupAlsa();
            DBusDataInfo info;
            info[DBUS_DATA_MESSAGE] = "ALSA initialization failed";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, false, info);
            state_ = State::IDLE; // Reset state
            continue; // Go back to waiting
        }

		// Notify that recording has started
        DBusDataInfo startInfo;
        startInfo[DBUS_DATA_MESSAGE] = "Recording started";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, true, startInfo);
		const auto startTime = std::chrono::steady_clock::now();
        const auto maxDuration = std::chrono::seconds(CONFIG_INSTANCE()->getMaxRecordDurationSec());
        bool durationExceeded = false;
        bool isCaptureError = false;
		
        while (runningFlag_ && state_ == State::RECORDING) {
			if (std::chrono::steady_clock::now() - startTime > maxDuration) {
                R_LOG(WARN, "Maximum recording duration reached. Stopping automatically.");
                durationExceeded = true;
                break;
            }

            if (!alsaHelper_->captureOnce()) {
                R_LOG(ERROR, "captureOnce failed, breaking capture loop");
                isCaptureError = true;
                break;
            }
        }

        // -- End Recording Session --
        if (isCaptureError) {
            R_LOG(WARN, "Recording stopped due to capture error. No WAV file will be saved.");
            DBusDataInfo info;
            info[DBUS_DATA_MESSAGE] = "Recording stopped due to capture error.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, info);
        } else if (cancelRequested_) {
            R_LOG(INFO, "Recording canceled. No WAV file will be saved.");
            DBusDataInfo info;
            info[DBUS_DATA_MESSAGE] = "Recording canceled by user.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::CANCEL_RECORD_NOTI, true, info);
        } else {
			if(durationExceeded) {
				R_LOG(INFO, "Recording stopped after reaching maximum duration. Saving WAV file...");
			} else {
				R_LOG(INFO, "Recording stopped by client. Saving WAV file...");
			}

			if (alsaHelper_->isAudioBufferEmpty()) {
				R_LOG(WARN, "Recording stopped but no audio was captured. No file saved.");
                DBusDataInfo info;
                info[DBUS_DATA_MESSAGE] = "No audio data captured.";
				DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, info);
			} else if (!alsaHelper_->saveWavFile()) {
                DBusDataInfo info;
                info[DBUS_DATA_MESSAGE] = "Failed to save WAV file.";
                DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, info);
                R_LOG(ERROR, "Failed to save WAV file");
            } else {
                DBusDataInfo info;
                info[DBUS_DATA_MESSAGE] = "WAV file saved: " + alsaHelper_->getOutputFilePath();
                DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, true, info);
                R_LOG(INFO, "WAV file saved successfully: %s", alsaHelper_->getOutputFilePath().c_str());

			    // Push event for further processing
                const auto endTime = std::chrono::steady_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
                const int durationSec = duration.count();

				std::shared_ptr<Payload> payload = std::make_shared<WavPayload>(alsaHelper_->getOutputFilePath(), durationSec);
				std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::FILTER_WAV_FILE, payload);
				eventQueue_->pushEvent(event);
            }
        }

        alsaHelper_->cleanupAlsa();
        state_ = State::IDLE; // Ensure state is IDLE before waiting again
        R_LOG(INFO, "Recording session finished. Returning to idle state.");
    }

	R_LOG(INFO, "RecordWorker thread finished");
}