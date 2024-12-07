#pragma once

#include "event_application.h"
#include "window_manager.h"
#include "subsystem.h"
#include "defines.h"
#include "window.h"

namespace Tabby {

class ImGuiRenderer;

struct ApplicationSpecification {
    WindowSpecification main_window_spec;
    std::string working_directory;
    Subsystem* root_system;
    uint64_t flags = 0;
};

// need to be defined by user
extern ApplicationSpecification ConstructRootSystem();

class Application {
public:
    Application(const ApplicationSpecification& spec);

    static Application* Get() { return s_Instance; }

    ApplicationResult Run();
    void Destroy();
    void OnEvent(Event& e);

    WindowManager* GetWindowManager() const { return m_WindowManger; }

    uint64_t GetFlags() const { return m_Specification.flags; }
    void SetFlags(uint64_t flags);

private:
    bool OnExit(AppCloseEvent& e);
    bool OnMainWindowResize(WindowResizeEvent& e);

private:
    inline static Application* s_Instance;

    ApplicationSpecification m_Specification;
    WindowManager* m_WindowManger;
    ImGuiRenderer* m_ImGuiRenderer;
    Subsystem* m_RootSystem;
    bool m_Running;
};

}
