#ifndef AUDIO_FILTER_HPP_
#define AUDIO_FILTER_HPP_

#include <string>

class AudioFilter {
    public:
        explicit AudioFilter() = default;
        ~AudioFilter() = default;

        // Tham chieu den file WAV can loc
        bool applyFilter(const std::string& wavFilePath);
        std::string getFilteredFilePath() const { return filteredFilePath_; }

    private:
        std::string filteredFilePath_ = "";
};

#endif // AUDIO_FILTER_HPP_