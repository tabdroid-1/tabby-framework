#pragma once

#include "input_key_code.h"

namespace Tabby {

class Input {
public:
    static void Init();
    static void Shutdown();

    static bool GetKeyDown(KeyCode key, const std::string& window_tag = "main");
    static bool GetKeyUp(KeyCode key, const std::string& window_tag = "main");
    static bool GetKey(KeyCode key, const std::string& window_tag = "main");
    static bool GetMouseButtonDown(MouseCode button, const std::string& window_tag = "main");
    static bool GetMouseButtonUp(MouseCode button, const std::string& window_tag = "main");
    static bool GetMouseButton(MouseCode button, const std::string& window_tag = "main");

    // static Vector2 GetMousePosition(const std::string& window_tag = "main");
    // static Vector2 GetMouseScrollDelta(const std::string& window_tag = "main");
    static float GetMouseX(const std::string& window_tag = "main");
    static float GetMouseY(const std::string& window_tag = "main");

    static void LockAndHideMouse(const std::string& window_tag = "main");
    static void ReleaseAndShowMouse(const std::string& window_tag = "main");

private:
    static void Update();

    struct WindowInputData {
        std::unordered_map<KeyCode, bool> current_key_states;
        std::unordered_map<KeyCode, bool> previous_key_states;
        std::unordered_map<MouseCode, bool> current_mouse_button_states;
        std::unordered_map<MouseCode, bool> previous_mouse_button_states;
    };

private:
    inline static Input* s_Instance;
    //                 window tag   input data
    std::unordered_map<std::string, WindowInputData> m_WindowInputData;

    friend class Application;
};

}
