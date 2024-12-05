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

    if (!m_Specification.working_directory.empty())
        std::filesystem::current_path(m_Specification.working_directory);

    m_WindowManger = WindowManager::Init();
    m_Specification.main_window_spec.event_callback = TB_BIND_EVENT_FUNCTION(Application::OnEvent);
    m_WindowManger->AddWindow("main", m_Specification.main_window_spec);

    auto main_window = m_WindowManger->GetWindow("main");

    m_RootSystem = spec.root_system;
    m_RootSystem->Launch();

    m_ImGuiRenderer = new ImGuiRenderer();
    m_ImGuiRenderer->Launch(main_window->Raw());

    Input::Init();

    TB_CORE_TRACE("Application specifications");
    TB_CORE_TRACE("\tWorking directory: {}", m_Specification.working_directory);
    TB_CORE_TRACE("\tFlags: {}", m_Specification.flags);

    TB_CORE_TRACE("Main window specifications");
    TB_CORE_TRACE("\tName: {}", m_Specification.main_window_spec.window_title);
    TB_CORE_TRACE("\tExtent: {}, {}", m_Specification.main_window_spec.width, m_Specification.main_window_spec.height);
    TB_CORE_TRACE("\tMinimum extent: {}, {}", m_Specification.main_window_spec.min_width, m_Specification.main_window_spec.min_height);
    TB_CORE_TRACE("\tFullscreen mode: {}", m_Specification.main_window_spec.fullscreen_mode);
    TB_CORE_TRACE("\tResizeable: {}", m_Specification.main_window_spec.resizable);
}

void Application::Destroy()
{
    Input::Shutdown();
    WindowManager::Shutdown();
    m_RootSystem->Destroy();
    delete m_RootSystem;
}

ApplicationResult Application::Run()
{
    if (!m_Running)
        return TABBY_APPLICATION_SUCCESS;

    Time::FrameStart();
    Input::Update();

    {
        m_ImGuiRenderer->BeginFrame();

        m_RootSystem->OnUpdate();

        m_ImGuiRenderer->EndFrame();
    }

    bgfx::frame();
    m_WindowManger->ProcessEvents();

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
