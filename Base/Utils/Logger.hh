#pragma once

#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#define CHECK(expr, fail_msg, ...)                                                                                                                   \
    if (!(expr)) LOG_ERROR(fail_msg, ##__VA_ARGS__);

#define ENABLE_LOG_TRACE
#ifdef ENABLE_LOG_TRACE
#define LOG_TRACE(...) lr::Logger::s_pCoreLogger->trace(__VA_ARGS__)
#else
#define LOG_TRACE(...) (void *)0
#endif

#define LOG_INFO(...) lr::Logger::s_pCoreLogger->info(__VA_ARGS__)
#define LOG_WARN(...) lr::Logger::s_pCoreLogger->warn(__VA_ARGS__)
#define LOG_ERROR(...)                                                                                                                               \
    {                                                                                                                                                \
        lr::Logger::s_pCoreLogger->error(__VA_ARGS__);                                                                                               \
        lr::Logger::s_pCoreLogger->dump_backtrace();                                                                                                 \
        lr::Logger::s_pCoreLogger->flush();                                                                                                          \
        abort();                                                                                                                                     \
    }

namespace lr
{
    struct Logger
    {
        static void Init()
        {
            eastl::vector<spdlog::sink_ptr> logSinks;

            logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("lorr.log", true));

            logSinks[0]->set_pattern("[%n] %Y-%m-%d_%T.%e | %5^%L%$ | %v");
            logSinks[1]->set_pattern("[%n] %Y-%m-%d_%T.%e | %L | %v");

            s_pCoreLogger = std::make_shared<spdlog::logger>("LR", logSinks.begin(), logSinks.end());
            spdlog::register_logger(s_pCoreLogger);

            s_pCoreLogger->set_level(spdlog::level::trace);
            s_pCoreLogger->flush_on(spdlog::level::trace);
        }

        inline static std::shared_ptr<spdlog::logger> s_pCoreLogger = nullptr;
    };

}  // namespace lr