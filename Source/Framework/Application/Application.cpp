#include "Application/Application.h"
#include "Application/ThreadManager.h"
#include "Assets/AssetManager.h"
#include "Core/Time.h"
#include "Render/RenderManager.h"
#include "Render/Importers/SceneImporter.h"
#include "Render/Importers/TextureImporter.h"
#include "Render/Exporters/TextureExporter.h"
#include "Application/ImGuiLab.h"
#include "Application/DebugManager.h"

void Application::PreInit(const WindowDesc &desc)
{
    auto time = Time::NanoTime();
    startTime = time;
    frameCountTime = time;

    gDebugManager->Startup();
    CT_LOG(Info, CT_TEXT("DebugManager startup."));

    gThreadManager->Startup();
    CT_LOG(Info, CT_TEXT("ThreadManager startup."));

    Window::Init();
    window = Window::Create(desc);
    window->Startup();
    CT_LOG(Info, CT_TEXT("Primary window created."));

    Input::Init();
    Clipboard::Init();
}

void Application::Init()
{
    gRenderManager->Startup();
    CT_LOG(Info, CT_TEXT("RenderManager startup."));

    gAssetManager->Startup();
    CT_LOG(Info, CT_TEXT("AssetManager startup."));

    gImGuiLab->Startup();
    CT_LOG(Info, CT_TEXT("ImGuiLab startup."));

    gLogic->Startup();

    gAssetManager->RegisterImporter<Texture>(Memory::New<TextureImporter>());
    gAssetManager->RegisterImporter<Scene>(Memory::New<SceneImporter>());
    gAssetManager->RegisterExporter<Texture>(Memory::New<TextureExporter>());

    GetWindow().windowResizedHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnWindowResized(e);
    });

    GetInput().keyTypedHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnKeyTyped(e);
    });
    GetInput().keyDownHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnKeyDown(e);
    });
    GetInput().keyUpHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnKeyUp(e);
    });
    GetInput().touchDownHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnTouchDown(e);
    });
    GetInput().touchUpHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnTouchUp(e);
    });
    GetInput().mouseMovedHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnMouseMoved(e);
    });
    GetInput().mouseScrolledHandler += ([](auto &e) {
        if (e.handled)
            return;
        gLogic->OnMouseScrolled(e);
    });

    CT_LOG(Info, CT_TEXT("Logic startup."));
}

void Application::Run()
{
    while (!requestingQuit)
    {
        auto time = Time::NanoTime();
        auto delta = time - lastTime < 100 ? 100 : time - lastTime; //TODO Sleep?
        deltaTime = lastTime > 0 ? delta / 1000000000.0f : 0.0f;
        lastTime = time;
        if (time - frameCountTime >= 1000000000)
        {
            frameCountTime = time;
            fps = frames;
            frames = 0;
        }
        ++frames;
        ++totalFrames;

        //CT_PROFILE_SESSION_BEGIN(CT_TEXT("ThreadManager"));
        gThreadManager->Tick();

        CT_PROFILE_SESSION_BEGIN(CT_TEXT("AssetManager"));
        gAssetManager->Tick();

        CT_PROFILE_SESSION_BEGIN(CT_TEXT("Logic"));
        gLogic->Tick();

        CT_PROFILE_SESSION_BEGIN(CT_TEXT("RenderManager"));
        gRenderManager->Tick();

        CT_PROFILE_SESSION_BEGIN(CT_TEXT("ImGuiLab"));
        gImGuiLab->Tick();

        CT_PROFILE_SESSION_BEGIN(CT_TEXT("Window"));
        window->Tick();

        CT_PROFILE_SESSION_BEGIN(CT_TEXT("RenderManager"));
        gRenderManager->Present();

        //CT_PROFILE_SESSION_BEGIN(CT_TEXT("DebugManager"));
        gDebugManager->Tick();

        if (window->ShouldClose())
            RequestQuit();
    }
}

void Application::Exit()
{
    gLogic->Shutdown();
    CT_LOG(Info, CT_TEXT("Logic shutdown."));

    gImGuiLab->Shutdown();
    CT_LOG(Info, CT_TEXT("ImGui shutdown."));

    gRenderManager->Shutdown();
    CT_LOG(Info, CT_TEXT("RenderManager shutdown."));

    window->Shutdown();
    CT_LOG(Info, CT_TEXT("Primary window shutdown."));

    gThreadManager->Shutdown();
    CT_LOG(Info, CT_TEXT("ThreadManager shutdown."));

    gDebugManager->Shutdown();
    CT_LOG(Info, CT_TEXT("DebugManager shutdown."));
}

Input &Application::GetInput()
{
    return window->GetInput();
}

Clipboard &Application::GetClipboard()
{
    return *gClipboard;
}