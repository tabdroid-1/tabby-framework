#include "tabby/event_application.h"
#include "tabby/window_manager.h"
#include "tabby/event_mouse.h"
#include "tabby/application.h"
#include "tabby/event_key.h"
#include "tabby/window.h"

#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

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
#if defined(SDL_PLATFORM_WIN32)
    platform_data.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    // if (hwnd) {
    //     ...
    // }
#elif defined(SDL_PLATFORM_MACOS)
    // NSWindow* nswindow = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        platform_data.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        platform_data.nwh = (void*)SDL_GetNumberProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        platform_data.type = bgfx::NativeWindowHandleType::Default;

    } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        platform_data.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        platform_data.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)m_WindowHandle), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        platform_data.type = bgfx::NativeWindowHandleType::Wayland;
    }
#elif defined(SDL_PLATFORM_IOS)
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

void Window::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED: {
            if (event.window.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            m_Specification.width = event.window.data1;
            m_Specification.height = event.window.data2;

            WindowResizeEvent resizeEvent(m_Specification.width, m_Specification.height);

            // SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)m_WindowHandle);
            // const char* window_tag = SDL_GetStringProperty(prop_id, "tabby_window_tag", "NO TAG FOUND");
            //
            // if (strcmp(window_tag, "main"))
            bgfx::reset(m_Specification.width, m_Specification.height);

            if (m_Specification.event_callback)
                m_Specification.event_callback(resizeEvent);

            break;
        }

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            if (event.window.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            WindowCloseEvent closeEvent;
            m_Specification.event_callback(closeEvent);

            SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)m_WindowHandle);
            const char* window_tag = SDL_GetStringProperty(prop_id, "tabby_window_tag", "NO TAG FOUND");
            Application::Get()->GetWindowManager()->RemoveWindow(window_tag);

            break;
        }

        case SDL_EVENT_KEY_DOWN: {
            if (event.key.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            if (event.key.repeat == 0) {
                KeyPressedEvent keyPressedEvent((KeyCode)event.key.scancode, false);
                m_Specification.event_callback(keyPressedEvent);
            }
            break;
        }

        case SDL_EVENT_KEY_UP: {
            if (event.key.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            KeyReleasedEvent keyReleasedEvent((KeyCode)event.key.scancode);
            m_Specification.event_callback(keyReleasedEvent);
            break;
        }

        case SDL_EVENT_TEXT_INPUT: {
            if (event.text.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            KeyTypedEvent keyTypedEvent((KeyCode)event.text.text[0]);
            m_Specification.event_callback(keyTypedEvent);
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event.button.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            if (event.button.down) {
                MouseButtonPressedEvent mouseButtonPressedEvent((MouseCode)event.button.button);
                m_Specification.event_callback(mouseButtonPressedEvent);
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            if (event.button.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            if (event.button.down) {
                MouseButtonReleasedEvent mouseButtonReleasedEvent((MouseCode)event.button.button);
                m_Specification.event_callback(mouseButtonReleasedEvent);
            }
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            if (event.motion.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            MouseMovedEvent mouseMovedEvent(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
            m_Specification.event_callback(mouseMovedEvent);
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            if (event.wheel.windowID != SDL_GetWindowID((SDL_Window*)m_WindowHandle))
                break;

            MouseScrolledEvent mouseScrolledEvent(static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y));
            m_Specification.event_callback(mouseScrolledEvent);
            break;
        }
        }
    }
}

void Window::Destroy()
{
    if (m_WindowHandle) {
        SDL_DestroyWindow((SDL_Window*)m_WindowHandle);
        m_WindowHandle = nullptr;
    }
}

}
