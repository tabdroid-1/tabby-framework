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

const std::unordered_map<std::string, Shared<Window>>& WindowManager::GetAllWindows()
{
    return m_ActiveWindowsByTag;
}

void WindowManager::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED: {

            // probably sohuld be error handling but eh
            auto window = m_ActiveWindowsByID[event.window.windowID];

            window->m_Specification.width = event.window.data1;
            window->m_Specification.height = event.window.data2;

            WindowResizeEvent resizeEvent(window->m_Specification.width, window->m_Specification.height);

            // SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)m_WindowHandle);
            // const char* window_tag = SDL_GetStringProperty(prop_id, "tabby_window_tag", "NO TAG FOUND");
            //
            // if (strcmp(window_tag, "main"))
            bgfx::reset(window->m_Specification.width, window->m_Specification.height);

            if (window->m_Specification.event_callback)
                window->m_Specification.event_callback(resizeEvent);

            break;
        }

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {

            auto window = m_ActiveWindowsByID[event.window.windowID];

            WindowCloseEvent closeEvent;
            window->m_Specification.event_callback(closeEvent);

            SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)window->Raw());
            const char* window_tag = SDL_GetStringProperty(prop_id, "tabby_window_tag", "NO TAG FOUND");
            Application::Get()->GetWindowManager()->RemoveWindow(window_tag);

            break;
        }

        case SDL_EVENT_KEY_DOWN: {
            auto window = m_ActiveWindowsByID[event.key.windowID];

            if (event.key.repeat == 0) {
                KeyPressedEvent keyPressedEvent((KeyCode)event.key.scancode, false);
                window->m_Specification.event_callback(keyPressedEvent);
            }
            break;
        }

        case SDL_EVENT_KEY_UP: {
            auto window = m_ActiveWindowsByID[event.key.windowID];

            KeyReleasedEvent keyReleasedEvent((KeyCode)event.key.scancode);
            window->m_Specification.event_callback(keyReleasedEvent);
            break;
        }

        case SDL_EVENT_TEXT_INPUT: {
            auto window = m_ActiveWindowsByID[event.text.windowID];

            KeyTypedEvent keyTypedEvent((KeyCode)event.text.text[0]);
            window->m_Specification.event_callback(keyTypedEvent);
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            auto window = m_ActiveWindowsByID[event.button.windowID];

            if (event.button.down) {
                MouseButtonPressedEvent mouseButtonPressedEvent((MouseCode)event.button.button);
                window->m_Specification.event_callback(mouseButtonPressedEvent);
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            auto window = m_ActiveWindowsByID[event.button.windowID];

            if (event.button.down) {
                MouseButtonReleasedEvent mouseButtonReleasedEvent((MouseCode)event.button.button);
                window->m_Specification.event_callback(mouseButtonReleasedEvent);
            }
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            auto window = m_ActiveWindowsByID[event.motion.windowID];

            MouseMovedEvent mouseMovedEvent(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
            window->m_Specification.event_callback(mouseMovedEvent);
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            auto window = m_ActiveWindowsByID[event.wheel.windowID];

            MouseScrolledEvent mouseScrolledEvent(static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y));
            window->m_Specification.event_callback(mouseScrolledEvent);
            break;
        }
        }
    }

    if (s_Instance->m_ActiveWindowsByTag.size() == 0) {
        AppCloseEvent event;
        Application::Get()->OnEvent(event);
    }
}

}
