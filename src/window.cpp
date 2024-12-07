#include "tabby/event_application.h"
#include "tabby/window_manager.h"
#include "tabby/event_mouse.h"
#include "tabby/application.h"
#include "tabby/event_key.h"
#include "tabby/window.h"

#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace Tabby {

Shared<Window> Window::Create(const WindowSpecification& spec)
{
    return CreateShared<Window>(spec);
}

Window::Window(const WindowSpecification& spec)
{
    RefreshSpecification(spec);
}

Window::~Window()
{
    Destroy();
}

void Window::RefreshSpecification(const WindowSpecification& spec)
{
    int success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    if (!success) {
        TB_CORE_INFO("{}", SDL_GetError());
    }
    TB_CORE_ASSERT_TAGGED(success, "Could not initialize SDL!");

    {
        // TB_PROFILE_SCOPE_NAME("Tabby::LinuxWindow::Init::SDL_CreateWindow");

        m_WindowHandle = SDL_CreateWindow(
            spec.window_title.c_str(),
            spec.width, spec.height,
            0);

        SDL_SetWindowResizable((SDL_Window*)m_WindowHandle, (spec.flags & TABBY_WINDOW_RESIZEABLE));
        SDL_SetWindowMinimumSize((SDL_Window*)m_WindowHandle, spec.min_width, spec.min_height);

        if (spec.flags & TABBY_WINDOW_MINIMIZE)
            SDL_MinimizeWindow((SDL_Window*)m_WindowHandle);
        else
            SDL_MaximizeWindow((SDL_Window*)m_WindowHandle);

        SDL_SetWindowFullscreen((SDL_Window*)m_WindowHandle, (spec.flags & TABBY_WINDOW_FULLSCREEN) ? true : false);
    }

    bgfx::PlatformData platform_data;
#if defined(TB_PLATFORM_WINDOWS)
    platform_data.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif defined(TB_PLATFORM_MACOS)
#    if BGFX_CONFIG_MULTITHREADED == 1
    bgfx::renderFrame();
#    endif

    platform_data.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
#elif defined(TB_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        platform_data.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        platform_data.nwh = (void*)SDL_GetNumberProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        platform_data.type = bgfx::NativeWindowHandleType::Default;

    } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        platform_data.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        platform_data.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        platform_data.type = bgfx::NativeWindowHandleType::Wayland;
    }
#elif defined(TB_PLATFORM_ANDROID)
    platform_data.nwh = (void*)"#canvas";
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
#elif defined(TB_PLATFORM_WEB)
    platform_data.nwh = (void*)"#canvas";
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
#elif defined(TB_PLATFORM_IOS)
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    platform_data.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#endif
    static bool s_Initialized = false;

    if (!s_Initialized) {
        bgfx::Init bgfxInit;
        bgfxInit.platformData = platform_data;
        bgfxInit.resolution.width = spec.width;
        bgfxInit.resolution.height = spec.height;
        bgfxInit.type = bgfx::RendererType::Count;

        uint64_t flags = Application::Get()->GetFlags();
        if (flags & TABBY_LAUNCH_OPTION_NO_API)
            bgfxInit.type = bgfx::RendererType::Noop;
        if (flags & TABBY_LAUNCH_OPTION_AGC)
            bgfxInit.type = bgfx::RendererType::Agc;
        if (flags & TABBY_LAUNCH_OPTION_DIRECT3D11)
            bgfxInit.type = bgfx::RendererType::Direct3D11;
        if (flags & TABBY_LAUNCH_OPTION_DIRECT3D12)
            bgfxInit.type = bgfx::RendererType::Direct3D12;
        if (flags & TABBY_LAUNCH_OPTION_GNM)
            bgfxInit.type = bgfx::RendererType::Gnm;
        if (flags & TABBY_LAUNCH_OPTION_METAL)
            bgfxInit.type = bgfx::RendererType::Metal;
        if (flags & TABBY_LAUNCH_OPTION_NVN)
            bgfxInit.type = bgfx::RendererType::Nvn;
        if (flags & TABBY_LAUNCH_OPTION_OPENGLES)
            bgfxInit.type = bgfx::RendererType::OpenGLES;
        if (flags & TABBY_LAUNCH_OPTION_OPENGL)
            bgfxInit.type = bgfx::RendererType::OpenGL;
        if (flags & TABBY_LAUNCH_OPTION_VULKAN)
            bgfxInit.type = bgfx::RendererType::Vulkan;
        if (flags & TABBY_LAUNCH_OPTION_AUTO_PICK_API)
            bgfxInit.type = bgfx::RendererType::Count;
        bgfx::init(bgfxInit);

        uint64_t debug_flags = 0;
        if (flags & TABBY_LAUNCH_OPTION_DEBUG_IFH)
            debug_flags |= BGFX_DEBUG_IFH;
        if (flags & TABBY_LAUNCH_OPTION_DEBUG_STATS)
            debug_flags |= BGFX_DEBUG_STATS;
        if (flags & TABBY_LAUNCH_OPTION_DEBUG_TEXT)
            debug_flags |= BGFX_DEBUG_TEXT;
        if (flags & TABBY_LAUNCH_OPTION_DEBUG_PROFILER)
            debug_flags |= BGFX_DEBUG_PROFILER;
        bgfx::setDebug(debug_flags);

        s_Initialized = true;
    }

    SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)m_WindowHandle);
    SDL_SetPointerProperty(prop_id, "tabby_window", this);

    m_Specification = spec;
}

void Window::Destroy()
{
    if (m_WindowHandle) {
        SDL_DestroyWindow((SDL_Window*)m_WindowHandle);
        m_WindowHandle = nullptr;
    }
}

}
