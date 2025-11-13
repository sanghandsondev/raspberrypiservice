#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "IConfig.hpp"
#include <alsa/asoundlib.h>

#define CONFIG_INSTANCE() Config::getInstance()

class Config {
    public:
        static Config *getInstance() {
            static Config instance;
            return &instance;
        }
        Config(const Config &) = delete;
        Config &operator=(const Config &) = delete;

        const std::string &getBinaryPath() const { return RECORDMGR_BINARYPATH;}
        const std::string &getServiceName() const { return RECORDMGR_SERVICE_NAME;}
        const std::string &getObjectPath() const { return RECORDMGR_OBJECT_PATH;}
        const std::string &getInterfaceName() const { return RECORDMGR_INTERFACE_NAME;}
        const std::string &getSignalName() const { return RECORDMGR_SIGNAL_NAME;}

        unsigned int getSampleRate() const { return SAMPLE_RATE; }
        snd_pcm_uframes_t getFramesPerPeriod() const { return FRAMES_PER_PERIOD; }
        const std::string &getMicrophoneDevice() const { return MICROPHONE_DEVICE; }
        const std::string &getWavOutputDir() const { return WAV_OUTPUT_DIR; }
        unsigned int getMaxRecordDurationSec() const { return MAX_RECORD_DURATION_SEC; }

    private:
        Config() = default;
        ~Config() = default;

        // C++ 17 inline static member
        inline static const std::string RECORDMGR_BINARYPATH = "/usr/local/bin/recordmanager";      
        inline static const std::string RECORDMGR_SERVICE_NAME = "com.example.recordmanager";
        inline static const std::string RECORDMGR_OBJECT_PATH = "/com/example/recordmanager";
        inline static const std::string RECORDMGR_INTERFACE_NAME = "com.example.recordmanager.interface";
        inline static const std::string RECORDMGR_SIGNAL_NAME = "RecordSignal";

        inline static const unsigned int SAMPLE_RATE = 16000;
        inline static const snd_pcm_uframes_t FRAMES_PER_PERIOD = 1024;
        inline static const unsigned int MAX_RECORD_DURATION_SEC = 300; // 5 minutes
#ifdef RASPBERRY_PI
        inline static const std::string MICROPHONE_DEVICE = "plughw:1,0"; // card 1 device 0 -> hw:1,0 (use plughw for convenience)
#else
        inline static const std::string MICROPHONE_DEVICE = "default"; // For laptops or other systems
#endif
        inline static const std::string WAV_OUTPUT_DIR = "/tmp";
};

#endif // CONFIG_HPP_