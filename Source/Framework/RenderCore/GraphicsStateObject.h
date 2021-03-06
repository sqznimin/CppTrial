#pragma once

#include "RenderCore/BlendState.h"
#include "RenderCore/DepthStencilState.h"
#include "RenderCore/FrameBuffer.h"
#include "RenderCore/Program.h"
#include "RenderCore/RasterizationState.h"
#include "RenderCore/RootSignature.h"
#include "RenderCore/VertexArray.h"

struct GraphicsStateObjectDesc
{
    SPtr<ProgramKernel> programKernel;
    SPtr<VertexLayout> vertexLayout;
    SPtr<RasterizationState> rasterizationState;
    SPtr<DepthStencilState> depthStencilState;
    SPtr<BlendState> blendState;
    SPtr<RootSignature> rootSignature;

    FrameBufferDesc frameBufferDesc;
    Topology topology = Topology::TriangleList;
    uint32 sampleMask = UINT32_MAX;

    bool operator==(const GraphicsStateObjectDesc &other) const
    {
        if (programKernel != other.programKernel)
            return false;
        if (vertexLayout != other.vertexLayout)
            return false;
        if (rasterizationState != other.rasterizationState)
            return false;
        if (depthStencilState != other.depthStencilState)
            return false;
        if (blendState != other.blendState)
            return false;
        if (rootSignature != other.rootSignature)
            return false;

        if (topology != other.topology)
            return false;
        if (sampleMask != other.sampleMask)
            return false;
        if (frameBufferDesc != other.frameBufferDesc)
            return false;
        return true;
    }

    bool operator!=(const GraphicsStateObjectDesc &other) const
    {
        return !(*this == other);
    }
};

class GraphicsStateObject
{
public:
    virtual ~GraphicsStateObject() = default;

    const GraphicsStateObjectDesc &GetDesc() const
    {
        return desc;
    }

    static SPtr<GraphicsStateObject> Create(const GraphicsStateObjectDesc &desc);

protected:
    GraphicsStateObject(const GraphicsStateObjectDesc &inDesc)
        : desc(inDesc)
    {
        if (!desc.blendState)
            desc.blendState = BlendState::Create({});
        if (!desc.depthStencilState)
            desc.depthStencilState = DepthStencilState::Create({});
        if (!desc.rasterizationState)
            desc.rasterizationState = RasterizationState::Create({});
    }

protected:
    GraphicsStateObjectDesc desc;
};