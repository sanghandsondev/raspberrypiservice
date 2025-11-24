#ifndef SCHEMA_HPP_
#define SCHEMA_HPP_

#include <string>

// Schema representation
struct AudioRecord {
    int id;
    std::string filePath;
    int durationSec;
    // std::string createdAt;
};

#endif // SCHEMA_HPP_