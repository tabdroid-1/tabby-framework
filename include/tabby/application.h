#pragma once

#include "event_application.h"
#include "window_manager.h"
#include "subsystem.h"
#include "defines.h"
#include "window.h"

namespace Tabby {

struct ApplicationSpecification {
    WindowSpecification main_window_spec;
    std::string working_directory;
    Subsystem* root_system;
    uint64_t flags = 0;
};

typedef enum ApplicationResult {
    TABBY_APPLICATION_CONTINUE, /**< Value that requests that the app continue from the main callbacks. */
    TABBY_APPLICATION_SUCCESS, /**< Value that requests termination with success from the main callbacks. */
    TABBY_APPLICATION_FAILURE /**< Value that requests termination with error from the main callbacks. */
} ApplicationResult;

// need to be defined by user
extern ApplicationSpecification ConstructRootSystem();

class Application {
public:
    enum class LaunchOptionsFlags {
        HEADLESS = BIT(0),
        OPENGLES = BIT(1),
        OPENGL = BIT(2),
        VULKAN = BIT(3),
        METAL = BIT(4),
        DIRECT3D = BIT(5),
    };

    Application(const ApplicationSpecification& spec);

    static Application* Get() { return s_Instance; }

    ApplicationResult Run();
    void Destroy();
    void OnEvent(Event& e);

    WindowManager* GetWindowManager() const { return m_WindowManger; }

private:
    bool OnExit(AppCloseEvent& e);
    bool OnMainWindowResize(WindowResizeEvent& e);

private:
    inline static Application* s_Instance;

    ApplicationSpecification m_Specification;
    WindowManager* m_WindowManger;
    Subsystem* m_RootSystem;
    bool m_Running;
};

}
