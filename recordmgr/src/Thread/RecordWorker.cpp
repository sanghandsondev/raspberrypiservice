#include "RecordWorker.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>
#include "AlsaHelper.hpp"

RecordWorker::RecordWorker(std::shared_ptr<EventQueue> eventQueue) : ThreadBase("RecordWorker"),
	 eventQueue_(eventQueue), alsaHelper_(std::make_unique<AlsaHelper>()), state_(State::IDLE) {}

RecordWorker::~RecordWorker() {}

void RecordWorker::stop() {
    ThreadBase::stop();
    state_.store(State::IDLE); // Change state to allow thread to exit if in capture loop
    cv_.notify_one(); // Wake up thread if it's waiting
}

void RecordWorker::startRecording() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (state_ == State::IDLE) {
        state_ = State::RECORDING;
        cv_.notify_one();
    } else {
        R_LOG(WARN, "Record worker is already recording. Ignoring start request.");
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, false, "Recording is already in progress.");
    }
}

void RecordWorker::stopRecording() {
    if (state_ == State::RECORDING) {
        state_ = State::IDLE;
        // The capture loop in threadFunction will see the state change and stop.
        R_LOG(INFO, "Recording stop requested.");
    } else {
        R_LOG(WARN, "No active recording to stop. Ignoring stop request.");
        DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, "No recording is in progress.");
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
            DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, false, "ALSA initialization failed");
            state_ = State::IDLE; // Reset state
            continue; // Go back to waiting
        }

		// Notify that recording has started
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, true, "Recording started");
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
            DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, "Recording stopped due to capture error.");
        } else {
			if(durationExceeded) {
				R_LOG(INFO, "Recording stopped after reaching maximum duration. Saving WAV file...");
			} else {
				R_LOG(INFO, "Recording stopped by client. Saving WAV file...");
			}
			if (alsaHelper_->isAudioBufferEmpty()) {
				R_LOG(WARN, "Recording stopped but no audio was captured. No file saved.");
				DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, "No audio data captured.");
			} else if (!alsaHelper_->saveWavFile()) {
                DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, "Failed to save WAV file.");
                R_LOG(ERROR, "Failed to save WAV file");
            } else {
                DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, true, "WAV file saved: " + alsaHelper_->getOutputFilePath());
                R_LOG(INFO, "WAV file saved successfully: %s", alsaHelper_->getOutputFilePath().c_str());
            }
        }

        alsaHelper_->cleanupAlsa();
        state_ = State::IDLE; // Ensure state is IDLE before waiting again
        R_LOG(INFO, "Recording session finished. Returning to idle state.");
    }

	R_LOG(INFO, "RecordWorker thread finished");
}