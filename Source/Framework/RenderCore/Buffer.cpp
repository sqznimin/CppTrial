#include "RenderCore/Buffer.h"

SPtr<Buffer> Buffer::CreateTyped(ResourceFormat format, int32 count, ResourceBindFlags bindFlags, BufferCpuAccess access, const void *data)
{
    uint32 size = count * GetResourceFormatBytes(format);
    auto buffer = Create(size, bindFlags, access, data);
    buffer->format = format;
    buffer->elementCount = count;
    return buffer;
}

SPtr<Buffer> Buffer::CreateStructured(uint32 structSize, int32 count, ResourceBindFlags bindFlags, BufferCpuAccess access, const void *data, bool createCounter)
{
    uint32 size = structSize * count;
    auto buffer = Create(size, bindFlags, access, data);
    buffer->elementCount = count;
    buffer->structSize = structSize;

    const uint32 zeroVal = 0;
    if (createCounter)
    {
        buffer->uavCounter = Create(sizeof(uint32), ResourceBind::UnorderedAccess, BufferCpuAccess::None, &zeroVal);
    }
    return buffer;
}

SPtr<Buffer> Buffer::CreateIndirect(const Array<DrawIndirectArgs> &args)
{
    uint32 size = args.Count() * sizeof(DrawIndirectArgs);
    auto buffer = Create(size, ResourceBind::IndirectArg, BufferCpuAccess::None, args.GetData());
    return buffer;
}

SPtr<Buffer> Buffer::CreateIndirect(const Array<DrawIndexedIndirectArgs> &args)
{
    uint32 size = args.Count() * sizeof(DrawIndexedIndirectArgs);
    auto buffer = Create(size, ResourceBind::IndirectArg, BufferCpuAccess::None, args.GetData());
    return buffer;
}

void Buffer::ClearViews()
{
    srvs.Clear();
    uavs.Clear();
    cbv = nullptr;
}

SPtr<ResourceView> Buffer::GetCbv()
{
    if (!cbv)
        cbv = ResourceView::CreateCbv(weakThis.lock());
    return cbv;
}

SPtr<ResourceView> Buffer::GetSrv(int32 first, int32 count)
{
    auto viewInfo = ResourceViewInfo(first, count);
    for (const auto &e : srvs)
    {
        if (e->GetViewInfo() == viewInfo)
            return e;
    }
    auto newView = ResourceView::CreateSrv(weakThis.lock(), first, count);
    srvs.Add(newView);
    return newView;
}

SPtr<ResourceView> Buffer::GetUav(int32 first, int32 count)
{
    auto viewInfo = ResourceViewInfo(first, count);
    for (const auto &e : uavs)
    {
        if (e->GetViewInfo() == viewInfo)
            return e;
    }
    auto newView = ResourceView::CreateUav(weakThis.lock(), first, count);
    uavs.Add(newView);
    return newView;
}