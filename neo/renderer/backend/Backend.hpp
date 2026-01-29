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

inline constexpr int MAX_MULTITEXTURE_UNITS = 8;

enum vertexLayoutType_t
{
	LAYOUT_UNKNOWN = 0,
	LAYOUT_DRAW_VERT,
	LAYOUT_DRAW_SHADOW_VERT,
	LAYOUT_DRAW_SHADOW_VERT_SKINNED
};

struct tmu_t
{
	crSampler*	current2DSamp;
	crSampler*	current2DArraySamp;
	crSampler*	currentCubeMapSamp;
	crTexture*	current2DMap;
	crTexture*	current2DArray;
	crTexture*	currentCubeMap;
};

struct trState_t
{
	tmu_t				tmu[MAX_MULTITEXTURE_UNITS];
	
	uint32_t			currenttmu;
	int					faceCulling;
	
	vertexLayoutType_t	vertexLayout;
	
	// RB: 64 bit fixes, changed unsigned int to uintptr_t
	uintptr_t			currentVertexBuffer;
	uintptr_t			currentIndexBuffer;
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

    void    StartUp( void );
    void    ShutDown( void );

	void 	ExecuteBackEndCommands( const emptyCommand_t* cmds );
	void 	DrawView( const void* data, const int stereoEye );
	void	CopyRender( const void* data );

	/*
	============================================================
	BACKEND
	============================================================
	*/
	
	/*
	============================================================
	BACKEND_DRAW
	============================================================
	*/
	void SelectTexture( uint32_t unit );
	void SetMVP( const idRenderMatrix& mvp );
	void DrawElementsWithCounters( const drawSurf_t* surf );
	void DrawViewInternal( const viewDef_t* viewDef, const int stereoEye, const bool skipNoBlur = true );	// sikk - Added 'skipNoBlur' arg - No Motionblur Material Stage Flag
	void DrawView( const void* data, const int stereoEye );
	void CopyRender( const void* data );
	void PostProcess( const void* data );

	/*
	=============================================================
	BACKEND_RENDERTOOLS
	=============================================================
	*/
	float	DrawTextLength( const char* text, float scale, int len );
	void	AddDebugText( const char* text, const idVec3& origin, float scale, const idVec4& color, const idMat3& viewAxis, const int align, const int lifetime, const bool depthTest );
	void	ClearDebugText( int time );
	void	AddDebugLine( const idVec4& color, const idVec3& start, const idVec3& end, const int lifeTime, const bool depthTest );
	void	ClearDebugLines( int time );
	void	AddDebugPolygon( const idVec4& color, const idWinding& winding, const int lifeTime, const bool depthTest );
	void	ShowDebugPolygons( void );
	void	ClearDebugPolygons( int time );
	void	DrawBounds( const idBounds& bounds );
	void	ShowLights( drawSurf_t** drawSurfs, int numDrawSurfs );
	void	ShowLightCount( drawSurf_t** drawSurfs, int numDrawSurfs );
	void	PolygonClear();
	void	ScanStencilBuffer();
	void	ShowDestinationAlpha();
	void	ShowOverdraw();
	void	RenderDebugTools( drawSurf_t** drawSurfs, int numDrawSurfs );
	void	ShutdownDebugTools();
	void	SetVertexColorParms( stageVertexColor_t svc );
	void	ShowTrace( drawSurf_t** drawSurfs, int numDrawSurfs );
	void	MotionBlur( void );

    // TODO better acess
    const viewDef_t*	viewDef;
	backEndCounters_t	pc;
	
	const viewEntity_t* currentSpace;			// for detecting when a matrix must change
	idScreenRect		currentScissor;			// for scissor clipping, local inside renderView viewport
	trState_t			trState;				// for OpenGL state deltas
	
	bool				currentRenderCopied;	// true if any material has already referenced _currentRender
	bool				glowRenderCopied;		// foresthale 2014-04-21: true if the glow buffer has been updated this frame, used instead of r_glowEnable checks because that may be on for a 2D scene where the last glow update was in the past
	
	idRenderMatrix		prevMVP[2];				// world MVP from previous frame for motion blur, per-eye

	// RB begin
	idRenderMatrix		shadowV[6];				// shadow depth view matrix
	idRenderMatrix		shadowP[6];				// shadow depth projection matrix
	// RB end

	// surfaces used for code-based drawing
	drawSurf_t			unitSquareSurface;
	drawSurf_t			zeroOneCubeSurface;
	drawSurf_t			testImageSurface;

	crShaderStorage*			GetShaderStorage( void ) const { return m_shaderStorage; }
	crGraphicCommandBuffer*		GetRenderCMD( void ) const { return m_renderCMD; }
	crTransferCommandBuffer*	GetTransferCMD( void ) const { return m_transferCMD;}
	crCommandBuffer*			GetComputeCMD( void ) const { return m_computeCMD; }

private:
	crSwapchain*				m_swapchain;		//
    crGraphicCommandBuffer*    	m_renderCMD;        // main render command buffer
	crTransferCommandBuffer*	m_transferCMD;		// resgister copy commands
    crCommandBuffer*			m_computeCMD;		// register the commands used in compute operations
    crShaderStorage*    		m_shaderStorage;    // shader storage blocks 

	/// Beckend.cpp
	void	DrawFlickerBox( void );
	void	SetBuffer( const void* data );
	void	BlockingSwapBuffers( void );
	void	StereoRenderExecuteBackEndCommands( const emptyCommand_t* const allCmds );

