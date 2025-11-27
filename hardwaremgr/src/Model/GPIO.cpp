#include "GPIO.hpp"
#include "Config.hpp"
#include "RLogger.hpp"
#include <fstream>
#include <dirent.h> // for directory operations
#include <iostream>

GPIO::GPIO() {
    findSensorFile();
}

void GPIO::findSensorFile() {
    DIR *dir;
    struct dirent *ent;
    std::string devicesPath = CONFIG_INSTANCE()->getW1DevicesPath();
    std::string sensorPrefix = CONFIG_INSTANCE()->getW1SensorPrefix();

    if ((dir = opendir(devicesPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string entryName = ent->d_name;
            if (entryName.rfind(sensorPrefix, 0) == 0) {
                sensorFile_ = devicesPath + entryName + "/w1_slave";
                R_LOG(INFO, "Found DS18B20 sensor file: %s", sensorFile_.c_str());
                closedir(dir);
                return;
            }
        }
        closedir(dir);
    }
    R_LOG(ERROR, "Could not find DS18B20 sensor in %s. Please check connection and dtoverlay in /boot/config.txt", devicesPath.c_str());
}

bool GPIO::readTemperatureSensor(float& temperature) {
    if (sensorFile_.empty()) {
        R_LOG(ERROR, "Sensor file not found. Cannot read temperature.");
        return false;
    }

    std::ifstream sensorStream(sensorFile_);
    if (!sensorStream.is_open()) {
        R_LOG(ERROR, "Failed to open sensor file: %s", sensorFile_.c_str());
        return false;
    }

    std::string line;
    // First line: "xx xx xx xx xx xx xx xx xx : crc=xx YES/NO"
    std::getline(sensorStream, line);
    if (line.find("YES") == std::string::npos) {
        R_LOG(ERROR, "CRC check failed for temperature sensor.");
        return false;
    }

    // Second line: "xx xx xx xx xx xx xx xx xx t=xxxxx"
    std::getline(sensorStream, line);
    size_t tempPos = line.find("t=");
    if (tempPos == std::string::npos) {
        R_LOG(ERROR, "Failed to find temperature value in sensor output.");
        return false;
    }

    try {
        int temp_mC = std::stoi(line.substr(tempPos + 2));
        temperature = static_cast<float>(temp_mC) / 1000.0f;
    } catch (const std::invalid_argument& ia) {
        R_LOG(ERROR, "Invalid temperature format: %s", ia.what());
        return false;
    } catch (const std::out_of_range& oor) {
        R_LOG(ERROR, "Temperature value out of range: %s", oor.what());
        return false;
    } catch (...) {
        R_LOG(ERROR, "Unknown error occurred while parsing temperature.");
        return false;
    }   

    return true;
}