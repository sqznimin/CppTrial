#pragma once

#include "Core/List.h"
#include "RenderCore/DescriptorPool.h"
#include "RenderCore/FrameBuffer.h"
#include "RenderCore/GpuFence.h"
#include "RenderCore/RenderContext.h"
#include "RenderCore/RenderWindow.h"
#include "RenderCore/Resource.h"
#include "RenderCore/QueryPool.h"

class DeferredReleaser
{
public:
    struct ReleaseData
    {
        uint64 fenceValue;
        Runnable<> releaseFunc;
    };

    void AddReleaseData(Runnable<> func, uint64 fenceValue)
    {
        releaseDatas.Add({ fenceValue, std::move(func) });
    }

    void Release(uint64 fenceValue)
    {
        while (releaseDatas.Count() && releaseDatas.First().fenceValue <= fenceValue)
        {
            releaseDatas.First().releaseFunc();
            releaseDatas.RemoveFirst();
        }
    }

private:
    List<ReleaseData> releaseDatas;
};

struct DeviceDesc
{
    ResourceFormat colorFormat = ResourceFormat::BGRA8Unorm; //ResourceFormat::BGRA8UnormSrgb;
    ResourceFormat depthFormat = ResourceFormat::D32Float;
    bool vsync = true;
};

class Device
{
public:
    Device(RenderWindow *window, const DeviceDesc &desc)
        : window(window), desc(desc)
    {
    }

    virtual ~Device() = default;

    virtual void ResizeSwapChain(int32 width, int32 height) = 0;
    virtual void Present() = 0;
    virtual void FlushAndSync() = 0;

    const DeviceDesc &GetDesc() const
    {
        return desc;
    }

    const SPtr<DescriptorPool> &GetCpuDescriptorPool() const
    {
        return cpuDescriptorPool;
    }

    const SPtr<DescriptorPool> &GetGpuDescriptorPool() const
    {
        return gpuDescriptorPool;
    }

    const SPtr<QueryPool> &GetQueryPool(QueryType queryType) const
    {
        return queryPools[(int32)queryType];
    }

    float GetTimestampPeriod() const
    {
        return timestampPeriod;
    }

    SPtr<FrameBuffer> GetSwapChainFrameBuffer() const
    {
        return swapChainFrameBuffers[curBackBufferIndex];
    }

    RenderContext *GetRenderContext() const
    {
        return renderContext.get();
    }

    void Release(Runnable<> func)
    {
        deferredReleaser.AddReleaseData(std::move(func), frameFence->GetCpuValue());
    }

    static SPtr<Device> Create(RenderWindow *window, const DeviceDesc &desc);

protected:
    void ExecuteDeferredRelease()
    {
        deferredReleaser.Release(frameFence->GetGpuValue());
    }

protected:
    RenderWindow *window;
    DeviceDesc desc;

    SPtr<GpuFence> frameFence;
    DeferredReleaser deferredReleaser;
    SPtr<RenderContext> renderContext;

    SPtr<DescriptorPool> cpuDescriptorPool;
    SPtr<DescriptorPool> gpuDescriptorPool;

    SPtr<QueryPool> queryPools[(int32)QueryType::Count];
    float timestampPeriod;

    int32 backBufferCount;
    Array<SPtr<FrameBuffer>> swapChainFrameBuffers;
    int32 curBackBufferIndex = 0;
};