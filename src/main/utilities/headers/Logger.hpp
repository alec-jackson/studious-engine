#include <iostream>
#include <string>

#define INFO(x) INFO_LOG(x, __FILE__, __LINE__)
#define WARN(x) WARN_LOG(x, __FILE__, __LINE__)
#define ERROR(x) ERROR_LOG(x, __FILE__, __LINE__)

namespace Log {
    enum class LogLevels {
        INFO,
        WARN,
        ERROR
    };
    void INFO_LOG(std::string text, const char *f, int l);
    void WARN_LOG(std::string text, const char *f, int l);
    void ERROR_LOG(std::string text, const char *f, int l);
};
