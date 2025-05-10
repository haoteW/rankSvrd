#pragma once
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

inline std::string _log_current_time() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto itt = system_clock::to_time_t(now);
    auto tm = *std::localtime(&itt);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%F %T");
    return oss.str();
}

#define LOG(level, msg) \
    std::cout << "[" << _log_current_time() << "] [" << level << "] [" << __FILE__ << ":" << __LINE__ << "] " << msg << std::endl

#define LOG_INFO(msg)  LOG("INFO", msg)
#define LOG_WARN(msg)  LOG("WARN", msg)
#define LOG_ERROR(msg) LOG("ERROR", msg)
#define LOG_DEBUG(msg) LOG("DEBUG", msg)
