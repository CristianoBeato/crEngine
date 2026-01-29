
#include "precompiled.h"
#include "renderer_common.h"
#include "glSwapchain.hpp"

glSwapchain::glSwapchain( const uint32_t in_width, const uint32_t in_height ) : crSwapchain( in_width, in_height ) 
{
    //
    glCreateRenderbuffers( SMP_FRAMES, m_renderbuffers.Ptr() );
    glCreateRenderbuffers( SMP_FRAMES, m_depthStencil.Ptr() );
    glCreateBuffers( SMP_FRAMES, m_framebuffers.Ptr() );

    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        /// create color buffer
        glNamedRenderbufferStorage( m_renderbuffers[i], GL_SRGB8_ALPHA8, m_width, m_height );

        /// create sample buffer
        glNamedRenderbufferStorage( m_depthStencil[i], GL_DEPTH24_STENCIL8, m_width, m_height );

        /// attach color buffer
        glNamedFramebufferRenderbuffer( m_framebuffers[i], GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderbuffers[i] );

        /// attach depth stencil color buffer
        glNamedFramebufferRenderbuffer( m_framebuffers[i], GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencil[i] );
        
        if( glCheckNamedFramebufferStatus( m_framebuffers[i], GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
            common->FatalError( "Swapchain frame buffer failed glCheckNamedFramebufferStatus" );
    }
}

glSwapchain::~glSwapchain( void )
{
    glDeleteFramebuffers( SMP_FRAMES, m_framebuffers.Ptr() );
    glDeleteFramebuffers( SMP_FRAMES, m_depthStencil.Ptr() );
    glDeleteRenderbuffers( SMP_FRAMES, m_renderbuffers.Ptr() );
}

bool glSwapchain::Recreate(const uint32_t in_width, const uint32_t in_height)
{
    glDeleteRenderbuffers( SMP_FRAMES, m_framebuffers.Ptr() );
    glDeleteRenderbuffers( SMP_FRAMES, m_depthStencil.Ptr() );
    glCreateRenderbuffers( SMP_FRAMES, m_framebuffers.Ptr() );
    glCreateRenderbuffers( SMP_FRAMES, m_depthStencil.Ptr() );
    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        /// create color buffer
        glNamedRenderbufferStorage( m_renderbuffers[i], GL_SRGB8_ALPHA8, m_width, m_height );

        /// create sample buffer
        glNamedRenderbufferStorage( m_depthStencil[i], GL_DEPTH24_STENCIL8, m_width, m_height );
     
        /// attach color buffer
        glNamedFramebufferRenderbuffer( m_framebuffers[i], GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderbuffers[i] );

        /// attach depth stencil color buffer
        glNamedFramebufferRenderbuffer( m_framebuffers[i], GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencil[i] );
        
        if( glCheckNamedFramebufferStatus( m_framebuffers[i], GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
            common->FatalError( "Swapchain frame buffer failed glCheckNamedFramebufferStatus" );
    }
}

void glSwapchain::AcquireImage(void)
{
    // bind the frame to draw
    glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffers[m_frame] );
}

void glSwapchain::PresentImage(void)
{
    // prsent to defalt frame buffer 
    glBlitNamedFramebuffer( m_framebuffers[m_frame], 0, 0, 0, m_width, m_height, 0, 0, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight, GL_COLOR_ATTACHMENT0, GL_NEAREST );
    m_frame = m_frame + 1 % SMP_FRAMES;
}
