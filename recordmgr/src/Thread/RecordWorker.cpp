#include "RecordWorker.hpp"
#include "DBusSender.hpp"
#include "RMLogger.hpp"
#include "Config.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>

RecordWorker::RecordWorker() : ThreadBase("RecordWorker"), pcmHandle_(nullptr) {}

RecordWorker::~RecordWorker() {
    cleanupAlsa();      // Ensure ALSA resources are freed
}

void RecordWorker::threadFunction() {
	RM_LOG(INFO, "RecordWorker thread started");
    
    // Clear previous buffer
	audioBuffer_.clear();

    // Initialize ALSA for audio capture
	if (!initAlsa()) {
		RM_LOG(ERROR, "Failed to initialize ALSA, exiting RecordWorker thread");
		cleanupAlsa();
        stop();
		join();
		DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, false, "ALSA initialization failed");
		return;
	}

    // Notify start recording via D-Bus
    DBUS_SENDER()->sendMessageNoti(DBusCommand::START_RECORD_NOTI, true, "Recording started");

	// Capture loop - uses runningFlag_ from ThreadBase
    bool isCaptureError = false;
	while (runningFlag_) {
		if (!captureOnce()) {
			RM_LOG(ERROR, "captureOnce failed, breaking capture loop");
            isCaptureError = true;
			break;
		}
	}

    if (isCaptureError) {
        RM_LOG(WARN, "Recording stopped due to capture error. No WAV file will be saved.");
		DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, "Recording stopped due to capture error. No WAV file will be saved.");
    } else {
        RM_LOG(INFO, "Recording stopped by client request");
        if (!saveWavFile()) {
			DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, false, "Failed to save WAV file.");
		    RM_LOG(ERROR, "Failed to save WAV file");
	    } else {
			DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_RECORD_NOTI, true, "WAV file saved: " + outputFilePath_);
			RM_LOG(INFO, "WAV file saved successfully: %s", outputFilePath_.c_str());
		}
    }

    // Cleanup ALSA resources
	cleanupAlsa();
	RM_LOG(INFO, "RecordWorker thread finished");
}

bool RecordWorker::initAlsa() {
	int err;
	snd_pcm_hw_params_t* hwParams = nullptr;
	const std::string device = CONFIG_INSTANCE()->getMicrophoneDevice(); // e.g. "plughw:1,0"

	err = snd_pcm_open(&pcmHandle_, device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0) {
		RM_LOG(ERROR, "snd_pcm_open(%s) failed: %s", device.c_str(), snd_strerror(err));
		pcmHandle_ = nullptr;
		return false;
	}

	snd_pcm_hw_params_malloc(&hwParams);
	if ((err = snd_pcm_hw_params_any(pcmHandle_, hwParams)) < 0) {
		RM_LOG(ERROR, "snd_pcm_hw_params_any failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params_set_access(pcmHandle_, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		RM_LOG(ERROR, "snd_pcm_hw_params_set_access failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params_set_format(pcmHandle_, hwParams, SND_PCM_FORMAT_S16_LE)) < 0) {
		RM_LOG(ERROR, "snd_pcm_hw_params_set_format failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	unsigned int rate = CONFIG_INSTANCE()->getSampleRate();
	if ((err = snd_pcm_hw_params_set_rate_near(pcmHandle_, hwParams, &rate, nullptr)) < 0) {
		RM_LOG(ERROR, "snd_pcm_hw_params_set_rate_near failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params_set_channels(pcmHandle_, hwParams, 1)) < 0) {
		RM_LOG(ERROR, "snd_pcm_hw_params_set_channels failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params(pcmHandle_, hwParams)) < 0) {
		RM_LOG(ERROR, "snd_pcm_hw_params failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	snd_pcm_hw_params_free(hwParams);

	if ((err = snd_pcm_prepare(pcmHandle_)) < 0) {
		RM_LOG(ERROR, "snd_pcm_prepare failed: %s", snd_strerror(err));
		return false;
	}

	RM_LOG(INFO, "ALSA init OK (device=%s, rate=%u)", device.c_str(), rate);
	return true;
}

void RecordWorker::cleanupAlsa() {
	if (pcmHandle_) {
		snd_pcm_drain(pcmHandle_);
		snd_pcm_close(pcmHandle_);
		pcmHandle_ = nullptr;
	}
}

bool RecordWorker::captureOnce() {
	if (!pcmHandle_) return false;

	const snd_pcm_uframes_t frames = CONFIG_INSTANCE()->getFramesPerPeriod();
	std::vector<int16_t> buffer(frames);

	snd_pcm_sframes_t r = snd_pcm_readi(pcmHandle_, buffer.data(), frames);
	if (r == -EPIPE) {
		RM_LOG(WARN, "ALSA overrun occurred");
		snd_pcm_prepare(pcmHandle_);
		return true; // continue capturing
	} else if (r < 0) {
		RM_LOG(ERROR, "snd_pcm_readi error: %s", snd_strerror(static_cast<int>(r)));
		int rc = snd_pcm_recover(pcmHandle_, r, 0);
		if (rc < 0) {
			RM_LOG(ERROR, "snd_pcm_recover failed: %s", snd_strerror(rc));
			return false;
		}
		return true;
	}

	// append read frames
	for (snd_pcm_sframes_t i = 0; i < r; ++i) audioBuffer_.push_back(buffer[i]);
	return true;
}

bool RecordWorker::saveWavFile() {
	if (audioBuffer_.empty()) {
		RM_LOG(WARN, "No audio captured, skipping WAV save");
		return false;
	}

	// Build timestamped filename under /tmp
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::localtime(&t);
	std::ostringstream ss;
	std::string warDir = CONFIG_INSTANCE()->getWavOutputDir();
	ss << warDir << "/record_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
	outputFilePath_ = ss.str();

	std::ofstream out(outputFilePath_, std::ios::binary);
	if (!out.is_open()) {
		RM_LOG(ERROR, "Failed to open output file %s", outputFilePath_.c_str());
		return false;
	}

	uint32_t subchunk2Size = static_cast<uint32_t>(audioBuffer_.size() * sizeof(int16_t));
	uint32_t chunkSize = 36 + subchunk2Size;
	uint32_t byteRate = CONFIG_INSTANCE()->getSampleRate() * 1 * 2;
	uint16_t blockAlign = 1 * 2;

	// RIFF header
	out.write("RIFF", 4);
	out.write(reinterpret_cast<const char*>(&chunkSize), 4);
	out.write("WAVE", 4);

	// fmt subchunk
	out.write("fmt ", 4);
	uint32_t subchunk1Size = 16;
	out.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
	uint16_t audioFormat = 1; // PCM
	out.write(reinterpret_cast<const char*>(&audioFormat), 2);
	uint16_t numChannels = 1;
	out.write(reinterpret_cast<const char*>(&numChannels), 2);
	uint32_t sampleRate = CONFIG_INSTANCE()->getSampleRate();;
	out.write(reinterpret_cast<const char*>(&sampleRate), 4);
	out.write(reinterpret_cast<const char*>(&byteRate), 4);
	out.write(reinterpret_cast<const char*>(&blockAlign), 2);
	uint16_t bitsPerSample = 16;
	out.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

	// data subchunk
	out.write("data", 4);
	out.write(reinterpret_cast<const char*>(&subchunk2Size), 4);

	// write samples
	out.write(reinterpret_cast<const char*>(audioBuffer_.data()), subchunk2Size);
	out.close();

	RM_LOG(INFO, "Saved WAV: %s (frames=%zu, bytes=%u)", outputFilePath_.c_str(), audioBuffer_.size(), subchunk2Size);
	return true;
}

