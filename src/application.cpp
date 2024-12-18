#include "tabby/event_application.h"
#include "tabby/window_manager.h"
#include "tabby/application.h"
#include "imgui_renderer.h"
#include "tabby/input.h"
#include "tabby/time.h"

#include <bgfx/bgfx.h>

namespace Tabby {

Application::Application(const ApplicationSpecification& spec)
    : m_Specification(spec)
{
    TB_CORE_ASSERT_TAGGED(!s_Instance, "Application already exists!");
    s_Instance = this;
    m_Running = true;

#ifndef TB_PLATFORM_WEB
    if (!m_Specification.working_directory.empty())
        std::filesystem::current_path(m_Specification.working_directory);
#endif // TB_PLATFORM_WEB

    m_WindowManger = WindowManager::Init();
    m_Specification.main_window_spec.event_callback = TB_BIND_EVENT_FUNCTION(Application::OnEvent);
    m_WindowManger->AddWindow("main", m_Specification.main_window_spec);

    auto main_window = m_WindowManger->GetWindow("main");

    if ((m_Specification.flags & TABBY_LAUNCH_OPTION_IMGUI)
        && !(m_Specification.flags & (TABBY_LAUNCH_OPTION_NO_API | TABBY_LAUNCH_OPTION_HEADLESS))) {
        m_ImGuiRenderer = new ImGuiRenderer();
        m_ImGuiRenderer->Launch(main_window->Raw());
    }

    Input::Init();

    m_RootSystem = spec.root_system;
    m_RootSystem->Launch();

    TB_CORE_TRACE("Application specifications");
    TB_CORE_TRACE("\tWorking directory: {}", m_Specification.working_directory);
    TB_CORE_TRACE("\tFlags: {}", m_Specification.flags);
    TB_CORE_TRACE("\tImgui enabled: {}", m_Specification.flags & TABBY_LAUNCH_OPTION_IMGUI);

    TB_CORE_TRACE("Main window specifications");
    TB_CORE_TRACE("\tName: {}", m_Specification.main_window_spec.window_title);
    TB_CORE_TRACE("\tExtent: {}, {}", m_Specification.main_window_spec.width, m_Specification.main_window_spec.height);
    TB_CORE_TRACE("\tMinimum extent: {}, {}", m_Specification.main_window_spec.min_width, m_Specification.main_window_spec.min_height);
    TB_CORE_TRACE("\tFullscreen mode: {}", m_Specification.main_window_spec.flags & TABBY_WINDOW_FULLSCREEN);
    TB_CORE_TRACE("\tResizeable: {}", m_Specification.main_window_spec.flags & TABBY_WINDOW_RESIZEABLE);
}

void Application::Destroy()
{
    Input::Shutdown();
    WindowManager::Shutdown();
    m_RootSystem->Destroy();
    delete m_RootSystem;

    if ((m_Specification.flags & TABBY_LAUNCH_OPTION_IMGUI)
        && !(m_Specification.flags & (TABBY_LAUNCH_OPTION_NO_API | TABBY_LAUNCH_OPTION_HEADLESS))) {
        m_ImGuiRenderer->Destroy();
        delete m_ImGuiRenderer;
    }
}

ApplicationResult Application::Run()
{
    if (!m_Running)
        return TABBY_APPLICATION_SUCCESS;

    Time::FrameStart();
    Input::Update();

    {
        if ((m_Specification.flags & TABBY_LAUNCH_OPTION_IMGUI)
            && !(m_Specification.flags & (TABBY_LAUNCH_OPTION_NO_API | TABBY_LAUNCH_OPTION_HEADLESS)))
            m_ImGuiRenderer->BeginFrame();

        m_RootSystem->OnUpdate();

        if ((m_Specification.flags & TABBY_LAUNCH_OPTION_IMGUI)
            && !(m_Specification.flags & (TABBY_LAUNCH_OPTION_NO_API | TABBY_LAUNCH_OPTION_HEADLESS)))
            m_ImGuiRenderer->EndFrame();
    }

    bgfx::frame();

    return TABBY_APPLICATION_CONTINUE;
}

void Application::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<AppCloseEvent>(TB_BIND_EVENT_FUNCTION(Application::OnExit));
    dispatcher.Dispatch<WindowResizeEvent>(TB_BIND_EVENT_FUNCTION(Application::OnMainWindowResize));

    m_RootSystem->OnEvent(e);
}

bool Application::OnExit(AppCloseEvent& e)
{
    m_Running = false;
    return true;
}

bool Application::OnMainWindowResize(WindowResizeEvent& e)
{

    return true;
}

}
