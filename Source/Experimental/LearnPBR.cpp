#include "Application/Application.h"
#include "IO/FileHandle.h"
#include "Render/CameraController.h"
#include "Render/Importers/SceneImporter.h"
#include "Render/Importers/TextureImporter.h"
#include "Render/MeshGenerator.h"
#include "Render/RenderManager.h"
#include "RenderCore/RenderAPI.h"
#include "Render/RenderPasses/FullScreenPass.h"

class Renderer
{
public:
    SPtr<CameraController> cameraController;

    SPtr<GraphicsState> state;
    SPtr<VertexArray> vao;
    SPtr<RasterizationState> rasterizationState;
    SPtr<DepthStencilState> depthStencilState;
    SPtr<BlendState> blendState;

    SPtr<Program> program;
    SPtr<GraphicsVars> vars;
    SPtr<Program> reflectionProgram;

    SPtr<FullScreenPass> testPass;

    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
    };

    Array<Vertex> vertices;
    Array<uint32> indices;

    Array<Vector3> lightPositions;
    Array<Vector3> lightColors;

    static constexpr int32 ROWS = 7;
    static constexpr int32 COLS = 7;

    void LoadModel()
    {
        MeshGenerator gen;
        //MeshData meshData = gen.CreateBox(1.0f, 1.0f, 1.0f, 1);
        MeshData meshData = gen.CreateSphere(1.0f, 6);

        vertices.SetCount(meshData.vertices.Count());
        for (int32 i = 0; i < meshData.vertices.Count(); ++i)
        {
            vertices[i].position = meshData.vertices[i].position;
            vertices[i].normal = meshData.vertices[i].normal;
        }
        indices = meshData.indices;
    }

    void InitLights()
    {
        lightPositions = {
            Vector3(-10.0f, 10.0f, 10.0f),
            Vector3(10.0f, 10.0f, 10.0f),
            Vector3(-10.0f, -10.0f, 10.0f),
            Vector3(10.0f, -10.0f, 10.0f)
        };
        lightColors = {
            Vector3(300.0f, 300.0f, 300.0f),
            Vector3(300.0f, 300.0f, 300.0f),
            Vector3(300.0f, 300.0f, 300.0f),
            Vector3(300.0f, 300.0f, 300.0f)
        };
    }

    void CreateObjectBuffer()
    {
    }

public:
    Renderer()
    {
        LoadModel();
        InitLights();
        CreateObjectBuffer();

        auto &window = gApp->GetWindow();
        float width = window.GetWidth();
        float height = window.GetHeight();
        auto camera = Camera::Create();
        auto controller = OrbiterCameraController::Create(camera);
        controller->SetModelParams(Vector3(), 1.0f, 15.0f);
        //auto controller = FirstPersonCameraController::Create(camera);
        //auto controller = SixDOFCameraController::Create(camera);
        controller->SetViewport(width, height);
        cameraController = controller;

        program = Program::Create(CT_TEXT("Assets/Shaders/Experimental/LearnPBR.glsl"));
        vars = GraphicsVars::Create(program);

        state = GraphicsState::Create();

        auto vertexBufferLayout = VertexBufferLayout::Create(
            { { 0, CT_TEXT("VertexPosition"), ResourceFormat::RGB32Float },
              { 1, CT_TEXT("VertexNormal"), ResourceFormat::RGB32Float } });

        auto vertexLayout = VertexLayout::Create();
        vertexLayout->AddBufferLayout(vertexBufferLayout);

        auto vbo = Buffer::Create(sizeof(Vertex) * vertices.Count(), ResourceBind::Vertex, BufferCpuAccess::None, vertices.GetData());
        auto ibo = Buffer::Create(sizeof(uint32) * indices.Count(), ResourceBind::Index, BufferCpuAccess::None, indices.GetData());

        vao = VertexArray::Create();
        vao->SetVertexLayout(vertexLayout);
        vao->AddVertexBuffer(vbo);
        vao->SetIndexBuffer(ibo);

        {
            RasterizationStateDesc desc;
            desc.frontCCW = true;
            desc.polygonMode = PolygonMode::Fill;
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

        state->SetProgram(program);
        state->SetVertexArray(vao);
        state->SetRasterizationState(rasterizationState);
        state->SetDepthStencilState(depthStencilState);
        state->SetBlendState(blendState);


        // Full screen pass
        testPass = FullScreenPass::Create(CT_TEXT("Assets/Shaders/Experimental/FullScreenTest.glsl"));
    }

    void Render(RenderContext *ctx, const SPtr<FrameBuffer> &fbo)
    {
        state->SetFrameBuffer(fbo);

        cameraController->Update();
        Matrix4 viewProj = cameraController->GetCamera()->GetViewProjection();
        vars->Root()["FrameUB"]["viewProj"] = viewProj;
        vars->Root()["FrameUB"]["camPos"] = cameraController->GetCamera()->GetPosition();
        for (int32 i = 0; i < lightPositions.Count(); ++i)
        {
            vars->Root()["FrameUB"]["lightPositions"][i] = lightPositions[i];
            vars->Root()["FrameUB"]["lightColors"][i] = lightColors[i];
        }

        for (int32 r = 0; r < ROWS; ++r)
        {
            for (int32 c = 0; c < COLS; ++c)
            {
                auto var = vars->Root()["ObjectUB"]["objects"][r * COLS + c];
                var["ao"] = 1.0f;
                var["albedo"] = Vector3(0.5f, 0.0f, 0.0f);
                var["metallic"] = (float)r / (float)ROWS;
                var["roughness"] = Math::Clamp((float)c / (float)COLS, 0.05f, 1.0f);
                float x = (c - (COLS / 2)) * 2.5f;
                float y = (r - (ROWS / 2)) * 2.5f;
                Matrix4 model = Matrix4::Translate(x, y, 0.0f);
                var["model"] = model;
            }
        }

        ctx->DrawIndexedInstanced(state.get(), vars.get(), indices.Count(), ROWS * COLS, 0, 0, 0);

        //testPass->Execute(ctx, fbo);
    }
};

class LearnPBR : public Logic
{
private:
    SPtr<Renderer> renderer;

public:
    virtual void Startup() override
    {
        renderer = Memory::MakeShared<Renderer>();
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
        renderer->cameraController->SetViewport((float)e.width, (float)e.height);
    }

    virtual void OnKeyDown(KeyDownEvent &e) override
    {
        renderer->cameraController->OnKeyDown(e);
    }

    virtual void OnKeyUp(KeyUpEvent &e) override
    {
        renderer->cameraController->OnKeyUp(e);
    }

    virtual void OnTouchDown(TouchDownEvent &e) override
    {
        renderer->cameraController->OnTouchDown(e);
    }

    virtual void OnTouchUp(TouchUpEvent &e) override
    {
        renderer->cameraController->OnTouchUp(e);
    }

    virtual void OnMouseMoved(MouseMovedEvent &e) override
    {
        renderer->cameraController->OnMouseMoved(e);
    }

    virtual void OnMouseScrolled(MouseScrolledEvent &e) override
    {
        renderer->cameraController->OnMouseScrolled(e);
    }
};

LearnPBR logic;
Logic *gLogic = &logic;