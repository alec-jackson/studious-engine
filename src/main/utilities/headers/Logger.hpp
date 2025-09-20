#include <iostream>
#include <string>

#if (SUPPRESS_LOGS == SUPPRESS_INFO)
#define INFO(x...)
#endif
#if (SUPPRESS_LOGS == SUPPRESS_WARN)
#define INFO(x...)
#define WARN(x...)
#endif
#if (SUPPRESS_LOGS == SUPPRESS_ERROR)
#define INFO(x...)
#define WARN(x...)
#define ERROR(x...)
#endif

// TODO: move Logger.cpp contents to Logger.hpp to avoid linker error

namespace Log {
    enum class LogLevels {
        INFO,
        WARN,
        ERROR
    };
    template<typename T, typename ...Args>
    void INFO(std::string text, T item, Args... remArgs);
    void INFO(std::string text);

    template<typename T, typename ...Args>
    void WARN(std::string text, T item, Args... remArgs);
    void WARN(std::string text);

    template<typename T, typename ...Args>
    void ERROR(std::string text, T item, Args... remArgs);
    void ERROR(std::string text);
};
