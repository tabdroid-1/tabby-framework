#pragma once

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

namespace Tabby {
class Log {
public:
    static void Init();

    static std::shared_ptr<spdlog::logger>& GetCoreLogger()
    {
        return s_CoreLogger;
    }
    static std::shared_ptr<spdlog::logger>& GetClientLogger()
    {
        return s_ClientLogger;
    }

private:
    inline static std::shared_ptr<spdlog::logger> s_CoreLogger;
    inline static std::shared_ptr<spdlog::logger> s_ClientLogger;
};
}

// Core log macros
#define TB_CORE_TRACE(...) ::Tabby::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TB_CORE_INFO(...) ::Tabby::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TB_CORE_WARN(...) ::Tabby::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TB_CORE_ERROR(...) ::Tabby::Log::GetCoreLogger()->error(__VA_ARGS__)
#define TB_CORE_CRITICAL(...) ::Tabby::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define TB_TRACE(...) ::Tabby::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TB_INFO(...) ::Tabby::Log::GetClientLogger()->info(__VA_ARGS__)
#define TB_WARN(...) ::Tabby::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TB_ERROR(...) ::Tabby::Log::GetClientLogger()->error(__VA_ARGS__)
#define TB_CRITICAL(...) ::Tabby::Log::GetClientLogger()->critical(__VA_ARGS__)
