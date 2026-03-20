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

#ifndef __BACKEND_HPP__
#define __BACKEND_HPP__

enum vertexLayoutType_t
{
	LAYOUT_UNKNOWN = 0,
	LAYOUT_DRAW_VERT,
	LAYOUT_DRAW_SHADOW_VERT,
	LAYOUT_DRAW_SHADOW_VERT_SKINNED
};

struct beState_t
{	
	uint32_t			currenttmu;
	int					faceCulling;
	
	vertexLayoutType_t	vertexLayout;
	
	// RB: 64 bit fixes, changed unsigned int to uintptr_t
	vkBufferHandle_t*	currentVertexBuffer;
	vkBufferHandle_t*	currentIndexBuffer;
	// RB end
	
	float				polyOfsScale;
	float				polyOfsBias;
	
	uint64_t				glStateBits;
};

struct backEndCounters_t
{
	int		c_surfaces;
	int		c_shaders;
	
	int		c_drawElements;
	int		c_drawIndexes;
	
	int		c_shadowElements;
	int		c_shadowIndexes;
	
	int		c_copyFrameBuffer;
	
	float	c_overDraw;
	
	int		totalMicroSec;			// total microseconds for backend run
	int		shadowMicroSec;
};

/*
=============================================================

RENDERER BACK END COMMAND QUEUE

TR_CMDS

=============================================================
*/
enum renderCommand_t
{
	RC_NOP,
	RC_DRAW_VIEW_3D,	// may be at a reduced resolution, will be upsampled before 2D GUIs
	RC_DRAW_VIEW_GUI,	// not resolution scaled
	RC_SET_BUFFER,
	RC_COPY_RENDER,
	RC_POST_PROCESS,
};

struct emptyCommand_t
{
	renderCommand_t		commandId;
	renderCommand_t* 	next;
};

struct setBufferCommand_t
{
	renderCommand_t		commandId;
	renderCommand_t* 	next;
//	GLenum	buffer;
};

struct drawSurfsCommand_t
{
	renderCommand_t		commandId;
	renderCommand_t* 	next;
	viewDef_t* 			viewDef;
};

struct copyRenderCommand_t
{
	renderCommand_t		commandId;
	renderCommand_t* 	next;
	int					x;
	int					y;
	int					imageWidth;
	int					imageHeight;
	idImage*				image;
	int					cubeFace;					// when copying to a cubeMap
	bool				clearColorAfterCopy;	
};

struct postProcessCommand_t
{
	renderCommand_t		commandId;
	renderCommand_t* 	next;
	viewDef_t* 			viewDef;
};

// all state modified by the back end is separated
// from the front end state
class crSwapchain;
class crCommandBuffer;
class crShaderStorage;
class crBackend
{
public:
    crBackend( void );
    ~crBackend( void );
	static crBackend* Get( void );
	const uint32_t		FrameID( void ) const { return m_frameID; }

    bool    			StartUp( const uint8_t in_samples, const uint32_t in_width, const uint32_t in_heigth );
	bool				SetScreenParms( const uint8_t in_samples, const uint32_t in_width, const uint32_t in_heigth );
    void    			ShutDown( void );
	void 				ExecuteBackEndCommands( const emptyCommand_t* cmds );
	float				ViewDefTime( void ) const { return viewDef != nullptr ? Sys_Milliseconds() : -1; }
	void				SetDefaultState( void );
	void				ZeroPerformanceCounters( void );
	backEndCounters_t	&PerformanceCounters( void ) { return pc; }

	// surfaces used for code-based drawing
	drawSurf_t			unitSquareSurface;
	drawSurf_t			zeroOneCubeSurface;
	drawSurf_t			testImageSurface;

private:
	const viewDef_t*	viewDef;
	backEndCounters_t	pc;
	
	const viewEntity_t* currentSpace;			// for detecting when a matrix must change
	idScreenRect		currentScissor;			// for scissor clipping, local inside renderView viewport
	beState_t			trState;				// for OpenGL state deltas
	
