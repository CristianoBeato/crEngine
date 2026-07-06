/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#ifndef GWEN_CONTROLS_IMAGEPANEL_H
#define GWEN_CONTROLS_IMAGEPANEL_H

namespace Gwen
{
	namespace Controls
	{
		class GWEN_EXPORT ImagePanel : public Controls::Base
		{
			public:

				GWEN_CONTROL( ImagePanel, Controls::Base );
				virtual ~ImagePanel( void );

				virtual void SetUV( float u1, float v1, float u2, float v2 );
				virtual void SetImage( const TextObject & imageName );

				virtual TextObject & GetImage( void )
				{
					return m_Texture.name;
				}

				virtual int TextureWidth( void )
				{
					return m_Texture.width;
				}

				virtual int TextureHeight( void )
				{
					return m_Texture.height;
				}

				virtual const TextObject & GetImageName()
				{
					return m_Texture.name;
				}

				virtual void Render( Skin::Base* skin )
				{
					skin->GetRender()->SetDrawColor( m_DrawColor );

					if ( m_bStretch )
					{ skin->GetRender()->DrawTexturedRect( &m_Texture, GetRenderBounds(), m_uv[0], m_uv[1], m_uv[2], m_uv[3] ); }
					else
					{ skin->GetRender()->DrawTexturedRect( &m_Texture, Gwen::Rect( 0, 0, m_Texture.width, m_Texture.height ), m_uv[0], m_uv[1], m_uv[2], m_uv[3] ); }
				}

				virtual void SizeToContents( void )
				{
					SetSize( m_Texture.width, m_Texture.height );
				}

				virtual void SetDrawColor( Gwen::Color color )
				{
					m_DrawColor = color;
				}

				virtual bool FailedToLoad( void )
				{
					return m_Texture.FailedToLoad();
				}

				virtual bool GetStretch( void ) { return m_bStretch; }
				virtual void SetStretch( bool b ) { m_bStretch = b; }

			protected:
				bool			m_bStretch;
				float			m_uv[4];
				Texture			m_Texture;
				Gwen::Color		m_DrawColor;


		};
	}
}
#endif
