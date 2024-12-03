#include "tabby/event_application.h"
#include "tabby/window_manager.h"
#include "tabby/application.h"

#include <SDL3/SDL.h>

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
    if (s_Instance->m_ActiveWindows.find(tag) != s_Instance->m_ActiveWindows.end()) {
        TB_CORE_ERROR("A window with tag \'{}\' already exists", tag);
        return;
    }

    s_Instance->m_ActiveWindows[tag] = Window::Create(spec);
    auto raw_window = s_Instance->m_ActiveWindows[tag]->Raw();

    SDL_PropertiesID prop_id = SDL_GetWindowProperties((SDL_Window*)raw_window);
    SDL_SetStringProperty(prop_id, "tabby_window_tag", tag.c_str());
}

void WindowManager::RemoveWindow(const std::string& tag)
{
    auto it = s_Instance->m_ActiveWindows.find(tag);
    if (it == s_Instance->m_ActiveWindows.end()) {
        TB_CORE_ERROR("A window with tag \'{}\' does not exist", tag);
        return;
    }

    it->second->Destroy();

    s_Instance->m_ActiveWindows.erase(it);
}

Shared<Window> WindowManager::GetWindow(const std::string& tag)
{
    auto it = s_Instance->m_ActiveWindows.find(tag);
    if (it == s_Instance->m_ActiveWindows.end()) {
        TB_CORE_ERROR("A window with tag \'{}\' does not exist", tag);
        return nullptr;
    }

    return it->second;
}

const std::unordered_map<std::string, Shared<Window>>& WindowManager::GetAllWindows()
{
    return m_ActiveWindows;
}

void WindowManager::ProcessEvents()
{
    for (auto& window : s_Instance->m_ActiveWindows) {
        window.second->ProcessEvents();
    }

    if (s_Instance->m_ActiveWindows.size() == 0) {
        AppCloseEvent event;
        Application::Get()->OnEvent(event);
    }
}

}
