#include "RenderVulkan/.Package.h"
#include "RenderVulkan/VulkanDevice.h"

VkImageLayout ToVkImageLayout(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case ResourceState::PreInitialized:
        return VK_IMAGE_LAYOUT_PREINITIALIZED;
    case ResourceState::Common:
    case ResourceState::UnorderedAccess:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::RenderTarget:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthStencil:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::ShaderResource:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::ResolveDest:
    case ResourceState::CopyDest:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::ResolveSource:
    case ResourceState::CopySource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported resource state!");
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

VkAccessFlags ToVkAccess(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:
    case ResourceState::Present:
    case ResourceState::Common:
    case ResourceState::PreInitialized:
        return 0;
    case ResourceState::VertexBuffer:
        return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case ResourceState::ConstantBuffer:
        return VK_ACCESS_UNIFORM_READ_BIT;
    case ResourceState::IndexBuffer:
        return VK_ACCESS_INDEX_READ_BIT;
    case ResourceState::RenderTarget:
        return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    case ResourceState::UnorderedAccess:
        return VK_ACCESS_SHADER_WRITE_BIT;
    case ResourceState::DepthStencil:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case ResourceState::ShaderResource:
        return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    case ResourceState::IndirectArg:
        return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case ResourceState::ResolveDest:
    case ResourceState::CopyDest:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    case ResourceState::ResolveSource:
    case ResourceState::CopySource:
        return VK_ACCESS_TRANSFER_READ_BIT;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported resource state!");
        return 0;
    }
}

