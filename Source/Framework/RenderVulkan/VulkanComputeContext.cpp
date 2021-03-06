#include "RenderVulkan/VulkanComputeContext.h"
#include "RenderVulkan/VulkanBuffer.h"
#include "RenderVulkan/VulkanTexture.h"
#include "RenderVulkan/VulkanComputeStateObject.h"

SPtr<ComputeContext> ComputeContext::Create(const SPtr<GpuQueue> &queue)
{
    return Memory::MakeShared<VulkanComputeContext>(queue);
}

VulkanComputeContextImpl::VulkanComputeContextImpl(const SPtr<GpuQueue> &queue, ComputeContext *ctx)
    : VulkanCopyContextImpl(queue, ctx), computeContext(ctx)
{
}

VulkanComputeContextImpl::~VulkanComputeContextImpl()
{
}

void VulkanComputeContextImpl::Dispatch(ComputeState *state, ComputeVars *vars, const Vector3I &size)
{
    CT_CHECK(size.x >= 0 && size.y >= 0 && size.z >= 0);

    if (PrepareForDispatch(state, vars) == false)
        return;

    vkCmdDispatch(contextData->GetCommandBufferHandle(), size.x, size.y, size.z);
}

void VulkanComputeContextImpl::DispatchIndirect(ComputeState *state, ComputeVars *vars, const Buffer *argBuffer, uint64 argBufferOffset)
{
    if (PrepareForDispatch(state, vars) == false)
        return;

    ResourceBarrier(argBuffer, ResourceState::IndirectArg, nullptr);
    auto vkBuffer = static_cast<const VulkanBuffer *>(argBuffer);
    vkCmdDispatchIndirect(contextData->GetCommandBufferHandle(), vkBuffer->GetHandle(), argBuffer->GetGpuAddressOffset() + argBufferOffset);
}

void VulkanComputeContextImpl::ClearUav(const ResourceView *uav, const Vector4 &value)
{
    ClearColorImage(uav, value.x, value.y, value.z, value.w);
    commandsPending = true;
}

void VulkanComputeContextImpl::ClearUav(const ResourceView *uav, const Vector4U &value)
{
    auto vkBuffer = dynamic_cast<const VulkanBuffer *>(uav->GetResource());
    if (vkBuffer)
    {
        vkCmdFillBuffer(contextData->GetCommandBufferHandle(), vkBuffer->GetHandle(), vkBuffer->GetGpuAddressOffset(), vkBuffer->GetSize(), value.x);
    }
    else
    {
        ClearColorImage(uav, value.x, value.y, value.z, value.w);
    }

    commandsPending = true;
}

void VulkanComputeContextImpl::ClearUavCounter(const Buffer *buffer, uint32 value)
{
    auto uavCounter = buffer->GetUavCounter();
    if (uavCounter)
    {
        Vector4U uvec{ value, 0, 0, 0 };
        ClearUav(uavCounter->GetUav().get(), uvec);
    }
}

void VulkanComputeContextImpl::ClearColorImage(const ResourceView *view, float v0, float v1, float v2, float v3)
{
    auto vkTexture = dynamic_cast<const VulkanTexture *>(view->GetResource());
    CT_CHECK(vkTexture != nullptr);

    ResourceBarrier(vkTexture, ResourceState::CopyDest, nullptr);
    VkClearColorValue colVal = {};
    colVal.float32[0] = v0;
    colVal.float32[1] = v1;
    colVal.float32[2] = v2;
    colVal.float32[3] = v3;

    VkImageSubresourceRange range = {};
    const auto &viewInfo = view->GetViewInfo();
    range.baseArrayLayer = viewInfo.firstArraySlice;
    range.baseMipLevel = viewInfo.mostDetailedMip;
    range.layerCount = viewInfo.arrayLayers;
    range.levelCount = viewInfo.mipLevels;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdClearColorImage(contextData->GetCommandBufferHandle(), vkTexture->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &colVal, 1, &range);
}

void VulkanComputeContextImpl::ClearColorImage(const ResourceView *view, uint32 v0, uint32 v1, uint32 v2, uint32 v3)
{
    auto vkTexture = dynamic_cast<const VulkanTexture *>(view->GetResource());
    CT_CHECK(vkTexture != nullptr);

    ResourceBarrier(vkTexture, ResourceState::CopyDest, nullptr);
    VkClearColorValue colVal = {};
    colVal.uint32[0] = v0;
    colVal.uint32[1] = v1;
    colVal.uint32[2] = v2;
    colVal.uint32[3] = v3;

    VkImageSubresourceRange range = {};
    const auto &viewInfo = view->GetViewInfo();
    range.baseArrayLayer = viewInfo.firstArraySlice;
    range.baseMipLevel = viewInfo.mostDetailedMip;
    range.layerCount = viewInfo.arrayLayers;
    range.levelCount = viewInfo.mipLevels;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdClearColorImage(contextData->GetCommandBufferHandle(), vkTexture->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &colVal, 1, &range);
}

bool VulkanComputeContextImpl::ApplyComputeVars(ComputeVars *vars, RootSignature *rootSignature)
{
    if (vars->Apply(computeContext, rootSignature) == false)
    {
        CT_LOG(Warning, CT_TEXT("VulkanComputeContextImpl::ApplyComputeVars() call failed."));
        return false;
    }
    return true;
}

void VulkanComputeContextImpl::SetPipelineState(ComputeStateObject *cso)
{
    auto commandBuffer = contextData->GetCommandBufferHandle();
    auto handle = static_cast<VulkanComputeStateObject *>(cso)->GetHandle();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, handle);
}

bool VulkanComputeContextImpl::PrepareForDispatch(ComputeState *state, ComputeVars *vars)
{
    auto cso = state->GetCso(vars);

    if (vars)
    {
        if (!ApplyComputeVars(vars, cso->GetDesc().rootSignature.get()))
            return false;
    }

    SetPipelineState(cso.get());

    return true;
}