#pragma once


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <memory>
#include <string>
#include <string_view>
#include <mutex>
#include <vector>

namespace code_graph {

class Logger {
public:

    explicit Logger(std::string_view logPath = "code_graph.log");


    static Logger& global();
    template <typename... Args>
    void info(std::string_view category, typename fmt::fstring<Args...>::t f, Args&&... args);

    void debug(std::string_view category, std::string_view message);
    void info (std::string_view category, std::string_view message);
    void warn (std::string_view category, std::string_view message);
    void error(std::string_view category, std::string_view message);

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::mutex                      mutex_;

    void log(spdlog::level::level_enum lvl,
             std::string_view category,
             std::string_view message);
};

inline Logger::Logger(std::string_view logPath) {
    std::vector<spdlog::sink_ptr> sinks;


    sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_st>());

    if (!logPath.empty()) {

        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            std::string(logPath),
            5 * 1024,
            3));
    }

    logger_ = std::make_shared<spdlog::logger>("code_graph",
                                                sinks.begin(),
                                                sinks.end());

    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    logger_->set_level(spdlog::level::debug);
    logger_->flush_on(spdlog::level::warn);
}

inline Logger& Logger::global() {
    static Logger instance{};
    return instance;
}
template <typename... Args>
inline void Logger::info(std::string_view category, fmt::format_string<Args...> f, Args&&... args) {
    std::string message = fmt::format(f, std::forward<Args>(args)...);
    log(spdlog::level::debug, category, message);
}
inline void Logger::info(std::string_view category, std::string_view message) {
    log(spdlog::level::info, category, message);
}
inline void Logger::warn(std::string_view category, std::string_view message) {
    log(spdlog::level::warn, category, message);
}
inline void Logger::error(std::string_view category, std::string_view message) {
    log(spdlog::level::err, category, message);
}

inline void Logger::log(spdlog::level::level_enum lvl,
                        std::string_view category,
                        std::string_view message) {
    std::lock_guard<std::mutex> guard(mutex_);

    std::string formatted;
    formatted.reserve(category.size() + 2 + message.size());
    formatted.append("[").append(category).append("] ").append(message);
    logger_->log(lvl, "{}", formatted);
}


inline void log_debug(std::string_view c, std::string_view m) { Logger::global().debug(c, m); }
template <typename... Args>
inline void log_info (std::string_view c, fmt::format_string<Args...> f, Args&&... args) { Logger::global().info(c, f, std::forward<Args>(args)...); }
inline void log_warn (std::string_view c, std::string_view m) { Logger::global().warn(c, m); }
inline void log_error(std::string_view c, std::string_view m) { Logger::global().error(c, m); }

}   
