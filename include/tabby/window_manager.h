#pragma once

#include "window.h"

namespace Tabby {

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    static WindowManager* Init();

    static void Shutdown() { delete s_Instance; };

    void AddWindow(const std::string tag, const WindowSpecification& config);
    void RemoveWindow(const std::string& tag);

    const std::unordered_map<std::string, Shared<Window>>& GetAllWindows();
    Shared<Window> GetWindow(const std::string& tag);

    void ProcessEvents();

protected:
    inline static WindowManager* s_Instance;
    std::unordered_map<std::string, Shared<Window>> m_ActiveWindowsByTag;
    std::unordered_map<uint64_t, Shared<Window>> m_ActiveWindowsByID;
};

};
