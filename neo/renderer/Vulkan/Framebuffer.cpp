
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

vkFramebuffer::vkFramebuffer( void ) :
    m_bufferID( 0 ),
    m_unifiedMemory( nullptr )
{
}

vkFramebuffer::~vkFramebuffer( void )
{
}

void vkFramebuffer::Create( const createInfo_t &in_createInfo )
{
    uint32_t                i = 0;
    VkResult                result = VK_SUCCESS;
    uint32_t                queues[2]{};
    VkDeviceSize            colorMemoryOffset[SMP_FRAMES]{};
    VkDeviceSize            depthMemoryOffset[SMP_FRAMES]{};
    VkMemoryRequirements    memRequirements{};
    VkMemoryAllocateInfo    memoryAllocate{};
    m_properties = in_createInfo;

    auto device = tr.GetRenderDevice();
    auto graphic = device->GraphicQueue();
    auto transfer = device->TransferQueue();
    
    queues[0] = graphic->Family();
    if ( transfer )
        queues[1] = transfer->Family();
    else
        queues[1] = graphic->Family();

    m_colorAttachament.SetNum( SMP_FRAMES );
    m_depthAttachament.SetNum( SMP_FRAMES );

    queues[0] = graphic->Family();
    if ( transfer )
        queues[1] = transfer->Family();
    else
        queues[1] = graphic->Family();

    m_properties = in_createInfo;

    ///
    /// Color attachament image creation
    VkImageCreateInfo   colorImageCI{};
    colorImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    colorImageCI.pNext = nullptr;
    colorImageCI.flags = 0;
    colorImageCI.imageType = VK_IMAGE_TYPE_2D;
    colorImageCI.format = in_createInfo.colorFormat;
    colorImageCI.extent = { m_properties.width, m_properties.height, 1 };
    colorImageCI.mipLevels = 1;
    colorImageCI.arrayLayers = in_createInfo.arrayLayers;
    colorImageCI.samples = in_createInfo.samples;
    colorImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    colorImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    colorImageCI.sharingMode = ( queues[0] != queues[1] ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    colorImageCI.queueFamilyIndexCount = ( queues[0] != queues[1] ) ? 2 : 1;
    colorImageCI.pQueueFamilyIndices = queues;
    colorImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    /// 
    /// Color attachamen image view 
    VkImageViewCreateInfo colorImageViewCI{};
    colorImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorImageViewCI.pNext = nullptr;
    colorImageViewCI.flags = 0;
    colorImageViewCI.image = nullptr;
    colorImageViewCI.viewType = ( in_createInfo.arrayLayers > 1 ) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    colorImageViewCI.format = in_createInfo.colorFormat;
    colorImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    colorImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    colorImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    colorImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    colorImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageViewCI.subresourceRange.baseMipLevel = 0;
    colorImageViewCI.subresourceRange.levelCount = 1;
    colorImageViewCI.subresourceRange.baseArrayLayer = 0;
    colorImageViewCI.subresourceRange.layerCount = in_createInfo.arrayLayers;
    
    /// 
    /// Depth stencil image creation
    VkImageCreateInfo   depthStencilImageCI{};
    depthStencilImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthStencilImageCI.pNext = nullptr;
    depthStencilImageCI.flags = 0;
    depthStencilImageCI.imageType = VK_IMAGE_TYPE_2D;
    depthStencilImageCI.format = in_createInfo.depthStencilFormat;
    depthStencilImageCI.extent = { m_properties.width, m_properties.height, 1 };
    depthStencilImageCI.mipLevels = 1;
    depthStencilImageCI.arrayLayers = in_createInfo.arrayLayers;
    depthStencilImageCI.samples = in_createInfo.samples;
    depthStencilImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthStencilImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    depthStencilImageCI.sharingMode = ( queues[0] != queues[1] ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    depthStencilImageCI.queueFamilyIndexCount = ( queues[0] != queues[1] ) ? 2 : 1;
    depthStencilImageCI.pQueueFamilyIndices = queues;
    /// VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    /// VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
    /// VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    depthStencilImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    /// 
    /// Color attachamen image view 
    VkImageViewCreateInfo depthStencilImageViewCI{};
    depthStencilImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthStencilImageViewCI.pNext = nullptr;
    depthStencilImageViewCI.flags = 0;
    depthStencilImageViewCI.image = nullptr;
    depthStencilImageViewCI.viewType = ( in_createInfo.arrayLayers > 1 ) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    depthStencilImageViewCI.format = in_createInfo.depthStencilFormat;
    depthStencilImageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthStencilImageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthStencilImageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthStencilImageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    depthStencilImageViewCI.subresourceRange.baseMipLevel = 0;
    depthStencilImageViewCI.subresourceRange.levelCount = 1;
    depthStencilImageViewCI.subresourceRange.baseArrayLayer = 0;
    depthStencilImageViewCI.subresourceRange.layerCount = in_createInfo.arrayLayers;
    depthStencilImageViewCI.subresourceRange.aspectMask = 0;
    if ( IsDepthFormat( in_createInfo.depthStencilFormat ) )
        depthStencilImageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT; 
    if ( IsStencilFormat( in_createInfo.depthStencilFormat ) )
        depthStencilImageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    memoryAllocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocate.pNext = nullptr;
    memoryAllocate.allocationSize = 0;
    memoryAllocate.memoryTypeIndex = 0;
    VkDeviceSize totalSize = 0;
    uint32_t memoryTypeBits = 0xFFFFFFFF;

    for ( i = 0; i < SMP_FRAMES; i++)
    {
        /*
        =====================================================================================================
                                        Color Attachament Image Creation
        =====================================================================================================
        */
        if( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
        {
            // create color image handle 
            result = vkCreateImage( *device, &colorImageCI, k_allocationCallbacks, &m_colorAttachament[i].image );
            if ( result != VK_SUCCESS ) 
                common->Error( "vkFramebuffer::Create::vkCreateImage ERROR: %s\n", VulkanErrorString( result ).c_str() );

            // get image memory requeriments 
            vkGetImageMemoryRequirements( *device, m_colorAttachament[i].image, &memRequirements );

            // Adjusts the current offset to the alignment required by the image.
            VkDeviceSize alignment = memRequirements.alignment;
            if (totalSize % alignment != 0) 
                totalSize += alignment - (totalSize % alignment);

            colorMemoryOffset[i] = totalSize; // Save where this image begins.
            totalSize += memRequirements.size;
            memoryTypeBits &= memRequirements.memoryTypeBits; // Intersection of supported memory types
        }

        /*
        =====================================================================================================
                                        Depth Attachament Image Creation
        =====================================================================================================
        */
        if( m_properties.depthStencilFormat != VK_FORMAT_UNDEFINED )
        {
            // create depth stencil image handle 
            result = vkCreateImage( *device, &depthStencilImageCI, k_allocationCallbacks, &m_depthAttachament[i].image );
            if ( result != VK_SUCCESS ) 
                common->Error( "vkFramebuffer::Create::vkCreateImage ERROR: %s\n", VulkanErrorString( result ).c_str() );

            // get image memory requeriments 
            vkGetImageMemoryRequirements( *device, m_depthAttachament[i].image, &memRequirements );

            // Adjusts the current offset to the alignment required by the image.
            VkDeviceSize alignment = memRequirements.alignment;
            if (totalSize % alignment != 0) 
                totalSize += alignment - (totalSize % alignment);

            depthMemoryOffset[i] = totalSize; // Save where this image begins.
            totalSize += memRequirements.size;
            memoryTypeBits &= memRequirements.memoryTypeBits; // Intersection of supported memory types
        }
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = totalSize;
    allocInfo.memoryTypeIndex = device->FindMemoryType( memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    // Keep only one VkDeviceMemory in your class: VkDeviceMemory m_unifiedMemory;
    result = vkAllocateMemory(*device, &allocInfo, k_allocationCallbacks, &m_unifiedMemory );
    if( result != VK_SUCCESS )
        common->Error( "vkFramebuffer::Create::vkAllocateMemory ERROR: %s\n", VulkanErrorString( result ).c_str() );

    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        /*
        =====================================================================================================
                                        Color Attachament Image Creation
        =====================================================================================================
        */
        if( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
        {
            
            memoryAllocate.allocationSize = memRequirements.size;
            memoryAllocate.memoryTypeIndex = device->FindMemoryType( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

            // bind image to the handle 
            result = vkBindImageMemory( *device, m_colorAttachament[i].image, m_unifiedMemory, colorMemoryOffset[i] );
            if( result != VK_SUCCESS )
                common->Error( "vkFramebuffer::Create::vkBindImageMemory ERROR: %s\n", VulkanErrorString( result ).c_str() );

            // create image view
            colorImageViewCI.image = m_colorAttachament[i].image;
            result = vkCreateImageView( *device, &colorImageViewCI, k_allocationCallbacks, &m_colorAttachament[i].view );
            if( result != VK_SUCCESS )
                common->Error( "vkFramebuffer::Create::vkCreateImageView ERROR: %s\n", VulkanErrorString( result ).c_str() );
        }

        /*
        =====================================================================================================
                                        Depth Attachament Image Creation
        =====================================================================================================
        */
        if( m_properties.depthStencilFormat != VK_FORMAT_UNDEFINED )
        {
            
            //
            memoryAllocate.allocationSize = memRequirements.size;
            memoryAllocate.memoryTypeIndex = device->FindMemoryType( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

            // bind image to the handle 
            result = vkBindImageMemory( *device, m_depthAttachament[i].image, m_unifiedMemory, depthMemoryOffset[i] );
            if( result != VK_SUCCESS )
                common->Error( "vkFramebuffer::Create::vkBindImageMemory ERROR: %s\n", VulkanErrorString( result ).c_str() );

            // create image view
            depthStencilImageViewCI.image = m_depthAttachament[i].image;
            result = vkCreateImageView( *device, &depthStencilImageViewCI, k_allocationCallbacks, &m_depthAttachament[i].view );
            if( result != VK_SUCCESS )
                common->Error( "vkFramebuffer::Create::vkCreateImageView ERROR: %s\n", VulkanErrorString( result ).c_str() );
        }
    }
}

void vkFramebuffer::Destroy(void)
{
    auto device = tr.GetRenderDevice();
    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        vkDestroyImageView( *device, m_colorAttachament[i].view, k_allocationCallbacks );
        vkDestroyImageView( *device, m_depthAttachament[i].view, k_allocationCallbacks );
        vkDestroyImage( *device, m_depthAttachament[i].image, k_allocationCallbacks );
        vkDestroyImage( *device, m_colorAttachament[i].image, k_allocationCallbacks );
    }

    vkFreeMemory( *device, m_unifiedMemory, k_allocationCallbacks );
}

void vkFramebuffer::Bind( void )
{
    uint32_t barriers = 0;
    VkImageMemoryBarrier2 acquireBarrier[2]{{},{}};
    VkRenderingAttachmentInfo colorAttachment{};
    VkRenderingAttachmentInfo depthStencilAttachment{};
    
    auto command = crBackend::Get()->CommandBuffer();
    m_bufferID = crBackend::Get()->FrameID();
    
    ///
    ///
    ///
    if( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
    {   
        acquireBarrier[barriers].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        acquireBarrier[barriers].srcStageMask = m_colorAttachament[m_bufferID].stage;
        acquireBarrier[barriers].srcAccessMask = m_colorAttachament[m_bufferID].access;
        acquireBarrier[barriers].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        acquireBarrier[barriers].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        acquireBarrier[barriers].oldLayout = m_colorAttachament[m_bufferID].layout;
        acquireBarrier[barriers].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        acquireBarrier[barriers].image = m_colorAttachament[m_bufferID];
        acquireBarrier[barriers].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        acquireBarrier[barriers].subresourceRange.levelCount = 1;
        acquireBarrier[barriers].subresourceRange.layerCount = 1;

        /// update image state
        m_colorAttachament[m_bufferID].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_colorAttachament[m_bufferID].stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        m_colorAttachament[m_bufferID].access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        /// bind color to render
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO; 
        colorAttachment.pNext = nullptr; 
        colorAttachment.imageView = m_colorAttachament[m_bufferID];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE; 
        colorAttachment.resolveImageView = nullptr;
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // clear to orange
        colorAttachment.clearValue.color.float32[0] = 0.7f; 
        colorAttachment.clearValue.color.float32[1] = 0.5f; 
        colorAttachment.clearValue.color.float32[2] = 0.2f; 
        colorAttachment.clearValue.color.float32[3] = 1.0f; 

        barriers++;
    }

    if ( m_properties.depthStencilFormat != VK_FORMAT_UNDEFINED )
    {
        acquireBarrier[barriers].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        acquireBarrier[barriers].srcStageMask = m_depthAttachament[m_bufferID].stage;
        acquireBarrier[barriers].srcAccessMask = m_depthAttachament[m_bufferID].access;
        acquireBarrier[barriers].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        acquireBarrier[barriers].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        acquireBarrier[barriers].oldLayout = m_depthAttachament[m_bufferID].layout; // Sempre trate como Undefined ao adquirir
        acquireBarrier[barriers].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        acquireBarrier[barriers].image = m_depthAttachament[m_bufferID];
        acquireBarrier[barriers].subresourceRange.aspectMask = 0;
        if( IsDepthFormat( m_properties.depthStencilFormat ) ) acquireBarrier[barriers].subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if( IsStencilFormat( m_properties.depthStencilFormat ) ) acquireBarrier[barriers].subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        /// update image state
        m_depthAttachament[m_bufferID].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        m_depthAttachament[m_bufferID].stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        m_depthAttachament[m_bufferID].access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        acquireBarrier[barriers].subresourceRange.levelCount = 1;
        acquireBarrier[barriers].subresourceRange.layerCount = 1;

        /// bind depth stencil to render
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO; 
        depthStencilAttachment.pNext = nullptr; 
        depthStencilAttachment.imageView = m_colorAttachament[m_bufferID];
        depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.resolveMode = VK_RESOLVE_MODE_NONE; 
        depthStencilAttachment.resolveImageView = nullptr;
        depthStencilAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // clear
        depthStencilAttachment.clearValue.depthStencil.depth = 0.0f;
        depthStencilAttachment.clearValue.depthStencil.stencil = 1; 

        barriers++;
    }
    
    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.imageMemoryBarrierCount = barriers;
    depInfo.pImageMemoryBarriers = acquireBarrier;
    vkCmdPipelineBarrier2( *command, &depInfo );
    
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

void vkFramebuffer::Unbind(void)
{
    uint32_t barriers = 0;
    VkImageMemoryBarrier2 acquireBarrier[2];
    auto command = crBackend::Get()->CommandBuffer(); 

    ///
    ///
    /// End frame rendering
    vkCmdEndRendering( *command );

    ///
    ///
    /// Perform a state transition in the present image
    if ( m_properties.colorFormat != VK_FORMAT_UNDEFINED )
    {
        acquireBarrier[barriers].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        acquireBarrier[barriers].pNext = nullptr;
        acquireBarrier[barriers].srcStageMask = m_colorAttachament[m_bufferID].stage;
        acquireBarrier[barriers].srcAccessMask = m_colorAttachament[m_bufferID].access;
        acquireBarrier[barriers].dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        acquireBarrier[barriers].dstAccessMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;
        acquireBarrier[barriers].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        acquireBarrier[barriers].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        acquireBarrier[barriers].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //m_graphicQueue->Family();
        acquireBarrier[barriers].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //m_presentQueue->Family();
        acquireBarrier[barriers].image = m_colorAttachament[m_bufferID];
        acquireBarrier[barriers].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        acquireBarrier[barriers].subresourceRange.baseMipLevel = 0;
        acquireBarrier[barriers].subresourceRange.levelCount = 1;
        acquireBarrier[barriers].subresourceRange.baseArrayLayer = 0;
        acquireBarrier[barriers].subresourceRange.layerCount = 1;

        /// update image state
        m_colorAttachament[m_bufferID].layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        m_colorAttachament[m_bufferID].stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        m_colorAttachament[m_bufferID].access = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;

        barriers++;
    }

    if ( m_properties.depthStencilFormat != VK_FORMAT_UNDEFINED )
    {
        acquireBarrier[barriers].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        acquireBarrier[barriers].pNext = nullptr;
        acquireBarrier[barriers].srcStageMask = m_depthAttachament[m_bufferID].stage;
        acquireBarrier[barriers].srcAccessMask = m_depthAttachament[m_bufferID].access;
        acquireBarrier[barriers].dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        acquireBarrier[barriers].dstAccessMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;
        acquireBarrier[barriers].oldLayout = m_depthAttachament[m_bufferID].layout;
        acquireBarrier[barriers].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        acquireBarrier[barriers].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //m_graphicQueue->Family();
        acquireBarrier[barriers].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //m_presentQueue->Family();
        acquireBarrier[barriers].image = m_depthAttachament[m_bufferID];
        acquireBarrier[barriers].subresourceRange.aspectMask = 0;
        if( IsDepthFormat( m_properties.depthStencilFormat ) ) acquireBarrier[barriers].subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if( IsStencilFormat( m_properties.depthStencilFormat ) ) acquireBarrier[barriers].subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        acquireBarrier[barriers].subresourceRange.baseMipLevel = 0;
        acquireBarrier[barriers].subresourceRange.levelCount = 1;
        acquireBarrier[barriers].subresourceRange.baseArrayLayer = 0;
        acquireBarrier[barriers].subresourceRange.layerCount = 1;

        /// update image state
        m_depthAttachament[m_bufferID].layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        m_depthAttachament[m_bufferID].stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        m_depthAttachament[m_bufferID].access = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;

        barriers++;
    }

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;
    dependencyInfo.dependencyFlags = 0;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.pMemoryBarriers = nullptr;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.pBufferMemoryBarriers = nullptr;
    dependencyInfo.imageMemoryBarrierCount = barriers;
    dependencyInfo.pImageMemoryBarriers = acquireBarrier;
    vkCmdPipelineBarrier2( *command, &dependencyInfo );
}

void vkFramebuffer::ReadPixels(int32_t x, int32_t y, uint32_t width, uint32_t height, VkFormat format, void *data)
{
}

void vkFramebuffer::DrawPixels(uint32_t width, uint32_t height, VkFormat format, const void *data)
{
}

void vkFramebuffer::BlitColorAttachament(const blitInfo_t &in_blitInfo)
{
    auto commands = crBackend::Get()->CommandBuffer();

    // update image state
    VkImageStateTransition( in_blitInfo.dstImage, *commands, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT );

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
}

void vkFramebuffer::BlitDepthStencilAttachament( const blitInfo_t &in_blitInfo )
{
}
