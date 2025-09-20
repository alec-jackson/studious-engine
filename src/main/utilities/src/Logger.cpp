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

    void baseLogPrint(string text) {
        std::cout << text << std::endl;
    }

    template<typename T, typename ...Args>
    void logPrint(string text, T item, Args... remArgs) {
        auto pos = text.find("%i");
        if (std::string::npos == pos) {
            baseLogPrint(text);
            return;
        }
        std::cout << text.substr(0, pos) << item;
        logPrint(text.substr(pos + 2, text.length()), remArgs...);
    }

    std::time_t getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::time_point_cast<std::chrono::seconds>(now);
        return epoch.time_since_epoch().count();
    }

    void printMetaData(LogLevels logLevel) {
        string level = levelMap[logLevel];
        std::cout << level << "\t| " << getCurrentTimestamp() << "\t| ";
    }

    template<typename T, typename ...Args>
    void printLogLine(string text, LogLevels logLevel, T item, Args... remArgs) {
        printMetaData(logLevel);
        logPrint(text, remArgs...);
    }

    void printLogLine(string text, LogLevels logLevel) {
        printMetaData(logLevel);
        std::cout << text << std::endl;
    }

    template<typename T, typename ...Args>
    void INFO(string text, T item, Args... remArgs) {
        if (logLevel < LogLevels::INFO) {
            return;
        }
        printLogLine(text, LogLevels::INFO, remArgs...);
    }

    void INFO(string text) {
        if (logLevel < LogLevels::INFO) {
            return;
        }
        printLogLine(text, LogLevels::INFO);
    }

    template<typename T, typename ...Args>
    void WARN(string text, T item, Args... remArgs) {
        if (logLevel < LogLevels::WARN) {
            return;
        }
        printLogLine(text, LogLevels::WARN, remArgs...);
    }

    void WARN(string text) {
        if (logLevel < LogLevels::WARN) {
            return;
        }
        printLogLine(text, LogLevels::WARN);
    }

    template<typename T, typename ...Args>
    void ERROR(string text, T item, Args... remArgs) {
        if (logLevel < LogLevels::ERROR) {
            return;
        }
        printLogLine(text, LogLevels::ERROR, remArgs...);
    }

    void ERROR(string text) {
        if (logLevel < LogLevels::ERROR) {
            return;
        }
        printLogLine(text, LogLevels::ERROR);
    }
};
