
#ifndef __GWEN_FRAMEBUFFER_H__
#define __GWEN_FRAMEBUFFER_H__

namespace Gwen
{
    //
    // Framebuffer
    //
    struct FrameBuffer
    {
        void*	data;
		bool	failed;
		int     samples;
        int		width;
		int		height;

        FrameBuffer( void )
        {
            data = nullptr;
            failed = true;
            samples = 0;
            width = 0;
            height = 0;
        }

        ~FrameBuffer( void )
        {
        }

        void Allocate( Gwen::Renderer::Base* pRender )
        {
            pRender->CreateFrameBuffer( this );
        }

        void Release( Gwen::Renderer::Base* pRender )
        {
            pRender->FreeFrameBuffer( this );
        }

    };
};

#endif //!__GWEN_FRAMEBUFFER_H__