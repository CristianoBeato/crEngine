
#ifndef __TOOLS_DIALOG_RENRER_HPP__
#define __TOOLS_DIALOG_RENRER_HPP__

#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"

class crToolsDialogRenderer : public Gwen::Renderer::Base
{
public:
    crToolsDialogRenderer( void );
    ~crToolsDialogRenderer( void );

    virtual void Init( void ) override;
	virtual void Release( void ) override;

    virtual void Begin( void ) override;
	virtual void End( void ) override;

	virtual void StartClip( void ) override;
	virtual void EndClip( void ) override;
				
	virtual void SetDrawColor( Gwen::Color color ) override;

	//virtual void DrawShavedCornerRect( Gwen::Rect rect, bool bSlight = false );
	virtual void DrawPixel( int x, int y ) override;
	virtual void DrawLinedRect( Gwen::Rect rect ) override;
	virtual void DrawFilledRect( Gwen::Rect rect ) override;
	virtual void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f ) override;
	virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text );
	virtual void LoadFont( Gwen::Font* pFont ) override;
	virtual void FreeFont( Gwen::Font* pFont ) override;
	virtual void LoadTexture( Gwen::Texture* pTexture ) override;
	virtual void FreeTexture( Gwen::Texture* pTexture ) override;
	virtual void CreateFrameBuffer( Gwen::FrameBuffer* pFrameBuffer ) override;
	virtual void FreeFrameBuffer( Gwen::FrameBuffer* pFrameBuffer ) override;
	virtual void BindFrameBuffer( Gwen::FrameBuffer* pFrameBuffer, Gwen::Rect renderRect ) override;
	virtual void DrawFrameBuffer( Gwen::FrameBuffer* pFrameBuffer, Gwen::Rect targetRect ) override;

	Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default ) override;
	Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString & text );

    //
	// Self Initialization
	//
	virtual bool InitializeContext( Gwen::WindowProvider* pWindow ) override {};
	virtual bool ShutdownContext( Gwen::WindowProvider* pWindow ) override {};
	virtual bool PresentContext( Gwen::WindowProvider* pWindow ) override {};
	virtual bool ResizedContext( Gwen::WindowProvider* pWindow, int w, int h ) override {};
	virtual bool BeginContext( Gwen::WindowProvider* pWindow ) override {};
	virtual bool EndContext( Gwen::WindowProvider* pWindow ) override {};

private:

    enum drawMode_t : uint8_t
	{
		RECT_LINE,
		RECT_FILL,
		RECT_TEXTURED,
        RECT_FONT
	};

    struct bounds_t
	{
        float	left = 0.0f;
        float	top = 0.0f;
        float	right = 0.0f;
        float	bottom = 0.0f;
    };			

    drawMode_t          m_currentMode;
    uint16_t			m_vhead;
    uint16_t			m_vtail;
    uint16_t			m_ihead;
    uint16_t			m_itail;
    int32_t				m_width;
    int32_t				m_heigth;
    uint32_t			m_vertexArray;
    uint32_t			m_program;
    uint32_t			m_vertexBuffer;
    uint32_t			m_elementBuffer;
    uint32_t			m_uniformBuffer;
    uint32_t			m_white;
    uint32_t			m_fontImage;				
    uint32_t			m_sample;
    uint32_t			m_fontSample;
    uint16_t*			m_elements;
    float*				m_vertexes;
				
	void        AddQuad( const bounds_t pos, const bounds_t uv );
	void        Flush( void );
				
	/// @brief Load OpenGL Shaders ( Future we implement as SpirV shader )
	void        InitShaders( void );

	/// @brief Create rendering buffers 
	void        InitBuffers( void );

	void        CreateSamplers( void );
	
	void        CreateVertexArray( void );
};

#endif //!__TOOLS_DIALOG_RENRER_HPP__