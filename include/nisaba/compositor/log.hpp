#pragma once

#include <format>
#include <iostream>
#include <string_view>

namespace nisaba::compositor {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
};

class Logger {
public:
    static void setLevel(LogLevel level) noexcept {
        s_currentLevel = level;
    }

    static LogLevel getLevel() noexcept {
        return s_currentLevel;
    }

    template <typename... Args>
    static void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
        if (level < s_currentLevel) {
            return;
        }

        std::string message = std::format(fmt, std::forward<Args>(args)...);
        std::string_view levelStr;
        std::string_view colorCode;

        switch (level) {
            case LogLevel::Trace:
                levelStr = "[TRACE]";
                colorCode = "\033[90m"; // Gray
                break;
            case LogLevel::Debug:
                levelStr = "[DEBUG]";
                colorCode = "\033[36m"; // Cyan
                break;
            case LogLevel::Info:
                levelStr = "[INFO ]";
                colorCode = "\033[32m"; // Green
                break;
            case LogLevel::Warn:
                levelStr = "[WARN ]";
                colorCode = "\033[33m"; // Yellow
                break;
            case LogLevel::Error:
                levelStr = "[ERROR]";
                colorCode = "\033[31m"; // Red
                break;
            case LogLevel::Critical:
                levelStr = "[CRIT ]";
                colorCode = "\033[1;31m"; // Bold Red
                break;
        }

        std::ostream& stream = (level >= LogLevel::Warn) ? std::cerr : std::cout;
        stream << colorCode << "[Nisaba::Compositor]" << levelStr << " " << message << "\033[0m\n";
    }

private:
    inline static LogLevel s_currentLevel{LogLevel::Info};
};

template <typename... Args>
inline void logTrace(std::format_string<Args...> fmt, Args&&... args) {
    Logger::log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logDebug(std::format_string<Args...> fmt, Args&&... args) {
    Logger::log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logInfo(std::format_string<Args...> fmt, Args&&... args) {
    Logger::log(LogLevel::Info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logWarn(std::format_string<Args...> fmt, Args&&... args) {
    Logger::log(LogLevel::Warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logError(std::format_string<Args...> fmt, Args&&... args) {
    Logger::log(LogLevel::Error, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logCritical(std::format_string<Args...> fmt, Args&&... args) {
    Logger::log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
}

} // namespace nisaba::compositor
