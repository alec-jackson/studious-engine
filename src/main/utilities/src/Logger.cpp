#include <iostream>
#include <string>
#include <chrono>
#include <map>
#include <Logger.hpp>
using std::string;

namespace Log {
    std::map<LogLevels,string> levelMap = {
        {LogLevels::INFO,"INFO"},
        {LogLevels::WARN,"WARN"},
        {LogLevels::ERROR,"ERROR"}
    };

    LogLevels logLevel = LogLevels::INFO;

    std::time_t getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::time_point_cast<std::chrono::seconds>(now);
        return epoch.time_since_epoch().count();
    }

    void printLogLine(string text, const char *f, int l, LogLevels logLevel) {
        string level = levelMap[logLevel];
        std::cout << level << "\t| " << getCurrentTimestamp() << "\t| " << text << "\t| " << "LINE: " << l << " FILE: " << f << std::endl;
    }

    void INFO_LOG(string text, const char *f, int l) {
        if (logLevel < LogLevels::INFO) {
            return;
        }
        printLogLine(text, f, l, LogLevels::INFO);
    }

    void WARN_LOG(string text, const char *f, int l) {
        if (logLevel < LogLevels::WARN) {
            return;
        }
        printLogLine(text, f, l, LogLevels::WARN);
    }

    void ERROR_LOG(string text, const char *f, int l) {
        if (logLevel < LogLevels::ERROR) {
            return;
        }
        printLogLine(text, f, l, LogLevels::ERROR);
    }
};
