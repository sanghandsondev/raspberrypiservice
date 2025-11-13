#ifndef ALSA_HELPER_HPP_
#define ALSA_HELPER_HPP_

#include <alsa/asoundlib.h>
#include <vector>
#include <string>
#include <cstdint>

class AlsaHelper {
public:
    AlsaHelper();
    ~AlsaHelper();

    bool initAlsa();
    void cleanupAlsa();
    bool captureOnce();
    bool saveWavFile();

    void clearAudioBuffer();
    bool isAudioBufferEmpty() const;
    const std::string& getOutputFilePath() const;

private:
    std::string findCaptureDevice();

    snd_pcm_t* pcmHandle_;
    std::vector<int16_t> audioBuffer_;
    std::string outputFilePath_;
};

#endif // ALSA_HELPER_HPP_
