#ifndef RECORD_WORKER_HPP_
#define RECORD_WORKER_HPP_

#include "ThreadBase.hpp"
#include <alsa/asoundlib.h>
#include <vector>
#include <string>

class RecordWorker : public ThreadBase {
    public:
        explicit RecordWorker();
        ~RecordWorker();

    private:
        void threadFunction() override;

        // ALSA handle
        snd_pcm_t* pcmHandle_;
        std::vector<int16_t> audioBuffer_;
        std::string outputFilePath_;

        // Configuration
        static const unsigned int SAMPLE_RATE;
        static const snd_pcm_uframes_t FRAMES_PER_PERIOD;

        // Helpers
        bool initAlsa();
        void cleanupAlsa();
        bool captureOnce();
        bool saveWavFile();
};

#endif // RECORD_WORKER_HPP_