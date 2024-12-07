#pragma once

#include "window.h"

namespace Tabby {

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    static WindowManager* Init();
    static void Shutdown();

    void AddWindow(const std::string tag, const WindowSpecification& config);
    void RemoveWindow(const std::string& tag);

    const std::unordered_map<std::string, Shared<Window>>& GetAllWindowsByTag();
    const std::unordered_map<uint64_t, Shared<Window>>& GetAllWindowsByID();
    Shared<Window> GetWindow(const std::string& tag);

    ApplicationResult ProcessEvents(void* event);

protected:
    inline static WindowManager* s_Instance;
    std::unordered_map<std::string, Shared<Window>> m_ActiveWindowsByTag;
    std::unordered_map<uint64_t, Shared<Window>> m_ActiveWindowsByID;
};

};
