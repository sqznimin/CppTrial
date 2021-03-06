#pragma once

#include "RenderCore/GpuFence.h"

struct DescriptorPoolDesc
{
    int32 descCount[(int32)DescriptorType::Count] = { 0 };
};

class DescriptorPool
{
public:
    DescriptorPool(const DescriptorPoolDesc &desc, const SPtr<GpuFence> &fence)
        : desc(desc), fence(fence)
    {
    }

    virtual ~DescriptorPool() = default;

    const DescriptorPoolDesc &GetDesc() const
    {
        return desc;
    }

    static SPtr<DescriptorPool> Create(const DescriptorPoolDesc &desc, const SPtr<GpuFence> &fence);

protected:
    DescriptorPoolDesc desc;
    SPtr<GpuFence> fence;
};