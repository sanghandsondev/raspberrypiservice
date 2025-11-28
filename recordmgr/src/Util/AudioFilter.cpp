#include "AudioFilter.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include <filesystem>
#include <cstdlib> // For system()

namespace fs = std::filesystem;

bool AudioFilter::applyFilter(const std::string& wavFilePath){
    R_LOG(INFO, "Applying audio filter to file: %s", wavFilePath.c_str());
    // Source: /tmp/record_...
    // Success dest: /var/local/recordmanager/audio/filtered_record_...
    // Fail dest:    /var/local/recordmanager/audio/record_...

    try {
        fs::path sourcePath(wavFilePath);
        if (!fs::exists(sourcePath)) {
            R_LOG(ERROR, "Source file does not exist: %s", wavFilePath.c_str());
            return false;
        }
        std::string fileName = sourcePath.filename().string();

        // Tạo đường dẫn cho file tạm đã được lọc
        fs::path tempFilteredPath = fs::temp_directory_path() / ("filtered_" + fileName);

        // 1. Thực hiện lọc âm thanh bằng sox, lưu vào file tạm
        // Cải tiến lệnh sox để giảm nhiễu tốt hơn bằng cách sử dụng noise profile.
        // Bước 1: Tạo một "noise profile" từ 0.4 giây đầu của file audio.
        std::string noiseProfilePath = tempFilteredPath.string() + ".prof";
        std::string noiseprof_command = "sox \"" + wavFilePath + "\" -n trim 0 0.4 noiseprof \"" + noiseProfilePath + "\"";
        
        R_LOG(INFO, "Creating noise profile: %s", noiseprof_command.c_str());
        int noiseprof_result = std::system(noiseprof_command.c_str());

        std::string command;
        if (noiseprof_result == 0) {
            // Bước 2: Áp dụng bộ lọc highpass, lowpass và noisered sử dụng profile đã tạo.
            // Mức giảm nhiễu 0.21 là một giá trị khởi đầu tốt, có thể điều chỉnh.
            command = "sox \"" + wavFilePath + "\" \"" + tempFilteredPath.string() + "\" highpass 100 lowpass 3000 noisered \"" + noiseProfilePath + "\" 0.21";
        } else {
            R_LOG(WARN, "Failed to create noise profile, falling back to simpler filter.");
            // Nếu không tạo được profile, dùng lại lệnh cũ.
            command = "sox \"" + wavFilePath + "\" \"" + tempFilteredPath.string() + "\" highpass 100 lowpass 3000";
        }

        R_LOG(INFO, "Executing audio filter command: %s", command.c_str());
        int result = std::system(command.c_str());

        // Dọn dẹp file noise profile
        if (fs::exists(noiseProfilePath)) {
            fs::remove(noiseProfilePath);
        }

        bool isFilterSuccess = (result == 0);

        // 2. Tạo đường dẫn đích cuối cùng
        std::string outputDir = CONFIG_INSTANCE()->getFilteredAudioDir();
        fs::create_directories(outputDir); // Đảm bảo thư mục đích tồn tại

        if (isFilterSuccess) {
            R_LOG(INFO, "Audio filtering successful.");
            // Đích là file đã lọc
            filteredFilePath_ = (fs::path(outputDir) / ("filtered_" + fileName)).string();
            // Dùng copy + remove thay vì rename (vì /tmp và /var/local có thể trên khác filesystem)
            fs::copy_file(tempFilteredPath, filteredFilePath_, fs::copy_options::overwrite_existing);
            fs::remove(tempFilteredPath);
            R_LOG(INFO, "Moved filtered file to: %s", filteredFilePath_.c_str());
        } else {
            R_LOG(WARN, "Audio filtering failed with exit code: %d. Using original file.", result);
            // Đích là file gốc
            filteredFilePath_ = (fs::path(outputDir) / fileName).string();
            // Sao chép file gốc vào vị trí cuối cùng
            fs::copy_file(sourcePath, filteredFilePath_, fs::copy_options::overwrite_existing);
            R_LOG(INFO, "Copied original file to: %s", filteredFilePath_.c_str());
            
            // Dọn dẹp file tạm nếu nó được tạo ra nhưng sox thất bại
            if (fs::exists(tempFilteredPath)) {
                fs::remove(tempFilteredPath);
            }
        }
        return true;

    } catch (const fs::filesystem_error& e) {
        R_LOG(ERROR, "Filesystem error during filtering: %s", e.what());
        return false;
    }
}