#include "tabby/application.h"
#include "tabby/input.h"

#include <SDL3/SDL.h>

namespace Tabby {

void Input::Init()
{
    if (!s_Instance)
        s_Instance = new Input();
}
void Input::Shutdown()
{
    if (s_Instance)
        delete s_Instance;
}

void Input::Update()
{

    auto window_manager = Application::Get()->GetWindowManager();

    for (auto window : window_manager->GetAllWindows()) {

        WindowInputData data;

        if (s_Instance->m_WindowInputData.find(window.first) != s_Instance->m_WindowInputData.end())
            data = s_Instance->m_WindowInputData[window.first];

        // Update key states
        const bool* keyState = SDL_GetKeyboardState(nullptr);
        for (auto& key : data.current_key_states) {
            data.previous_key_states[key.first] = key.second;
            key.second = keyState[static_cast<SDL_Scancode>(key.first)] == 1;
        }

        // Update mouse button states
        Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
        for (auto& button : data.current_mouse_button_states) {
            data.previous_mouse_button_states[button.first] = button.second;
            button.second = (mouseState & SDL_BUTTON_MASK(static_cast<int>(button.first))) != 0;
        }

        s_Instance->m_WindowInputData[window.first] = data;
    }
}

bool Input::GetKeyDown(KeyCode key, const std::string& window_tag)
{
    return s_Instance->m_WindowInputData[window_tag].current_key_states[key] && !s_Instance->m_WindowInputData[window_tag].previous_key_states[key];
}

bool Input::GetKeyUp(KeyCode key, const std::string& window_tag)
{
    return !s_Instance->m_WindowInputData[window_tag].current_key_states[key] && s_Instance->m_WindowInputData[window_tag].previous_key_states[key];
}

bool Input::GetKey(KeyCode key, const std::string& window_tag)
{
    return s_Instance->m_WindowInputData[window_tag].current_key_states[key];
}

bool Input::GetMouseButtonDown(MouseCode button, const std::string& window_tag)
{
    return s_Instance->m_WindowInputData[window_tag].current_mouse_button_states[button] && !s_Instance->m_WindowInputData[window_tag].previous_mouse_button_states[button];
}

bool Input::GetMouseButtonUp(MouseCode button, const std::string& window_tag)
{
    return !s_Instance->m_WindowInputData[window_tag].current_mouse_button_states[button] && s_Instance->m_WindowInputData[window_tag].previous_mouse_button_states[button];
}

bool Input::GetMouseButton(MouseCode button, const std::string& window_tag)
{
    return s_Instance->m_WindowInputData[window_tag].current_mouse_button_states[button];
}

// Vector2 Input::GetMouseScrollDelta(const std::string& window_tag)
// {
//     TB_PROFILE_SCOPE_NAME("Tabby::Input::GetMouseScrollDelta");
//
//     return s_Instance->m_MouseScrollDelta;
// }
//
// Vector2 Input::GetMousePosition()
// {
//     TB_PROFILE_SCOPE_NAME("Tabby::Input::GetMousePosition");
//
//     int x, y;
//     SDL_GetMouseState(&x, &y);
//     return { static_cast<float>(x), static_cast<float>(y) };
// }
//
float Input::GetMouseX(const std::string& window_tag)
{
    // return GetMousePosition().x;
    return 0;
}

float Input::GetMouseY(const std::string& window_tag)
{
    // return GetMousePosition().y;
    return 0;
}

void Input::LockAndHideMouse(const std::string& window_tag)
{
}
void Input::ReleaseAndShowMouse(const std::string& window_tag)
{
}
}
