#include "Application/ImGuiLab.h"
#include "Render/RenderAPI.H"
#include "Render/OrthographicCamera.h"

ImGuiLab imguiLab;
ImGuiLab *gImGuiLab = &imguiLab;

void ImGuiLab::Startup()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    BindPlatform();
    BindRenderer();
}

void ImGuiLab::Shutdown()
{
    UnbindRenderer();
    UnbindPlatform();

    ImGui::DestroyContext();
}

void ImGuiLab::Tick()
{
    CT_PROFILE_FUNCTION();

    Begin();

    static bool open = false;
    ImGui::ShowDemoWindow(&open);

    drawHandler();

    End();
}

void ImGuiLab::BindPlatform()
{
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendPlatformName = "WIP";

    io.KeyMap[ImGuiKey_Tab] = CT_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = CT_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = CT_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = CT_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = CT_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = CT_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = CT_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = CT_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = CT_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = CT_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = CT_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = CT_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = CT_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = CT_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = CT_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = CT_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = CT_KEY_A;
    io.KeyMap[ImGuiKey_C] = CT_KEY_C;
    io.KeyMap[ImGuiKey_V] = CT_KEY_V;
    io.KeyMap[ImGuiKey_X] = CT_KEY_X;
    io.KeyMap[ImGuiKey_Y] = CT_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = CT_KEY_Z;

#if defined(_WIN32)
    io.ImeWindowHandle = gApp->GetWindow().GetNativeHandle();
#endif

    // TODO: Clipboard

    // WIP: Create mouse cursors

    // Input events

    KeyTypedHandle = gApp->GetInput().keyTypedHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(event.character);
    });

    keyUpHandle = gApp->GetInput().keyUpHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.KeysDown[event.key] = false;
        io.KeyCtrl = io.KeysDown[CT_KEY_LEFT_CONTROL] || io.KeysDown[CT_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[CT_KEY_LEFT_SHIFT] || io.KeysDown[CT_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[CT_KEY_LEFT_ALT] || io.KeysDown[CT_KEY_RIGHT_ALT];
#ifdef _WIN32
        io.KeySuper = false;
#else
        io.KeySuper = io.KeysDown[CT_KEY_LEFT_SUPER] || io.KeysDown[CT_KEY_RIGHT_SUPER];
#endif
    });

    keyDownHandle = gApp->GetInput().keyDownHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.KeysDown[event.key] = true;
        io.KeyCtrl = io.KeysDown[CT_KEY_LEFT_CONTROL] || io.KeysDown[CT_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[CT_KEY_LEFT_SHIFT] || io.KeysDown[CT_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[CT_KEY_LEFT_ALT] || io.KeysDown[CT_KEY_RIGHT_ALT];
#ifdef _WIN32
        io.KeySuper = false;
#else
        io.KeySuper = io.KeysDown[CT_KEY_LEFT_SUPER] || io.KeysDown[CT_KEY_RIGHT_SUPER];
#endif
    });

    touchUpHandle = gApp->GetInput().touchUpHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDown[event.button] = false;
    });

    touchDownHandle = gApp->GetInput().touchDownHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDown[event.button] = true;
    });

    mouseMovedHandle = gApp->GetInput().mouseMovedHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos = ImVec2((float)event.x, (float)event.y);
    });

    mouseScrolledHandle = gApp->GetInput().mouseScrolledHandler.On([](auto &event) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseWheel += (float)event.amount;
        //io.MouseWheelH += (float)e.amount;
    });
}

void ImGuiLab::BindRenderer()
{
    ImGuiIO &io = ImGui::GetIO();
    io.BackendRendererName = "WIP";

    shader = Shader::Create(CT_TEXT("Assets/Shaders/ImGuiShader.glsl"));

    vertexBuffer = VertexBuffer::Create(nullptr, 0, GpuBufferUsage::Dynamic);
    vertexBuffer->SetLayout({
        {CT_TEXT("VertexPosition"), VertexDataType::Float2},
        {CT_TEXT("VertexUV"), VertexDataType::Float2},
        {CT_TEXT("VertexColor"), VertexDataType::UByte4, true},
    });

    indexBuffer = IndexBuffer::Create(nullptr, 0, GpuBufferUsage::Dynamic);
    vertexArray = VertexArray::Create();
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);

    uchar8 *pixels;
    int32 width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    auto pixmap = Pixmap::Create(width, height, PixelFormat::RGBA8888, pixels);
    texture = Texture::Create(pixmap);

    //io.Fonts->TexID = 0;
}

void ImGuiLab::UnbindPlatform()
{
    KeyTypedHandle.Off();
    keyUpHandle.Off();
    keyDownHandle.Off();
    touchUpHandle.Off();
    touchDownHandle.Off();
    mouseMovedHandle.Off();
    mouseScrolledHandle.Off();
}

void ImGuiLab::UnbindRenderer()
{
    shader.reset();
    texture.reset();
}

void ImGuiLab::Begin()
{
    ImGuiIO &io = ImGui::GetIO();
    if (!io.Fonts->IsBuilt())
    {
        CT_EXCEPTION(ImGuiLab, "Font atlas not built!");
        return;
    }

    io.DeltaTime = gApp->GetDeltaTime();

    uint32 width = gApp->GetWindow().GetWidth();
    uint32 height = gApp->GetWindow().GetHeight();
    io.DisplaySize = ImVec2((float)width, (float)height);
    if (width > 0 && height > 0)
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // WIP
    // UpdateMouseCursor
    // UpdateGamepads

    ImGui::NewFrame();
}

void ImGuiLab::End()
{
    ImGui::Render();

    RenderDrawData(ImGui::GetDrawData());

    uint32 width = gApp->GetWindow().GetWidth();
    uint32 height = gApp->GetWindow().GetHeight();

    RenderAPI::SetViewport(0, 0, width, height);
    RenderAPI::SetScissor(0, 0, width, height);
}

void ImGuiLab::SetupRenderState(ImDrawData *drawData, uint32 width, uint32 height)
{
    RenderAPI::SetViewport(0, 0, width, height);

    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

    Matrix4 mvp = Matrix4::Ortho(L, R, B, T, -1.0f, 1.0f);

    shader->Bind();
    texture->Bind(0);
    shader->SetInt(CT_TEXT("MainTex"), 0);
    shader->SetMatrix4(CT_TEXT("MVP"), mvp);
}

void ImGuiLab::RenderDrawData(ImDrawData *drawData)
{
    int32 width = (int32)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int32 height = (int32)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (width <= 0 || height <= 0)
        return;

    SetupRenderState(drawData, width, height);

    ImVec2 clip_off = drawData->DisplayPos;
    ImVec2 clip_scale = drawData->FramebufferScale;

    for (int32 n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList *cmd_list = drawData->CmdLists[n];

        vertexBuffer->SetData((float *)cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        indexBuffer->SetData((uint32 *)cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size);

        for (int32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback != NULL)
            {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(drawData, width, height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < width && clip_rect.y < height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    RenderAPI::SetScissor((int32)clip_rect.x, (int32)(height - clip_rect.w), (int32)(clip_rect.z - clip_rect.x), (int32)(clip_rect.w - clip_rect.y));

                    //glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                    vertexArray->Bind();
                    RenderAPI::DrawIndexed(pcmd->IdxOffset, pcmd->ElemCount);
                    vertexArray->Unbind();
                }
            }
        }
    }
}