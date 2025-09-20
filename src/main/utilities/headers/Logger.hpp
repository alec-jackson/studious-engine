#include <iostream>
#include <string>


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