	bool				currentRenderCopied;	// true if any material has already referenced _currentRender
	bool				glowRenderCopied;		// foresthale 2014-04-21: true if the glow buffer has been updated this frame, used instead of r_glowEnable checks because that may be on for a 2D scene where the last glow update was in the past
	
	idRenderMatrix		prevMVP[2];				// world MVP from previous frame for motion blur, per-eye
	idRenderMatrix		shadowV[6];				// shadow depth view matrix
	idRenderMatrix		shadowP[6];				// shadow depth projection matrix
	
	
	/// @brief  frame control
    idList<vkImageHandle_t, TAG_VULKAN>		m_presentImages;

	uint32_t			m_frameID;		/// current frame buffers parity
	uint64_t			m_frame;		/// current rendering frame number
	VkClearValue 		m_clearValues;
	vkFramebuffer*		m_defaultFB;		

	void	DrawFlickerBox( void );
	void	SetBuffer( const void* data );
	void	StereoRenderExecuteBackEndCommands( const emptyCommand_t* const allCmds );
	void 	DrawElementsWithCounters( const drawSurf_t* surf );

	/*
	============================================================
	BACKEND
	============================================================
	*/
	void 	CopyRender( const void* data );
	void	PostProcess( const void* data );
	void 	DrawView( const void* data, const int stereoEye );
	void 	DrawViewInternal( const viewDef_t* viewDef, const int stereoEye, const bool skipNoBlur = true );	// sikk - Added 'skipNoBlur' arg - No Motionblur Material Stage Flag
	void	WaitForEndFrame( void );	// wait for the GPU to reach the last end frame marker
	void 	SetMVP( const idRenderMatrix& mvp );
	void	SetVertexColorParms( stageVertexColor_t svc );
	void	Cull( const cullType_t cullType );
	void	Flush( void );				// flush the GPU command buffer
	void	Finish( void );				// wait for the GPU to have executed all commands
	void	Color( float * color );
	void	Color( float r, float g, float b );
	void	Color( float r, float g, float b, float a );
	void	PolygonOffset( const float scale, const float bias );
	void	DepthBoundsTest( const float zmin, const float zmax );
	void	Clear( bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a );
	void	Scissor( const int x /* left*/, const int y /* bottom */, const int w, const int h );
	void	Viewport( const int x /* left */, const int y /* bottom */, const int w, const int h );
	
	
	
	// wait for the GPU to reach the last end frame marker
	void	WaitForEndFrame( void );

	ID_INLINE void	Scissor( const idScreenRect& rect );
	ID_INLINE void	Viewport( const idScreenRect& rect );
	ID_INLINE void	ViewportAndScissor( const int x, const int y, const int w, const int h );
	ID_INLINE void	ViewportAndScissor( const idScreenRect& rect );
	
};

ID_INLINE void crBackend::Scissor( const idScreenRect& in_rect )
{
	VkRect2D rect{};
	auto cmd = m_graphicCommandBuffer->CommandBuffer();
    rect.offset.x = in_rect.x1;
    rect.offset.y = in_rect.y1;
    rect.extent.width = in_rect.GetWidth();
    rect.extent.height = in_rect.GetHeight();
    vkCmdSetScissor( cmd, 0, 1, &rect );		
}

ID_INLINE void crBackend::Viewport( const idScreenRect& rect )
{
	VkViewport viewport{};
	auto cmd = m_graphicCommandBuffer->CommandBuffer();
	float x = static_cast<float>( rect.x1 );
	float y = static_cast<float>( rect.y1 );
	float w = static_cast<float>( rect.GetWidth() );
	float h = static_cast<float>( rect.GetHeight() );
    viewport.x = x;
    viewport.y = y + h;
    viewport.width = w;
    viewport.height = -std::abs(h);;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport( cmd, 0, 1, &viewport );
}

ID_INLINE void crBackend::ViewportAndScissor( int x, int y, int w, int h )
{
	Viewport( x, y, w, h );
	Scissor( x, y, w, h );
}

ID_INLINE void crBackend::ViewportAndScissor( const idScreenRect& rect )
{
	Viewport( rect );
	Scissor( rect );
}

#endif //!__BACKEND_HPP__