#pragma once

#include "RenderCore/.Package.h"

namespace RenderCore
{

struct UniformBufferCreateParams
{
    uint32 size;
    GpuBufferUsage usage = GpuBufferUsage::Dynamic;
};

class UniformBuffer
{
public:
    virtual ~UniformBuffer() = default;

    virtual void *Map() = 0;
    virtual void Unmap() = 0;
    virtual uint32 GetSize() const = 0;
    virtual GpuBufferUsage GetUsage() const = 0;

    static SPtr<UniformBuffer> Create(const UniformBufferCreateParams &params);
};

}