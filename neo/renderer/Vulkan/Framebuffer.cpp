
#include "Framebuffer.hpp"
#include "Core.hpp"

static inline bool IsDepthFormat( const VkFormat format ) 
{
    static const VkFormat depthFormats[5] = 
    {
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT
    };
 
    for ( uint32_t i = 0; i < 5; i++)
    {
        if (format == depthFormats[i] )
            return true;
    }
    return false;
}

static inline bool IsStencilFormat( const VkFormat format ) 
{
    static const VkFormat stencilFormats[4] = 
    {
        VK_FORMAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT
    };

    for ( uint32_t i = 0; i < 4; i++)
    {
        if (format == stencilFormats[i] )
            return true;
    }
    return false;
}

crFramebuffer::crFramebuffer( void ) : m_unifiedMemory( nullptr )
{
}

crFramebuffer::~crFramebuffer( void )
{
}

void crFramebuffer::Create( const createInfo_t &in_createInfo, const uint32_t in_frames )
{
}

void crFramebuffer::Destroy(void)
{
}

void crFramebuffer::Bind( void )
{
    VkRenderingAttachmentInfo colorAttachment{};
    VkRenderingAttachmentInfo depthStencilAttachment{};
    auto command = tr.GraphicCommandBuffer();

    if( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
    {
        crTexture::state_t state{};
        state.family = VK_QUEUE_FAMILY_IGNORED;
        state.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        state.stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        state.access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        m_colorAttachament[m_bufferID].SetState( command, state );
    }

    if ( m_properties.depthStencilFormat != VK_FORMAT_UNDEFINED )
    {
        crTexture::state_t state{};
        state.family = VK_QUEUE_FAMILY_IGNORED;
        state.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        state.stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        state.access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        m_depthAttachament[m_bufferID].SetState( command, state );
    }

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.flags = 0;
    renderingInfo.renderArea = { 0, 0, m_properties.width, m_properties.height };
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    
    if( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
    {
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
    }
    else
    {
        renderingInfo.colorAttachmentCount = 0;
        renderingInfo.pColorAttachments = nullptr;
    }

    if ( IsDepthFormat( m_properties.depthStencilFormat ) )
        renderingInfo.pDepthAttachment = &depthStencilAttachment;
    else
        renderingInfo.pDepthAttachment = nullptr;

    if ( IsStencilFormat( m_properties.depthStencilFormat ) )
        renderingInfo.pStencilAttachment = &depthStencilAttachment;
    else
        renderingInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering( *command, &renderingInfo );
}

void crFramebuffer::Unbind(void)
{
    auto command = tr.GraphicCommandBuffer();

    ///
    ///
    /// End frame rendering
    vkCmdEndRendering( *command );

    ///
    ///
    /// Perform a state transition in the present image
    if ( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
    {
        crTexture::state_t state{};
        state.family = VK_QUEUE_FAMILY_IGNORED;
        state.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        state.stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        state.access = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;
        m_colorAttachament[m_bufferID].SetState( command, state );
    }

    if ( m_properties.depthStencilFormat != VK_FORMAT_UNDEFINED )
    {
        crTexture::state_t state{};
        state.family = VK_QUEUE_FAMILY_IGNORED;
        state.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        state.stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        state.access = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;
        m_colorAttachament[m_bufferID].SetState( command, state );
    }
}

void crFramebuffer::ReadPixels(int32_t x, int32_t y, uint32_t width, uint32_t height, VkFormat format, void *data)
{
}

void crFramebuffer::DrawPixels(uint32_t width, uint32_t height, VkFormat format, const void *data)
{
}

void crFramebuffer::BlitColorAttachament(const blitInfo_t &in_blitInfo)
{
#if 0
    auto commands = crBackend::Get()->CommandBuffer();

    // update image state
    //VkImageStateTransition( in_blitInfo.dstImage, *commands, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT );

    VkImageBlit2 imageBlit{};
    imageBlit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
    imageBlit.pNext = nullptr;
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
    imageBlit.srcSubresource.mipLevel = 0;
    imageBlit.srcSubresource.baseArrayLayer = 0;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcOffsets[0].x = in_blitInfo.srcX;
    imageBlit.srcOffsets[0].y = in_blitInfo.srcY;
    imageBlit.srcOffsets[0].z = in_blitInfo.srcZ;
    imageBlit.srcOffsets[1].x = in_blitInfo.srcWidth;
    imageBlit.srcOffsets[1].y = in_blitInfo.srcHeigth;
    imageBlit.srcOffsets[1].z = in_blitInfo.srcDepth;
    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
    imageBlit.dstSubresource.mipLevel = 0;
    imageBlit.dstSubresource.baseArrayLayer = 0;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstOffsets[0].x = in_blitInfo.dstX;
    imageBlit.dstOffsets[0].y = in_blitInfo.dstY;
    imageBlit.dstOffsets[0].z = in_blitInfo.dstZ;
    imageBlit.dstOffsets[1].x = in_blitInfo.dstWidth;
    imageBlit.dstOffsets[1].y = in_blitInfo.dstHeigth;
    imageBlit.dstOffsets[1].z = in_blitInfo.dstDepth;

    VkBlitImageInfo2 blitImageInfo{};
    blitImageInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    blitImageInfo.pNext = nullptr;
    blitImageInfo.srcImage = m_colorAttachament[m_bufferID];
    blitImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitImageInfo.dstImage = in_blitInfo.dstImage->image;
    blitImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitImageInfo.regionCount = 1;
    blitImageInfo.pRegions = &imageBlit;
    blitImageInfo.filter = VK_FILTER_LINEAR;
    vkCmdBlitImage2( *commands, &blitImageInfo );
#endif
}

void crFramebuffer::BlitDepthStencilAttachament( const blitInfo_t &in_blitInfo )
{
}