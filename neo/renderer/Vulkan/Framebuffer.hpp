
#ifndef __FRAMEBUFFER_HPP__
#define __FRAMEBUFFER_HPP__

class crFramebuffer
{    
public:
    struct createInfo_t
    {
        uint32_t                width;
        uint32_t                height;
        uint32_t                arrayLayers;
        VkFormat                colorFormat;
        VkFormat                depthStencilFormat;
        VkSampleCountFlagBits   samples;
    };

    struct blitInfo_t
    {
        int32_t             srcX = 0;
        int32_t             srcY = 0;
        int32_t             srcZ = 0;
        int32_t             dstX = 0;
        int32_t             dstY = 0;
        int32_t             dstZ = 0;
        uint32_t            srcWidth = 0;
        uint32_t            srcHeigth = 0;
        uint32_t            srcDepth = 0;
        uint32_t            dstWidth = 0;
        uint32_t            dstHeigth = 0;
        uint32_t            dstDepth = 0;
        crTexture*          dstImage = nullptr;
    };

    crFramebuffer( void );
    ~crFramebuffer( void );
    void                Create( const createInfo_t &in_createInfo, const uint32_t in_frames );
    void                Destroy( void );
    void                Bind( void );
    void                Unbind( void );
    void                ReadPixels( int32_t x, int32_t y, uint32_t width, uint32_t height, VkFormat format, void * data );
    void                DrawPixels(	uint32_t width, uint32_t height, VkFormat format, const void * data);
    void                BlitColorAttachament( const blitInfo_t &in_blitInfo );
    void                BlitDepthStencilAttachament( const blitInfo_t &in_blitInfo );
    ID_INLINE void      SwapFrame( void ) { m_bufferID = ( m_bufferID + 1 ) % m_frameCount; }
    crTexture*          ImageColor( void ) const { return const_cast<crTexture*>( &m_colorAttachament[m_bufferID] ); }
    crTexture*          ImageDepthStencil( void ) const { return const_cast<crTexture*>( &m_depthAttachament[m_bufferID] ); }

private:
    uint16_t                                m_bufferID;
    uint16_t                                m_frameCount;
    createInfo_t                            m_properties;
    VkDeviceMemory                          m_unifiedMemory;
    idStaticList<crTexture, MAX_SMP_FRAMES> m_colorAttachament;
    idStaticList<crTexture, MAX_SMP_FRAMES> m_depthAttachament;
};

#endif //!__FRAMEBUFFER_HPP__