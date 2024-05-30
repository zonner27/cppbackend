#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>
#include <iostream>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t_c), "%Y_%m_%d");
        return ss.str();
    }

    void open_file() {
        current_time = GetFileTimeStamp();
        log_file_.open("/var/log/sample_log_"s + current_time + ".log"s, std::ios::app);
    }

    //Logger() = default;
    Logger() {
        open_file();
    }
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    template<typename T>
    void LogSingle(const T& arg) {
        log_file_ << arg;
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_ << GetTimeStamp() << ": "sv;
        (LogSingle(args), ...);
        log_file_ << std::endl;
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть 
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard<std::mutex> lock(mutex_);
        manual_ts_ = ts - std::chrono::hours(3);
        if (GetFileTimeStamp() != current_time) {
            log_file_.close();
            open_file();
        }
    }

private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::mutex mutex_;
    std::ofstream log_file_;
    std::string current_time;
};



