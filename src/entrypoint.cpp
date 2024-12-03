#include "tabby/application.h"

using namespace Tabby;

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    Tabby::Log::Init();

    ApplicationSpecification app_spec = ConstructRootSystem();

    appstate[0] = (void*)new Application(app_spec);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto app = (Application*)appstate;
    return (SDL_AppResult)app->Run();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto app = (Application*)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        AppCloseEvent close_event;
        app->OnEvent(close_event);
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto app = (Application*)appstate;

    app->Destroy();
    delete app;
}
