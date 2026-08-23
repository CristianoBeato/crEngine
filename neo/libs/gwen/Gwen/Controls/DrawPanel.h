
#ifndef __GWEN_DRAW_PANEL_H__
#define __GWEN_DRAW_PANEL_H__

namespace Gwen
{
	namespace Controls
	{
        class GWEN_EXPORT DrawPanel : public Controls::Base
        {
        public:     
            GWEN_CONTROL( DrawPanel, Controls::Base );
            ~DrawPanel( void );

            virtual void Initialize( void );
            virtual void Render( Skin::Base* skin );
			virtual void OnBoundsChanged( Gwen::Rect oldBounds );
			// Background
			virtual void SetBackgroundColor( const Gwen::Color & color );

            virtual bool InputMouseMoved( int x, int y, int deltaX, int deltaY ) { return false; }
			virtual bool InputMouseButton( int iButton, bool bDown ) { return false; }
			virtual bool InputKey( int iKey, bool bDown ) { return false; }
			virtual bool InputCharacter( Gwen::UnicodeChar chr ) { return false; }
			virtual bool InputMouseWheel( int val ) { return false; }

            virtual void Draw( void ) {};

            virtual void SizeToContents( void )
			{
                SetSize( m_framebuffer.width, m_framebuffer.height );
			}
            
            void    SetSamples( const uint8_t in_samples ) { m_samples = in_samples; }

        private:
            uint8_t             m_samples;
            Gwen::FrameBuffer   m_framebuffer;
            Gwen::Color         m_backgroundColor;
        };
    };
};

#endif //__GWEN_DRAW_PANEL_H__