VkPipelineStageFlags ToVkPipelineStage(ResourceState state, bool src)
{
    switch (state)
    {
    case ResourceState::Undefined:
    case ResourceState::PreInitialized:
    case ResourceState::Common:
        CT_CHECK(src);
        return src ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    case ResourceState::VertexBuffer:
    case ResourceState::IndexBuffer:
        return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case ResourceState::UnorderedAccess:
    case ResourceState::ConstantBuffer:
    case ResourceState::ShaderResource:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case ResourceState::RenderTarget:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ResourceState::DepthStencil:
        return src ? VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    case ResourceState::IndirectArg:
        return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    case ResourceState::CopyDest:
    case ResourceState::CopySource:
    case ResourceState::ResolveDest:
    case ResourceState::ResolveSource:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case ResourceState::Present:
        return src ? (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported resource state!");
        return 0;
    }
}

VkShaderStageFlagBits ToVkShaderStage(ShaderType shaderType)
{
    switch (shaderType)
    {
    case ShaderType::Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderType::Pixel:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderType::Geometry:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderType::Hull:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderType::Domain:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderType::Compute:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported shader type!");
        return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

VkShaderStageFlags ToVkShaderVisibility(ShaderVisibilityFlags visibility)
{
    VkShaderStageFlags flags = 0;

    if (visibility & ShaderVisibility::Vertex)
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (visibility & ShaderVisibility::Pixel)
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (visibility & ShaderVisibility::Hull)
        flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    ;
    if (visibility & ShaderVisibility::Domain)
        flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (visibility & ShaderVisibility::Geometry)
        flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (visibility & ShaderVisibility::Compute)
        flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    return flags;
}

VkDescriptorType ToVkDescriptorType(DescriptorType descType)
{
    switch (descType)
    {
    case DescriptorType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case DescriptorType::TextureSrv:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case DescriptorType::TextureUav:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case DescriptorType::RawBufferSrv:
    case DescriptorType::TypedBufferSrv:
        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case DescriptorType::RawBufferUav:
    case DescriptorType::TypedBufferUav:
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case DescriptorType::Cbv:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::StructuredBufferSrv:
    case DescriptorType::StructuredBufferUav:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::Dsv:
    case DescriptorType::Rtv:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported desc type!");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

VkQueryType ToVkQueryType(QueryType queryType)
{
    switch (queryType)
    {
    case QueryType::Timestamp:
        return VK_QUERY_TYPE_TIMESTAMP;
    case QueryType::Occlusion:
        return VK_QUERY_TYPE_OCCLUSION;
    case QueryType::PipelineStats:
        return VK_QUERY_TYPE_PIPELINE_STATISTICS;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported query type!");
        return VK_QUERY_TYPE_MAX_ENUM;
    }
}

VkMemoryPropertyFlags ToVkMemoryProperty(MemoryUsage usage)
{
    switch (usage)
    {
    case MemoryUsage::Default:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    case MemoryUsage::Upload:
        return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    case MemoryUsage::Download:
        return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }
    return 0;
}

VkBufferUsageFlags ToVkBufferUsage(ResourceBindFlags bind)
{
    VkBufferUsageFlags flags = 0;
    if (bind & ResourceBind::Vertex)
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (bind & ResourceBind::Index)
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (bind & ResourceBind::UnorderedAccess)
        flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (bind & ResourceBind::ShaderResource)
        flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    if (bind & ResourceBind::IndirectArg)
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    if (bind & ResourceBind::Constant)
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    return flags;
}

VkImageType ToVkImageType(ResourceType resourceType)
{
    switch (resourceType)
    {
    case ResourceType::Texture1D:
        return VK_IMAGE_TYPE_1D;
    case ResourceType::Texture2D:
    case ResourceType::Texture2DMultisample:
    case ResourceType::TextureCube:
        return VK_IMAGE_TYPE_2D;
    case ResourceType::Texture3D:
        return VK_IMAGE_TYPE_3D;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported image type.");
        return VK_IMAGE_TYPE_2D;
    }
}

VkImageUsageFlags ToVkImageUsage(ResourceBindFlags bind)
{
    VkImageUsageFlags flags = 0;
    if (bind & ResourceBind::UnorderedAccess)
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (bind & ResourceBind::ShaderResource)
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (bind & ResourceBind::DepthStencil)
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (bind & ResourceBind::RenderTarget)
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return flags;
}

VkImageViewType ToVkImageViewType(ResourceType resourceType, bool array)
{
    switch (resourceType)
    {
    case ResourceType::Texture1D:
        return array ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    case ResourceType::Texture2D:
    case ResourceType::Texture2DMultisample:
        return array ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    case ResourceType::TextureCube:
        return array ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
    case ResourceType::Texture3D:
        CT_CHECK(!array);
        return VK_IMAGE_VIEW_TYPE_3D;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported image type.");
        return array ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    }
}

VkImageAspectFlags ToVkImageAspect(ResourceFormat format, bool ignoreStencil)
{
    VkImageAspectFlags flags = 0;
    if (IsDepthFormat(format))
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (!ignoreStencil && IsStencilFormat(format))
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    if (!IsDepthStencilFormat(format))
        flags |= VK_IMAGE_ASPECT_COLOR_BIT;

    return flags;
}

VkPrimitiveTopology ToVkTopology(Topology topology)
{
    switch (topology)
    {
    case Topology::PointList:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case Topology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case Topology::LineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case Topology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case Topology::TriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported topology.");
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

VkPolygonMode ToVkPolygonMode(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::Fill:
        return VK_POLYGON_MODE_FILL;
    case PolygonMode::Wireframe:
        return VK_POLYGON_MODE_LINE;
    }

    CT_EXCEPTION(RenderCore, "Unsupported polygon mode.");
    return VK_POLYGON_MODE_FILL;
}

VkCullModeFlagBits ToVkCullMode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None:
        return VK_CULL_MODE_NONE;
    case CullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case CullMode::Back:
        return VK_CULL_MODE_BACK_BIT;
    }

    CT_EXCEPTION(RenderCore, "Unsupported cull mode.");
    return VK_CULL_MODE_NONE;
}

VkBlendFactor ToVkBlendFactor(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:
        return VK_BLEND_FACTOR_ZERO;
    case BlendFactor::One:
        return VK_BLEND_FACTOR_ONE;
    case BlendFactor::SrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFactor::OneMinusSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DstColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFactor::OneMinusDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::SrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFactor::OneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    }

    CT_EXCEPTION(RenderCore, "Unsupported blend factor.");
    return VK_BLEND_FACTOR_ZERO;
}

VkBlendOp ToVkBlendOperation(BlendOperation operation)
{
    switch (operation)
    {
    case BlendOperation::Add:
        return VK_BLEND_OP_ADD;
    case BlendOperation::Sub:
        return VK_BLEND_OP_SUBTRACT;
    case BlendOperation::ReverseSub:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    }

    CT_EXCEPTION(RenderCore, "Unsupported blend operation.");
    return VK_BLEND_OP_ADD;
}

VkStencilOp ToVkStencilOperation(StencilOperation operation)
{
    switch (operation)
    {
    case StencilOperation::Zero:
        return VK_STENCIL_OP_ZERO;
    case StencilOperation::Keep:
        return VK_STENCIL_OP_KEEP;
    case StencilOperation::Replace:
        return VK_STENCIL_OP_REPLACE;
    case StencilOperation::Increment:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOperation::Decrement:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOperation::IncrementWrap:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOperation::DecrementWrap:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    case StencilOperation::Invert:
        return VK_STENCIL_OP_INVERT;
    }

    CT_EXCEPTION(RenderCore, "Unsupported stencil operation.");
    return VK_STENCIL_OP_ZERO;
}

VkCompareOp ToVkCompareOperation(CompareOperation operation)
{
    switch (operation)
    {
    case CompareOperation::AlwaysFail:
        return VK_COMPARE_OP_NEVER;
    case CompareOperation::AlwaysPass:
        return VK_COMPARE_OP_ALWAYS;
    case CompareOperation::Less:
        return VK_COMPARE_OP_LESS;
    case CompareOperation::LessEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOperation::Equal:
        return VK_COMPARE_OP_EQUAL;
    case CompareOperation::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOperation::Greater:
        return VK_COMPARE_OP_GREATER;
    case CompareOperation::GreaterEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    }

    CT_EXCEPTION(RenderCore, "Unsupported compare operation.");
    return VK_COMPARE_OP_NEVER;
}

VkSampleCountFlagBits ToVkSampleCount(int32 sample)
{
    switch (sample)
    {
    case 0:
    case 1:
        return VK_SAMPLE_COUNT_1_BIT;
    case 2:
        return VK_SAMPLE_COUNT_2_BIT;
    case 4:
        return VK_SAMPLE_COUNT_4_BIT;
    case 8:
        return VK_SAMPLE_COUNT_8_BIT;
    case 16:
        return VK_SAMPLE_COUNT_16_BIT;
    case 32:
        return VK_SAMPLE_COUNT_32_BIT;
    case 64:
        return VK_SAMPLE_COUNT_64_BIT;
    }

    CT_EXCEPTION(RenderCore, "Unsupported sample count.");
    return VK_SAMPLE_COUNT_1_BIT;
}

VkSamplerAddressMode ToVkAddressMode(TextureWrap wrap)
{
    switch (wrap)
    {
    case TextureWrap::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureWrap::Mirror:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureWrap::Clamp:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureWrap::Border:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }

    CT_EXCEPTION(RenderCore, "Unsupported texture wrap mode.");
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkFilter ToVkFilter(TextureFilter filter)
{
    switch (filter)
    {
    case TextureFilter::Nearest:
        return VK_FILTER_NEAREST;
    case TextureFilter::Linear:
        return VK_FILTER_LINEAR;
    }

    CT_EXCEPTION(RenderCore, "Unsupported texture filter.");
    return VK_FILTER_NEAREST;
}

VkSamplerMipmapMode ToVkMipFilter(TextureFilter filter)
{
    switch (filter)
    {
    case TextureFilter::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case TextureFilter::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    CT_EXCEPTION(RenderCore, "Unsupported texture filter.");
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

VkBorderColor ToVkBorderColor(const Color &color)
{
    if (color.r > 0.0f || color.g > 0.0f || color.b > 0.0f)
        return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    if (color.a > 0.0f)
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
}

VkIndexType ToVkIndexType(ResourceFormat format)
{
    switch (format)
    {
    case ResourceFormat::R16UInt:
        return VK_INDEX_TYPE_UINT16;
    case ResourceFormat::R32UInt:
        return VK_INDEX_TYPE_UINT32;
    default:
        CT_EXCEPTION(RenderCore, "Unsupported index buffer format.");
        return VK_INDEX_TYPE_UINT32;
    }
}

struct VulkanResourceFormatDesc
{
    ResourceFormat format;
    VkFormat vkFormat;
};

static VulkanResourceFormatDesc formatDescs[] = {
    { ResourceFormat::Unknown, VK_FORMAT_UNDEFINED },
    { ResourceFormat::R8Unorm, VK_FORMAT_R8_UNORM },
    { ResourceFormat::R8Snorm, VK_FORMAT_R8_SNORM },
    { ResourceFormat::R16Unorm, VK_FORMAT_R16_UNORM },
    { ResourceFormat::R16Snorm, VK_FORMAT_R16_SNORM },
    { ResourceFormat::RG8Unorm, VK_FORMAT_R8G8_UNORM },
    { ResourceFormat::RG8Snorm, VK_FORMAT_R8G8_SNORM },
    { ResourceFormat::RG16Unorm, VK_FORMAT_R16G16_UNORM },
    { ResourceFormat::RG16Snorm, VK_FORMAT_R16G16_SNORM },
    { ResourceFormat::RGB16Unorm, VK_FORMAT_R16G16B16_UNORM },
    { ResourceFormat::RGB16Snorm, VK_FORMAT_R16G16B16_SNORM },
    { ResourceFormat::R24UnormX8, VK_FORMAT_UNDEFINED },
    { ResourceFormat::RGB5A1Unorm, VK_FORMAT_B5G5R5A1_UNORM_PACK16 }, // VK different component order?
    { ResourceFormat::RGBA8Unorm, VK_FORMAT_R8G8B8A8_UNORM },
    { ResourceFormat::RGBA8Snorm, VK_FORMAT_R8G8B8A8_SNORM },
    { ResourceFormat::RGB10A2Unorm, VK_FORMAT_A2R10G10B10_UNORM_PACK32 }, // VK different component order?
    { ResourceFormat::RGB10A2UInt, VK_FORMAT_A2R10G10B10_UINT_PACK32 },   // VK different component order?
    { ResourceFormat::RGBA16Unorm, VK_FORMAT_R16G16B16A16_UNORM },
    { ResourceFormat::RGBA8UnormSrgb, VK_FORMAT_R8G8B8A8_SRGB },
    { ResourceFormat::R16Float, VK_FORMAT_R16_SFLOAT },
    { ResourceFormat::RG16Float, VK_FORMAT_R16G16_SFLOAT },
    { ResourceFormat::RGB16Float, VK_FORMAT_R16G16B16_SFLOAT },
    { ResourceFormat::RGBA16Float, VK_FORMAT_R16G16B16A16_SFLOAT },
    { ResourceFormat::R32Float, VK_FORMAT_R32_SFLOAT },
    { ResourceFormat::R32FloatX32, VK_FORMAT_UNDEFINED },
    { ResourceFormat::RG32Float, VK_FORMAT_R32G32_SFLOAT },
    { ResourceFormat::RGB32Float, VK_FORMAT_R32G32B32_SFLOAT },
    { ResourceFormat::RGBA32Float, VK_FORMAT_R32G32B32A32_SFLOAT },
    { ResourceFormat::R11G11B10Float, VK_FORMAT_B10G11R11_UFLOAT_PACK32 }, // Unsigned in VK
    { ResourceFormat::RGB9E5Float, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 },     // Unsigned in VK
    { ResourceFormat::R8Int, VK_FORMAT_R8_SINT },
    { ResourceFormat::R8UInt, VK_FORMAT_R8_UINT },
    { ResourceFormat::R16Int, VK_FORMAT_R16_SINT },
    { ResourceFormat::R16UInt, VK_FORMAT_R16_UINT },
    { ResourceFormat::R32Int, VK_FORMAT_R32_SINT },
    { ResourceFormat::R32UInt, VK_FORMAT_R32_UINT },
    { ResourceFormat::RG8Int, VK_FORMAT_R8G8_SINT },
    { ResourceFormat::RG8UInt, VK_FORMAT_R8G8_UINT },
    { ResourceFormat::RG16Int, VK_FORMAT_R16G16_SINT },
    { ResourceFormat::RG16UInt, VK_FORMAT_R16G16_UINT },
    { ResourceFormat::RG32Int, VK_FORMAT_R32G32_SINT },
    { ResourceFormat::RG32UInt, VK_FORMAT_R32G32_UINT },
    { ResourceFormat::RGB16Int, VK_FORMAT_R16G16B16_SINT },
    { ResourceFormat::RGB16UInt, VK_FORMAT_R16G16B16_UINT },
    { ResourceFormat::RGB32Int, VK_FORMAT_R32G32B32_SINT },
    { ResourceFormat::RGB32UInt, VK_FORMAT_R32G32B32_UINT },
    { ResourceFormat::RGBA8Int, VK_FORMAT_R8G8B8A8_SINT },
    { ResourceFormat::RGBA8UInt, VK_FORMAT_R8G8B8A8_UINT },
    { ResourceFormat::RGBA16Int, VK_FORMAT_R16G16B16A16_SINT },
    { ResourceFormat::RGBA16UInt, VK_FORMAT_R16G16B16A16_UINT },
    { ResourceFormat::RGBA32Int, VK_FORMAT_R32G32B32A32_SINT },
    { ResourceFormat::RGBA32UInt, VK_FORMAT_R32G32B32A32_UINT },
    { ResourceFormat::BGRA8Unorm, VK_FORMAT_B8G8R8A8_UNORM },
    { ResourceFormat::BGRA8UnormSrgb, VK_FORMAT_B8G8R8A8_SRGB },
    { ResourceFormat::BGRX8Unorm, VK_FORMAT_B8G8R8A8_UNORM },
    { ResourceFormat::BGRX8UnormSrgb, VK_FORMAT_B8G8R8A8_SRGB },
    { ResourceFormat::Alpha8Unorm, VK_FORMAT_UNDEFINED },
    { ResourceFormat::Alpha32Float, VK_FORMAT_UNDEFINED },
    { ResourceFormat::R5G6B5Unorm, VK_FORMAT_R5G6B5_UNORM_PACK16 },
    { ResourceFormat::D32Float, VK_FORMAT_D32_SFLOAT },
    { ResourceFormat::D16Unorm, VK_FORMAT_D16_UNORM },
    { ResourceFormat::D32FloatS8X24, VK_FORMAT_D32_SFLOAT_S8_UINT },
    { ResourceFormat::D24UnormS8, VK_FORMAT_D24_UNORM_S8_UINT },
    { ResourceFormat::BC1Unorm, VK_FORMAT_BC1_RGB_UNORM_BLOCK },
    { ResourceFormat::BC1UnormSrgb, VK_FORMAT_BC1_RGB_SRGB_BLOCK },
    { ResourceFormat::BC2Unorm, VK_FORMAT_BC2_UNORM_BLOCK },
    { ResourceFormat::BC2UnormSrgb, VK_FORMAT_BC2_SRGB_BLOCK },
    { ResourceFormat::BC3Unorm, VK_FORMAT_BC3_UNORM_BLOCK },
    { ResourceFormat::BC3UnormSrgb, VK_FORMAT_BC3_SRGB_BLOCK },
    { ResourceFormat::BC4Unorm, VK_FORMAT_BC4_UNORM_BLOCK },
    { ResourceFormat::BC4Snorm, VK_FORMAT_BC4_SNORM_BLOCK },
    { ResourceFormat::BC5Unorm, VK_FORMAT_BC5_UNORM_BLOCK },
    { ResourceFormat::BC5Snorm, VK_FORMAT_BC5_SNORM_BLOCK },
    { ResourceFormat::BC6HS16, VK_FORMAT_BC6H_SFLOAT_BLOCK },
    { ResourceFormat::BC6HU16, VK_FORMAT_BC6H_UFLOAT_BLOCK },
    { ResourceFormat::BC7Unorm, VK_FORMAT_BC7_UNORM_BLOCK },
    { ResourceFormat::BC7UnormSrgb, VK_FORMAT_BC7_SRGB_BLOCK },
};

VkFormat ToVkResourceFormat(ResourceFormat format)
{
    CT_CHECK(formatDescs[static_cast<int32>(format)].format == format);
    return formatDescs[static_cast<int32>(format)].vkFormat;
}

ResourceBindFlags GetResourceFormatBindFlags(ResourceFormat format)
{
    VkFormatProperties p;
    vkGetPhysicalDeviceFormatProperties(gVulkanDevice->GetPhysicalDeviceHandle(), ToVkResourceFormat(format), &p);

    auto ConvertFlags = [](VkFormatFeatureFlags vk) -> ResourceBindFlags {
        ResourceBindFlags f = ResourceBind::None;
        if (vk & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
            f |= ResourceBind::ShaderResource;
        if (vk & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)
            f |= ResourceBind::ShaderResource;
        if (vk & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
            f |= ResourceBind::ShaderResource;
        if (vk & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            f |= ResourceBind::UnorderedAccess;
        if (vk & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)
            f |= ResourceBind::UnorderedAccess;
        if (vk & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)
            f |= ResourceBind::UnorderedAccess;
        if (vk & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)
            f |= ResourceBind::UnorderedAccess;
        if (vk & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
            f |= ResourceBind::Vertex;
        if (vk & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
            f |= ResourceBind::RenderTarget;
        if (vk & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)
            f |= ResourceBind::RenderTarget;
        if (vk & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            f |= ResourceBind::DepthStencil;

        return f;
    };

    ResourceBindFlags flags = ResourceBind::None;
    flags |= ConvertFlags(p.bufferFeatures);
    flags |= ConvertFlags(p.linearTilingFeatures);
    flags |= ConvertFlags(p.optimalTilingFeatures);

    switch (format)
    {
    case ResourceFormat::R16UInt:
    case ResourceFormat::R32UInt:
        flags |= ResourceBind::Index;
        break;
    default:
        break;
    }

    return flags;
}