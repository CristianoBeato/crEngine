/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

This file is part of the crEngine GPL Source Code ("crEngine Source Code").

crEngine Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

crEngine Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef __VK_COMMAND_BUFFER_HPP__
#define __VK_COMMAND_BUFFER_HPP__

class vkCommandBuffer : public crCommandBuffer
{
public: 
    vkCommandBuffer( const vkDeviceQueue* in_queue );
    ~vkCommandBuffer( void );

    virtual void    Begin( void ) override;
    virtual void    End( void ) override;
    virtual void    Submit( void ) override;
    
    void ExecuteCommands( const uint32_t in_commandBufferCount, const VkCommandBuffer* in_commandBuffers );
    ID_INLINE void              FrameOperationsFenceCountIncrement( void ) { m_frameOperationsFenceCount++; }
    ID_INLINE VkSemaphore       FrameOperationsFenceSemaphore( void ) const { return m_frameOperationsFence; }
    ID_INLINE uint32_t          Family( void ) const { return m_queue->Family(); }
    ID_INLINE VkCommandBuffer   CommandBuffer( void ) const { return m_command[m_frame]; }   

protected:
    uint32_t            m_frame;
    uint64_t            m_frameOperationsFenceCount;
    VkSemaphore         m_frameOperationsFence;
    VkDevice            m_device;
    vkDeviceQueue*      m_queue;
    VkCommandBuffer     m_command[SMP_FRAMES];
    VkFence             m_renderDone[SMP_FRAMES];
};

class vkTransferCommandBuffer : 
    public crTransferCommandBuffer, 
    public vkCommandBuffer
{
public:
    vkTransferCommandBuffer( const vkDeviceQueue* in_queue );
    ~vkTransferCommandBuffer( void );

    virtual void    Begin( void ) override { vkCommandBuffer::Begin(); };
    virtual void    End( void ) override { vkCommandBuffer::End(); };
    virtual void    Submit( void ) override { vkCommandBuffer::Submit(); };

    virtual void    CopyTexture( const crTexture* in_src, const crTexture* in_dst, const idList<crTexture::subImage_t> in_subImages ) override;
    virtual void    CopyBufferToTexture( const crBuffer* in_buffer, const crTexture* in_texture, const idList<crTexture::subImage_t> in_subImages ) override;
    virtual void    CopyTextureToBuffer( const crBuffer* in_buffer, const crTexture* in_texture, const idList<crTexture::subImage_t> in_subImages ) override;
    virtual void    CopyBuffer( const crBuffer* in_srcBuffer, const crBuffer* in_dstBuffer, const uintptr_t in_offset, const size_t in_size ) override;
};

class vkGraphicCommandBuffer : 
    public crGraphicCommandBuffer, 
    public vkCommandBuffer
{
public:
    vkGraphicCommandBuffer( const vkDeviceQueue* in_queue );
    ~vkGraphicCommandBuffer( void );

    virtual void    Begin( void ) override { vkCommandBuffer::Begin(); };
    virtual void    End( void ) override { vkCommandBuffer::End(); };
    virtual void    Submit( void ) override { vkCommandBuffer::Submit(); };

    virtual void    LineWidth( const float in_lineWidth ) const;
    virtual void    BindFrameBuffer( const crFramebuffer* in_framebuffef );
    virtual void    BindIndexBuffer( const crBuffer* in_buffer );
    virtual void    BindVertexBuffers( const crBuffer* crPipelinein_buffer, uint32_t in_binding, const uintptr_t in_offsets, const size_t in_sizes, const size_t in_strides );
    virtual void    BindPipeline( const crPipeline* in_pipeline );
    virtual void    EndRenderPass( void ) const;
    virtual void    Draw(  const uint32_t in_vertexCount, const uint32_t in_instanceCount, const uint32_t in_firstVertex, const uint32_t in_firstInstance ) const;
    virtual void    DrawIndexed( const uint32_t in_indexCount, const uint32_t in_instanceCount, const uint32_t in_firstIndex, const int32_t in_vertexOffset, const uint32_t in_firstInstance ) const;
    virtual void    Dispatch(  const uint32_t in_groupCountX, const uint32_t in_groupCountY, const uint32_t in_groupCountZ ) const;
    virtual void    Clear( bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a );
    virtual void    PolygonOffset( const float scale, const float bias, const bool enable );
    virtual void    DepthBoundsTest( const float zmin, const float zmax, const bool enable );
    virtual void    FaceCull( const crPipeline::Face_t in_cullType );
    virtual void    Scissor( const int x, const int y, const int w, const int h ) const;
    virtual void    Viewport( const int x, const int y, const int w, const int h ) const;
    virtual void    CopyTexture( const crTexture* in_src, const crTexture* in_dst, const idList<crTexture::subImage_t> in_subImages );
    virtual void    CopyBufferToTexture( const crBuffer* in_buffer, const crTexture* in_texture, const idList<crTexture::subImage_t> in_subImages );    
    virtual void    CopyTextureToBuffer( const crBuffer* in_buffer, const crTexture* in_texture, const idList<crTexture::subImage_t> in_subImages );
    virtual void    CopyBuffer( const crBuffer* in_srcBuffer, const crBuffer* in_dstBuffer, const uintptr_t in_offset, const size_t in_size );
    
private:
    VkClearValue        m_clearValues;
    VkDescriptorPool    m_descriptorPool;
};

#endif //!__VK_COMMAND_BUFFER_HPP__