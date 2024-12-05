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
    if (success != 0) {
        TB_CORE_INFO("{}", SDL_GetError());
    }
    TB_CORE_ASSERT_TAGGED(success, "Could not initialize SDL!");

    {
        // TB_PROFILE_SCOPE_NAME("Tabby::LinuxWindow::Init::SDL_CreateWindow");

        m_WindowHandle = SDL_CreateWindow(
            spec.window_title.c_str(),
            spec.width, spec.height,
            0);
    }

    bgfx::PlatformData platform_data;
#if defined(TB_PLATFORM_WINDOWS)
    platform_data.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    // if (hwnd) {
    //     ...
    // }
#elif defined(TB_PLATFORM_MACOS)
    bgfx::renderFrame();
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
#elif defined(TB_PLATFORM_IOS)
    // SDL_PropertiesID props = SDL_GetWindowProperties(window);
    // UIWindow* uiwindow = (__bridge UIWindow*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#endif

    bgfx::Init bgfxInit;
    bgfxInit.type = bgfx::RendererType::Count; // Automatically choose a renderer.
    bgfxInit.platformData = platform_data;
    bgfxInit.resolution.width = spec.width;
    bgfxInit.resolution.height = spec.height;
    bgfx::init(bgfxInit);
    bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);

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
