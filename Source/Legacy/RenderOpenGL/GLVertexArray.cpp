#include "RenderOpenGL/GLVertexArray.h"

SPtr<VertexArray> VertexArray::Create()
{
    return Memory::MakeShared<GLVertexArray>();
}

GLVertexArray::GLVertexArray()
{
    glCreateVertexArrays(1, &id);
}

GLVertexArray::~GLVertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void GLVertexArray::Bind() const
{
    glBindVertexArray(id);
}

void GLVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void GLVertexArray::AddVertexBuffer(const SPtr<VertexBuffer> &buffer)
{
    Bind();
    buffer->Bind();

    const auto &layout = buffer->GetLayout();
    for (const auto &attr : layout.GetAttributes())
    {
        glEnableVertexAttribArray(vertexBufferIndex);
        glVertexAttribPointer(
            vertexBufferIndex,
            Render::GetVertexDataComponentCount(attr.dataType),
            Render::GetGLDataType(attr.dataType),
            attr.normalized ? GL_TRUE : GL_FALSE,
            layout.GetStride(),
            reinterpret_cast<void *>(attr.offset));

        ++vertexBufferIndex;
    }

    vertexBuffers.Add(buffer);

    Unbind();
    buffer->Unbind();
}

void GLVertexArray::SetIndexBuffer(const SPtr<IndexBuffer> &buffer)
{
    Bind();
    buffer->Bind();

    indexBuffer = buffer;

    Unbind();
    buffer->Unbind();
}