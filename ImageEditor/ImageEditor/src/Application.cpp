#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "SDL.h"

#include "Application.h"
#include "modules/Module.h"
#include "modules/Window.h"
#include "modules/Renderer.h"
#include "modules/GUI.h"
#include "modules/Input.h"
#include "modules/Editor.h"
#include "modules/TorchLoader.h"

Application::Application()
    : running(true), name("Image Editor")
{
    this->AddModule(this->input = new Input());
    this->AddModule(this->window = new Window("Image Editor", 1024, 768));
    this->AddModule(this->editor = new Editor());
    this->AddModule(this->torch_loader = new TorchLoader());
    this->AddModule(this->renderer = new Renderer());
    this->AddModule(this->gui = new GUI());
}

Application::~Application()
{
    for (auto item = this->modules.rbegin(); item != this->modules.rend(); ++item)
    {
        delete *item;
    }
}

void Application::Run()
{
    this->Start();

    while (this->running)
    {
        this->Update();
    }

    this->CleanUp();
}

void Application::Start()
{
    for (auto item = this->modules.begin(); item != this->modules.end(); ++item)
    {
        (*item)->Start();
    }
}

void Application::Update()
{
    Uint64 start = SDL_GetPerformanceCounter();

    for (auto item = this->modules.begin(); item != this->modules.end(); ++item)
    {
        (*item)->PreUpdate();
    }

    for (auto item = this->modules.begin(); item != this->modules.end(); ++item)
    {
        (*item)->Update();
    }

    for (auto item = this->modules.begin(); item != this->modules.end(); ++item)
    {
        (*item)->PostUpdate();
    }

    Uint64 end = SDL_GetPerformanceCounter();

    float elapsed_ms = (end - start) / (float)SDL_GetPerformanceFrequency();
    SDL_Delay(floor(16.666f - elapsed_ms));
}

void Application::CleanUp()
{
    for (auto item = this->modules.rbegin(); item != this->modules.rend(); ++item)
    {
        (*item)->CleanUp();
    }
}

void Application::RequestBrowser(const std::string& url) const
{
    ShellExecuteA(0, "open", url.c_str(), 0, 0, SW_SHOWNORMAL);
}

void Application::AddModule(Module* module)
{
    this->modules.push_back(module);
}