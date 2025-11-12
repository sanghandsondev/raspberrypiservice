#ifndef RECORD_WORKER_HPP_
#define RECORD_WORKER_HPP_

#include "ThreadBase.hpp"
#include <alsa/asoundlib.h>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>

class RecordWorker : public ThreadBase {
    public:
        explicit RecordWorker();
        ~RecordWorker();

        void startRecording();
        void stopRecording();
        void stop() override;

    private:
        enum class State {
            IDLE,
            RECORDING
        };

        void threadFunction() override;

        // ALSA handle
        snd_pcm_t* pcmHandle_;
        std::vector<int16_t> audioBuffer_;
        std::string outputFilePath_;

        // Thread control
        std::mutex mtx_;
        std::condition_variable cv_;
        std::atomic<State> state_;

        // Helpers
        bool initAlsa();
        void cleanupAlsa();
        bool captureOnce();
        bool saveWavFile();
};

#endif // RECORD_WORKER_HPP_