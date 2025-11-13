#include "Util/AlsaHelper.hpp"
#include "Config.hpp"
#include "RLogger.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

AlsaHelper::AlsaHelper() : pcmHandle_(nullptr) {}

AlsaHelper::~AlsaHelper() {
    cleanupAlsa();
}

std::string AlsaHelper::findCaptureDevice() {
    int card = -1;
    int err;

    // Iterate over sound cards
    while (snd_card_next(&card) >= 0 && card >= 0) {
        snd_ctl_t *ctl_handle;
        char card_name[32];
        sprintf(card_name, "hw:%d", card);

        if ((err = snd_ctl_open(&ctl_handle, card_name, 0)) < 0) {
            R_LOG(WARN, "Cannot open control for card %d: %s", card, snd_strerror(err));
            continue;
        }

        int dev = -1;
        while (snd_ctl_pcm_next_device(ctl_handle, &dev) >= 0 && dev >= 0) {
            snd_pcm_info_t *pcm_info;
            snd_pcm_info_alloca(&pcm_info);
            snd_pcm_info_set_device(pcm_info, dev);
            snd_pcm_info_set_subdevice(pcm_info, 0);
            snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_CAPTURE);

            if ((err = snd_ctl_pcm_info(ctl_handle, pcm_info)) >= 0) {
                // Found a capture device
                // std::string device_name = "hw:" + std::to_string(card) + "," + std::to_string(dev);
                R_LOG(INFO, "Found capture device: %s on card %d", snd_pcm_info_get_name(pcm_info), card);
                snd_ctl_close(ctl_handle);
                // Use plughw for better compatibility
                return "plughw:" + std::to_string(card) + "," + std::to_string(dev);
            }
        }
        snd_ctl_close(ctl_handle);
    }

    R_LOG(WARN, "No capture device found. Falling back to default from config.");
    return CONFIG_INSTANCE()->getMicrophoneDevice();
}

bool AlsaHelper::initAlsa() {
	int err;
	snd_pcm_hw_params_t* hwParams = nullptr;
	const std::string device = findCaptureDevice();

	err = snd_pcm_open(&pcmHandle_, device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0) {
		R_LOG(ERROR, "snd_pcm_open(%s) failed: %s", device.c_str(), snd_strerror(err));
		pcmHandle_ = nullptr;
		return false;
	}

	snd_pcm_hw_params_malloc(&hwParams);
	if ((err = snd_pcm_hw_params_any(pcmHandle_, hwParams)) < 0) {
		R_LOG(ERROR, "snd_pcm_hw_params_any failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params_set_access(pcmHandle_, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		R_LOG(ERROR, "snd_pcm_hw_params_set_access failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params_set_format(pcmHandle_, hwParams, SND_PCM_FORMAT_S16_LE)) < 0) {
		R_LOG(ERROR, "snd_pcm_hw_params_set_format failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	unsigned int rate = CONFIG_INSTANCE()->getSampleRate();
	if ((err = snd_pcm_hw_params_set_rate_near(pcmHandle_, hwParams, &rate, nullptr)) < 0) {
		R_LOG(ERROR, "snd_pcm_hw_params_set_rate_near failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params_set_channels(pcmHandle_, hwParams, 1)) < 0) {
		R_LOG(ERROR, "snd_pcm_hw_params_set_channels failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	if ((err = snd_pcm_hw_params(pcmHandle_, hwParams)) < 0) {
		R_LOG(ERROR, "snd_pcm_hw_params failed: %s", snd_strerror(err));
		snd_pcm_hw_params_free(hwParams);
		return false;
	}

	snd_pcm_hw_params_free(hwParams);

	if ((err = snd_pcm_prepare(pcmHandle_)) < 0) {
		R_LOG(ERROR, "snd_pcm_prepare failed: %s", snd_strerror(err));
		return false;
	}

	R_LOG(INFO, "ALSA init OK (device=%s, rate=%u)", device.c_str(), rate);
	return true;
}

void AlsaHelper::cleanupAlsa() {
	if (pcmHandle_) {
		snd_pcm_drain(pcmHandle_);
		snd_pcm_close(pcmHandle_);
		pcmHandle_ = nullptr;
	}
}

bool AlsaHelper::captureOnce() {
	if (!pcmHandle_) return false;

	const snd_pcm_uframes_t frames = CONFIG_INSTANCE()->getFramesPerPeriod();
	std::vector<int16_t> buffer(frames);

	snd_pcm_sframes_t r = snd_pcm_readi(pcmHandle_, buffer.data(), frames);
	if (r == -EPIPE) {
		R_LOG(WARN, "ALSA overrun occurred");
		snd_pcm_prepare(pcmHandle_);
		return true; // continue capturing
	} else if (r < 0) {
		R_LOG(ERROR, "snd_pcm_readi error: %s", snd_strerror(static_cast<int>(r)));
		int rc = snd_pcm_recover(pcmHandle_, r, 0);
		if (rc < 0) {
			R_LOG(ERROR, "snd_pcm_recover failed: %s", snd_strerror(rc));
			return false;
		}
		return true;
	}

	// append read frames
	audioBuffer_.insert(audioBuffer_.end(), buffer.begin(), buffer.begin() + r);
	return true;
}

bool AlsaHelper::saveWavFile() {
	if (audioBuffer_.empty()) {
		R_LOG(WARN, "No audio captured, skipping WAV save");
		return false;
	}

	// Build timestamped filename
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::localtime(&t);
	std::ostringstream ss;
	std::string wavDir = CONFIG_INSTANCE()->getWavOutputDir();
	ss << wavDir << "/record_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
	outputFilePath_ = ss.str();

	std::ofstream out(outputFilePath_, std::ios::binary);
	if (!out.is_open()) {
		R_LOG(ERROR, "Failed to open output file %s", outputFilePath_.c_str());
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

	R_LOG(INFO, "Saved WAV: %s (frames=%zu, bytes=%u)", outputFilePath_.c_str(), audioBuffer_.size(), subchunk2Size);
	return true;
}

void AlsaHelper::clearAudioBuffer() {
    audioBuffer_.clear();
}

bool AlsaHelper::isAudioBufferEmpty() const {
    return audioBuffer_.empty();
}

const std::string& AlsaHelper::getOutputFilePath() const {
    return outputFilePath_;
}
