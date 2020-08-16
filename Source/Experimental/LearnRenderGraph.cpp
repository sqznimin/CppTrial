#include "Application/Application.h"
#include "Application/ImGuiLab.h"
#include "Assets/AssetManager.h"
#include "Experimental/Widgets/CameraView.h"
#include "Experimental/Widgets/ImageWindow.h"
#include "Experimental/Widgets/MaterialView.h"
#include "IO/FileHandle.h"
#include "Render/Importers/SceneImporter.h"
#include "Render/Importers/TextureImporter.h"
#include "Render/RenderManager.h"
#include "RenderCore/RenderAPI.h"

#include "Experimental/Widgets/ProfileWindow.h"

class Renderer
{
public:
    SPtr<GraphicsState> state;
    SPtr<RasterizationState> rasterizationState;
    SPtr<DepthStencilState> depthStencilState;
    SPtr<BlendState> blendState;
    SPtr<Sampler> sampler;
    SPtr<Program> program;
    SPtr<GraphicsVars> vars;

    SPtr<Scene> scene;
    SPtr<CameraController> cameraController;
    float sceneUpdateTime = 0.0f;

public:
    Renderer()
    {
        state = GraphicsState::Create();

        {
            RasterizationStateDesc desc;
            desc.cullMode = CullMode::None;
            desc.polygonMode = PolygonMode::Wireframe;
            rasterizationState = RasterizationState::Create(desc);
        }
        {
            DepthStencilStateDesc desc;
            desc.depthReadEnabled = true;
            desc.depthWriteEnabled = true;
            depthStencilState = DepthStencilState::Create(desc);
        }
        {
            BlendStateDesc desc;
            desc.attachments.SetCount(1);
            desc.attachments[0].enabled = true;
            blendState = BlendState::Create(desc);
        }

        state->SetRasterizationState(rasterizationState);
        state->SetDepthStencilState(depthStencilState);
        state->SetBlendState(blendState);
    }

    void LoadScene(const String &path)
    {
        auto &window = gApp->GetWindow();
        float width = window.GetWidth();
        float height = window.GetHeight();

        auto settings = SceneImportSettings::Create();
        settings->dontLoadBones = true;
        scene = gAssetManager->Import<Scene>(path, settings);

        if (scene)
        {
            sceneUpdateTime = 0.0f;

            cameraController = FirstPersonCameraController::Create(scene->GetCamera());
            cameraController->SetViewport(width, height);
            scene->SetCameraController(cameraController);

            program = Program::Create(CT_TEXT("Assets/Shaders/Experimental/LearnVK.glsl"), scene->GetSceneDefines());
            vars = GraphicsVars::Create(program);
            state->SetProgram(program);
        }
    }

    void Render(RenderContext *ctx, const SPtr<FrameBuffer> &fbo)
    {
        if (!scene)
            return;

        sceneUpdateTime += gApp->GetDeltaTime();

        state->SetFrameBuffer(fbo);
        scene->Update(ctx, sceneUpdateTime);

        SceneRenderFlags flags = SceneRender::None;
        scene->Render(ctx, state.get(), vars.get(), flags);
    }
};

class LearnRenderGraph : public Logic
{
private:
    SPtr<Renderer> renderer;
    ImGui::FileBrowser modelFileDialog;
    ImageWindow imageWindow;
    CameraView cameraView;
    MaterialView materialView;

    Array<Profiler::SessionData> sessions;
    ProfileWindow profileWindow;

public:
    virtual void Startup() override
    {
        renderer = Memory::MakeShared<Renderer>();

        gImGuiLab->drawHandler.On([this]() {
            ImGui::Begin("ModelViewer");

            ImGuiIO &io = ImGui::GetIO();
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Separator();

            if (ImGui::Button("Open model file"))
                modelFileDialog.Open();

            modelFileDialog.Display();

            if (modelFileDialog.HasSelected())
            {
                renderer->LoadScene(modelFileDialog.GetSelected().c_str());
                modelFileDialog.ClearSelected();
            }

            ImGui::Separator();
            cameraView.SetScene(renderer->scene);
            cameraView.OnGui();

            ImGui::Separator();
            materialView.SetScene(renderer->scene);
            materialView.OnGui();

            ImGui::End();

            imageWindow.OnGui();

            profileWindow.AddFrameData(sessions);
            sessions.Clear();
            profileWindow.OnGui();
        });

        auto &gProfiler = Profiler::GetGlobalProfiler();
        gProfiler.sessionEndEventHandler.On([this](const auto &data) {
            sessions.Add(data);
            //CT_LOG(Debug, CT_TEXT("Session end, name:{0}, elapsed:{1}, frameID:{2}"), data.name, data.elapsedTime, gApp->GetTotalFrames());
        });
    }

    virtual void Shutdown() override
    {
        renderer.reset();
    }

    virtual void Tick() override
    {
        renderer->Render(gRenderManager->GetRenderContext(), gRenderManager->GetTargetFrameBuffer());
    }

    virtual void OnWindowResized(WindowResizedEvent &e) override
    {
        if (!renderer->scene)
            return;

        renderer->cameraController->SetViewport((float)e.width, (float)e.height);
    }

    virtual void OnKeyDown(KeyDownEvent &e) override
    {
        if (!renderer->scene)
            return;

        renderer->cameraController->OnKeyDown(e);

        if (e.key == CT_KEY_R)
        {
            renderer->scene->ResetCamera();
        }
    }

    virtual void OnKeyUp(KeyUpEvent &e) override
    {
        if (!renderer->scene)
            return;
        renderer->cameraController->OnKeyUp(e);
    }

    virtual void OnTouchDown(TouchDownEvent &e) override
    {
        if (!renderer->scene)
            return;
        renderer->cameraController->OnTouchDown(e);
    }

    virtual void OnTouchUp(TouchUpEvent &e) override
    {
        if (!renderer->scene)
            return;
        renderer->cameraController->OnTouchUp(e);
    }

    virtual void OnMouseMoved(MouseMovedEvent &e) override
    {
        if (!renderer->scene)
            return;
        renderer->cameraController->OnMouseMoved(e);
    }

    virtual void OnMouseScrolled(MouseScrolledEvent &e) override
    {
        if (!renderer->scene)
            return;
        renderer->cameraController->OnMouseScrolled(e);
    }
};

LearnRenderGraph logic;
Logic *gLogic = &logic;