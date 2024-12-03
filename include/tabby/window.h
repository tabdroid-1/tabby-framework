#pragma once

#include "event.h"

namespace Tabby {

struct WindowSpecification {
    using EventCallbackFn = std::function<void(Event&)>;

    std::string window_title; // window title
    uint32_t width;
    uint32_t height;
    uint32_t min_width;
    uint32_t min_height;
    uint8_t fullscreen_mode;
    bool resizable;
    EventCallbackFn event_callback;

    static WindowSpecification Default()
    {
        WindowSpecification spec;
        spec.window_title = "Window";
        spec.width = 1920;
        spec.height = 1080;
        spec.min_width = 192;
        spec.min_height = 108;
        spec.fullscreen_mode = 0;
        spec.resizable = true;
        return spec;
    }
};

class Window {
public:
    static Shared<Window> Create(const WindowSpecification& spec);

    Window(const WindowSpecification& spec);
    ~Window();

    void ProcessEvents();

    void RefreshSpecification(const WindowSpecification& spec);

    const WindowSpecification& GetSpecification() const { return m_Specification; }
    void* Raw() const { return m_WindowHandle; }

    void Destroy();

private:
    void* m_WindowHandle;

    WindowSpecification m_Specification;
};

}
