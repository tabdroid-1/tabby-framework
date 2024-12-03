#pragma once

#include <memory>

// Platform detection using predefined macros
// TODO: Add TB_ARCH
#ifdef _WIN32
/* Windows x64/x86 */
#    ifdef _WIN64
/* Windows x64  */
#        define TB_PLATFORM_WINDOWS
#    else
/* Windows x86 */
#        define TB_PLATFORM_WINDOWS
#    endif
#elif defined(__APPLE__) || defined(__MACH__)
#    include <TargetConditionals.h>
/* TARGET_OS_MAC exists on all the platforms
 * so we must check all of them (in this order)
 * to ensure that we're running on MAC
 * and not some other Apple platform */
#    if TARGET_IPHONE_SIMULATOR
#        define TB_PLATFORM_IOS
#        error "IOS simulator is not supported!"
#    elif TARGET_OS_IPHONE
#        define TB_PLATFORM_IOS
#        error "IOS is not supported!"
#    elif TARGET_OS_MAC
#        define TB_PLATFORM_MACOS
#    else
#        error "Unknown Apple platform!"
#    endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
#    define TB_PLATFORM_ANDROID
#elif defined(__linux__)
#    define TB_PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__)
#    define TB_PLATFORM_WEB
#else
/* Unknown compiler/platform */
#    error "Unknown platform!"
#endif // End of platform detection

#if defined(__x86_64__) || defined(_M_X64)
#    define TB_CPU_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#    define TB_CPU_X86_32
#elif defined(__aarch64__) || defined(_M_ARM64)
#    define TB_CPU_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#    define TB_CPU_ARM32
#elif defined(__EMSCRIPTEN__)
#    define TB_CPU_WASM
#else
#    error Unknown CPU architexture
#endif

#if defined(TB_PLATFORM_WINDOWS)
#    define TB_DEBUGBREAK() __debugbreak()
#elif defined(TB_PLATFORM_LINUX)
#    include <signal.h>
#    define TB_DEBUGBREAK() raise(SIGTRAP)
#elif defined(TB_PLATFORM_MACOS)
#    include <signal.h>
#    define TB_DEBUGBREAK() raise(SIGTRAP)
#elif defined(TB_PLATFORM_ANDROID)
#    define TB_DEBUGBREAK() __builtin_trap()
#elif defined(TB_PLATFORM_WEB)
#    include <emscripten.h>
#    define TB_DEBUGBREAK() emscripten_force_exit(1)
// #define TB_DEBUGBREAK() emscripten_debugger()
#else
#    error "Platform doesn't support debugbreak yet!"
#endif

#ifdef TB_DEBUG
#    define TB_ENABLE_ASSERTS
#endif

#define TB_ENABLE_VERIFY

#define TB_EXPAND_MACRO(x) x
#define TB_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define TB_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define TB_CORE_VERIFY(expression)                                           \
    do {                                                                     \
        if (!(expression)) {                                                 \
            TB_CORE_ERROR("Assertion failed: {0}({1})", __FILE__, __LINE__); \
            TB_DEBUGBREAK();                                                 \
        }                                                                    \
    } while (false)
#define TB_CORE_VERIFY_TAGGED(expression, ...)      \
    do {                                            \
        if (!(expression)) {                        \
            TB_CORE_ERROR(                          \
                "Assertion failed: {0}({1}) | {2}", \
                __FILE__,                           \
                __LINE__,                           \
                __VA_ARGS__);                       \
            TB_DEBUGBREAK();                        \
        }                                           \
    } while (false)

#ifdef TB_ENABLE_ASSERTS

#    define TB_CORE_ASSERT(expression)                                           \
        do {                                                                     \
            if (!(expression)) {                                                 \
                TB_CORE_ERROR("Assertion failed: {0}({1})", __FILE__, __LINE__); \
                TB_DEBUGBREAK();                                                 \
            }                                                                    \
        } while (false)
#    define TB_CORE_ASSERT_TAGGED(expression, ...)      \
        do {                                            \
            if (!(expression)) {                        \
                TB_CORE_ERROR(                          \
                    "Assertion failed: {0}({1}) | {2}", \
                    __FILE__,                           \
                    __LINE__,                           \
                    __VA_ARGS__);                       \
                TB_DEBUGBREAK();                        \
            }                                           \
        } while (false)
#else
#    define TB_CORE_ASSERT(expression)
#    define TB_CORE_ASSERT_TAGGED(expression, ...)
#endif

namespace Tabby {

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Shared = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Shared<T> CreateShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T1, typename T2>
Shared<T1> ShareAs(Shared<T2>& ptr)
{
    return std::static_pointer_cast<T1>(ptr);
};

template <typename T1, typename T2>
Shared<T1> ShareAs(const Shared<T2>& ptr)
{
    return std::static_pointer_cast<T1>(ptr);
};

}

#include "log.h"