	/// Backend_draw.cpp
	void	PrepareStageTexturing( const shaderStage_t* pStage,  const drawSurf_t* surf );
	void	FinishStageTexturing( const shaderStage_t* pStage, const drawSurf_t* surf );
	void	FillDepthBufferGeneric( const drawSurf_t* const* drawSurfs, int numDrawSurfs );
	void 	FillDepthBufferFast( drawSurf_t** drawSurfs, int numDrawSurfs );
	void	DrawInteractions( const viewDef_t* viewDef );
	void	DrawSingleInteraction( drawInteraction_t* din );
	void	RenderInteractions( const drawSurf_t* surfList, const viewLight_t* vLight, int depthFunc, bool performStencilTest, bool useLightDepthBounds );
	int		DrawShaderPasses( const drawSurf_t* const* const drawSurfs, const int numDrawSurfs, const float guiStereoScreenOffset, const int stereoEye, const int stopSort, const bool skipNoBlur, const bool glowStage );
	void	StencilShadowPass( const drawSurf_t* drawSurfs, const viewLight_t* vLight );
	void	StencilSelectLight( const viewLight_t* vLight );
	void	ShadowMapPass( const drawSurf_t* drawSurfs, const viewLight_t* vLight, int side );
	void 	BlendLight( const drawSurf_t* drawSurfs, const viewLight_t* vLight );
	void 	BlendLight( const drawSurf_t* drawSurfs, const drawSurf_t* drawSurfs2, const viewLight_t* vLight, bool glowStage );
	void	BasicFog( const drawSurf_t* drawSurfs, const idPlane fogPlanes[4], const idRenderMatrix* inverseBaseLightProject );
	void 	FogPass( const drawSurf_t* drawSurfs,  const drawSurf_t* drawSurfs2, const viewLight_t* vLight, bool glowStage );
	void	FogAllLights( const bool glowStage );
	void	PostProcess( const void* data );
	void	PostProcessHDRGlowProcess(int textureSizes[4][6]);

	void	DrawViewInternal( const viewDef_t* viewDef, const int stereoEye, const bool skipNoBlur );

	void	DrawElementsWithCounters( const drawSurf_t* surf );

	void	ResetViewportAndScissorToDefaultCamera( const viewDef_t* viewDef );

	ID_INLINE void	PolygonOffset( const float scale, const float bias );
	ID_INLINE void	DepthBoundsTest( const float zmin, const float zmax );
	ID_INLINE void	Scissor( const int x /* left*/, const int y /* bottom */, const int w, const int h );
	ID_INLINE void	Viewport( const int x /* left */, const int y /* bottom */, const int w, const int h );
	ID_INLINE void	Scissor( const idScreenRect& rect );
	ID_INLINE void	Viewport( const idScreenRect& rect );
	ID_INLINE void	ViewportAndScissor( const int x, const int y, const int w, const int h );
	ID_INLINE void	ViewportAndScissor( const idScreenRect& rect );
	ID_INLINE void	Clear( bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a );
	ID_INLINE void	Cull( const cullType_t cullType );

};

/*
====================
crBackend::PolygonOffset
====================
*/
void crBackend::PolygonOffset( const float scale, const float bias )
{
	trState.polyOfsScale = scale;
	trState.polyOfsBias = bias;
    m_renderCMD->PolygonOffset( scale, bias, trState.glStateBits & GLS_POLYGON_OFFSET );
}

/*
========================
crBackend::DepthBoundsTest
========================
*/
void crBackend::DepthBoundsTest( const float zmin, const float zmax )
{
	if( !glConfig.depthBoundsTestAvailable || zmin > zmax )
		return;
        
    m_renderCMD->DepthBoundsTest( zmin, zmax, ( zmin == 0.0f && zmax == 0.0f ) ? false : true );	
}

ID_INLINE void crBackend::Scissor( const int x /* left*/, const int y /* bottom */, const int w, const int h )
{
	m_renderCMD->Scissor( x, y, w, h );
}

ID_INLINE void crBackend::Viewport( const int x /* left */, const int y /* bottom */, const int w, const int h )
{
	m_renderCMD->Viewport( x, y, w, h );
}	

ID_INLINE void crBackend::Scissor( const idScreenRect& rect )
{
	m_renderCMD->Scissor( rect );
}

ID_INLINE void crBackend::Viewport( const idScreenRect& rect )
{
	m_renderCMD->Viewport( rect );
}

ID_INLINE void crBackend::ViewportAndScissor( int x, int y, int w, int h )
{
	m_renderCMD->Viewport( x, y, w, h );
	m_renderCMD->Scissor( x, y, w, h );
}

ID_INLINE void crBackend::ViewportAndScissor( const idScreenRect& rect )
{
	m_renderCMD->Viewport( rect );
	m_renderCMD->Scissor( rect );
}

ID_INLINE void crBackend::Clear(bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a)
{
	m_renderCMD->Clear( color, depth, stencil, stencilValue, r, g, b, a );
}

/*
====================
crBackend::Cull

This handles the flipping needed when the view being
rendered is a mirored view.
====================
*/
ID_INLINE void crBackend::Cull( const cullType_t cullType )
{
	crPipeline::Face_t faceCull;
	if( trState.faceCulling == cullType )
		return;

	if( cullType == CT_TWO_SIDED )
		faceCull = crPipeline::FC_TWO_FACES;
	else
	{		
		if( cullType == CT_BACK_SIDED )
		{
			if( viewDef->isMirror )
				faceCull = crPipeline::FC_FRONT;
			else
				faceCull = crPipeline::FC_BACK;
		}
		else
		{
			if( viewDef->isMirror )
				faceCull = crPipeline::FC_BACK;
			else
				faceCull = crPipeline::FC_FRONT;
		}
	}

	m_renderCMD->FaceCull( faceCull );
	trState.faceCulling = cullType;
}

extern crBackend			backEnd;

#endif //!__BACKEND_HPP__