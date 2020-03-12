#include "Render/OpenGL/.Package.h"
#include "Render/RenderAPI.h"

void RenderAPI::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
{
    glViewport(x, y, width, height);
}

void RenderAPI::SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void RenderAPI::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderAPI::DrawIndexed(const SPtr<VertexArray> &vertexArray)
{
    glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}