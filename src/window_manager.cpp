#include "tabby/event_application.h"
#include "tabby/window_manager.h"
#include "tabby/application.h"
#include "tabby/event_mouse.h"
#include "tabby/application.h"
#include "tabby/event_key.h"

#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>
#include "imgui/backends/imgui_impl_sdl3.h"

namespace Tabby {

WindowManager::WindowManager()
{
    if (Application::Get()->GetFlags() & TABBY_LAUNCH_OPTION_WAYLAND)
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");
    else if (Application::Get()->GetFlags() & TABBY_LAUNCH_OPTION_X11)
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
}

WindowManager::~WindowManager()
{
}

WindowManager* WindowManager::Init()
{
    if (!s_Instance)
        s_Instance = new WindowManager();

    return s_Instance;
}

void WindowManager::Shutdown()
{
    if (s_Instance)
        delete s_Instance;
}

void WindowManager::AddWindow(const std::string tag, const WindowSpecification& spec)
{
    if (s_Instance->m_ActiveWindowsByTag.find(tag) != s_Instance->m_ActiveWindowsByTag.end()) {
        TB_CORE_ERROR("A window with tag \'{}\' already exists", tag);
        return;
    }

    auto window = Window::Create(spec);
    auto window_id = SDL_GetWindowID((SDL_Window*)window->Raw());
    s_Instance->m_ActiveWindowsByTag[tag] = window;
    s_Instance->m_ActiveWindowsByID[window_id] = window;

    SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)window->Raw());
    SDL_SetStringProperty(prop_id, "tabby_window_tag", tag.c_str());
}

void WindowManager::RemoveWindow(const std::string& tag)
{
    auto it_tag = s_Instance->m_ActiveWindowsByTag.find(tag);
    if (it_tag == s_Instance->m_ActiveWindowsByTag.end()) {
        TB_CORE_ERROR("A window with tag \'{}\' does not exist", tag);
        return;
    }

    auto window_id = SDL_GetWindowID((SDL_Window*)it_tag->second->Raw());

    auto it_id = s_Instance->m_ActiveWindowsByID.find(window_id);
    if (it_id == s_Instance->m_ActiveWindowsByID.end()) {
        TB_CORE_ERROR("A window with id \'{}\' does not exist", (uint64_t)window_id);
        return;
    }

    it_tag->second->Destroy();

    s_Instance->m_ActiveWindowsByTag.erase(it_tag);
    s_Instance->m_ActiveWindowsByID.erase(it_id);
}

Shared<Window> WindowManager::GetWindow(const std::string& tag)
{
    auto it = s_Instance->m_ActiveWindowsByTag.find(tag);
    if (it == s_Instance->m_ActiveWindowsByTag.end()) {
        TB_CORE_ERROR("A window with tag \'{}\' does not exist", tag);
        return nullptr;
    }

    return it->second;
}

const std::unordered_map<std::string, Shared<Window>>& WindowManager::GetAllWindowsByTag()
{
    return m_ActiveWindowsByTag;
}

const std::unordered_map<uint64_t, Shared<Window>>& WindowManager::GetAllWindowsByID()
{
    return m_ActiveWindowsByID;
}

ApplicationResult WindowManager::ProcessEvents(void* event)
{

    auto sdl_event = (SDL_Event*)event;
    auto window_manager = Application::Get()->GetWindowManager();

    uint64_t flags = Application::Get()->GetFlags();
    if ((flags & TABBY_LAUNCH_OPTION_IMGUI)
        && !(flags & (TABBY_LAUNCH_OPTION_NO_API | TABBY_LAUNCH_OPTION_HEADLESS)))
        ImGui_ImplSDL3_ProcessEvent((SDL_Event*)event);

    switch (sdl_event->type) {

    case SDL_EVENT_WINDOW_RESIZED: {

        // probably sohuld be error handling but eh
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->window.windowID];

        window->m_Specification.width = sdl_event->window.data1;
        window->m_Specification.height = sdl_event->window.data2;

        WindowResizeEvent resizeEvent(window->m_Specification.width, window->m_Specification.height);

        // SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)m_WindowHandle);
        // const char* window_tag = SDL_GetStringProperty(prop_id, "tabby_window_tag", "NO TAG FOUND");
        //
        // if (strcmp(window_tag, "main"))
        bgfx::reset(window->m_Specification.width, window->m_Specification.height);
        TB_CORE_INFO("{}, {}", window->m_Specification.width, window->m_Specification.height);

        if (window->m_Specification.event_callback)
            window->m_Specification.event_callback(resizeEvent);

        break;
    }

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {

        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->window.windowID];

        WindowCloseEvent closeEvent;
        window->m_Specification.event_callback(closeEvent);

        SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)window->Raw());
        const char* window_tag = SDL_GetStringProperty(prop_id, "tabby_window_tag", "NO TAG FOUND");
        Application::Get()->GetWindowManager()->RemoveWindow(window_tag);

        break;
    }

    case SDL_EVENT_KEY_DOWN: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->key.windowID];

        if (sdl_event->key.repeat == 0) {
            KeyPressedEvent keyPressedEvent((KeyCode)sdl_event->key.scancode, false);
            window->m_Specification.event_callback(keyPressedEvent);
        }
        break;
    }

    case SDL_EVENT_KEY_UP: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->key.windowID];

        KeyReleasedEvent keyReleasedEvent((KeyCode)sdl_event->key.scancode);
        window->m_Specification.event_callback(keyReleasedEvent);
        break;
    }

    case SDL_EVENT_TEXT_INPUT: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->text.windowID];

        KeyTypedEvent keyTypedEvent((KeyCode)sdl_event->text.text[0]);
        window->m_Specification.event_callback(keyTypedEvent);
        break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->button.windowID];

        if (sdl_event->button.down) {
            MouseButtonPressedEvent mouseButtonPressedEvent((MouseCode)sdl_event->button.button);
            window->m_Specification.event_callback(mouseButtonPressedEvent);
        }
        break;
    }

    case SDL_EVENT_MOUSE_BUTTON_UP: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->button.windowID];

        if (sdl_event->button.down) {
            MouseButtonReleasedEvent mouseButtonReleasedEvent((MouseCode)sdl_event->button.button);
            window->m_Specification.event_callback(mouseButtonReleasedEvent);
        }
        break;
    }

    case SDL_EVENT_MOUSE_MOTION: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->motion.windowID];

        MouseMovedEvent mouseMovedEvent(static_cast<float>(sdl_event->motion.x), static_cast<float>(sdl_event->motion.y));
        window->m_Specification.event_callback(mouseMovedEvent);
        break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
        auto& window = window_manager->m_ActiveWindowsByID[sdl_event->wheel.windowID];

        MouseScrolledEvent mouseScrolledEvent(static_cast<float>(sdl_event->wheel.x), static_cast<float>(sdl_event->wheel.y));
        window->m_Specification.event_callback(mouseScrolledEvent);
        break;
    }

    case SDL_EVENT_QUIT: {
        AppCloseEvent close_event;
        Application::Get()->OnEvent(close_event);

        return TABBY_APPLICATION_SUCCESS;
    }
    }
    return TABBY_APPLICATION_CONTINUE;

    // SDL_Event event;
    // while (SDL_PollEvent(&event)) {
    //     TB_CORE_INFO("1");
    //     // ImGui_ImplSDL3_ProcessEvent(&event);
    // }

    // if (s_Instance->m_ActiveWindowsByTag.size() == 0) {
    //     AppCloseEvent event;
    //     Application::Get()->OnEvent(event);
    // }
}

}
