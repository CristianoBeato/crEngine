/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2025 Cristiano B. Santos

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef __RENDERER_LOCAL_H__
#define __RENDERER_LOCAL_H__

/// @brief Most renderer globals are defined here.
/// backend functions should never modify any of these fields,
/// but may read fields that aren't dynamically modified
/// by the frontend.
class idRenderSystemLocal : public idRenderSystem
{
public:
	// external functions
	virtual void			Init( void );
	virtual void			Shutdown( void );
	virtual void			ResetGuiModels( void );
	virtual void			InitRenderAPI( void );
	virtual void			ShutdownRenderAPI( void );
	virtual bool			IsRenderAPIRunning( void ) const;
	virtual bool			IsFullScreen( void ) const;
	virtual stereo3DMode_t	GetStereo3DMode( void ) const;
	virtual bool			HasQuadBufferSupport( void ) const;
	virtual bool			IsStereoScopicRenderingSupported( void ) const;
	virtual stereo3DMode_t	GetStereoScopicRenderingMode( void ) const;
	virtual void			EnableStereoScopicRendering( const stereo3DMode_t mode ) const;
	virtual uint32_t		GetWidth( void ) const;
	virtual uint32_t		GetHeight( void ) const;
	virtual float			GetPixelAspect() const;
	virtual float			GetPhysicalScreenWidthInCentimeters() const;
	virtual idRenderWorld* 	AllocRenderWorld();
	virtual void			FreeRenderWorld( idRenderWorld* rw );
	virtual void			BeginLevelLoad();
	virtual void			EndLevelLoad();
	virtual void			LoadLevelImages();
	virtual void			Preload( const idPreloadManifest& manifest, const char* mapName );
	virtual void			BeginAutomaticBackgroundSwaps( autoRenderIconType_t icon = AUTORENDER_DEFAULTICON );
	virtual void			EndAutomaticBackgroundSwaps();
	virtual bool			AreAutomaticBackgroundSwapsRunning( autoRenderIconType_t* usingAlternateIcon = nullptr ) const;
	
// BEATO Begin:

	/// @brief Resize framebuffers and swapchain
	/// @param in_width 
	/// @param in_height 
	virtual void			UpdateRenderSize( const uint32_t in_width, const uint32_t in_height );
// BEATO End

	virtual idFont* 		RegisterFont( const char* fontName );
	virtual void			ResetFonts();
	virtual void			PrintMemInfo( MemInfo_t* mi );
	
	virtual void			SetColor( const idVec4& color );
	virtual uint32_t		GetColor( void );
	virtual void			SetGLState( const uint64_t glState ) ;
	virtual void			DrawFilled( const idVec4& color, float x, float y, float w, float h );
	virtual void			DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const idMaterial* material );
	virtual void			DrawStretchPic( const idVec4& topLeft, const idVec4& topRight, const idVec4& bottomRight, const idVec4& bottomLeft, const idMaterial* material );
	virtual void			DrawStretchTri( const idVec2& p1, const idVec2& p2, const idVec2& p3, const idVec2& t1, const idVec2& t2, const idVec2& t3, const idMaterial* material );
	virtual idDrawVert* 	AllocTris( int numVerts, const triIndex_t* indexes, int numIndexes, const idMaterial* material, const stereoDepthType_t stereoType = STEREO_DEPTH_TYPE_NONE );
	virtual void			DrawSmallChar( int x, int y, int ch );
	virtual void			DrawSmallStringExt( int x, int y, const char* string, const idVec4& setColor, bool forceColor );
	virtual void			DrawBigChar( int x, int y, int ch );
	virtual void			DrawBigStringExt( int x, int y, const char* string, const idVec4& setColor, bool forceColor );
	
	virtual void			WriteDemoPics();
	virtual void			WriteEndFrame();
	virtual void			DrawDemoPics();
	virtual const emptyCommand_t* 	SwapCommandBuffers( uint64_t* frontEndMicroSec, uint64_t* backEndMicroSec, uint64_t* shadowMicroSec, uint64_t* gpuMicroSec );
	// foresthale 2014-05-19: the editor views need some wrapper code to set up a view render and restore state afterward so that the fixed function OpenGL code of the editors keep working
	virtual void			Editor_SetupState();
	virtual void			Editor_BeginView(int width, int height, int &restoreWidth, int &restoreHeight);
	virtual void			Editor_EndView(int restoreWidth, int restoreHeight);
	
	virtual void			SwapCommandBuffers_FinishRendering( uint64_t* frontEndMicroSec, uint64_t* backEndMicroSec, uint64_t* shadowMicroSec, uint64_t* gpuMicroSec );
	virtual const emptyCommand_t* 	SwapCommandBuffers_FinishCommandBuffers();
	
	virtual void			RenderCommandBuffers( const emptyCommand_t* commandBuffers );
	virtual void			TakeScreenshot( int width, int height, const char* fileName, int downSample, renderView_t* ref );
	virtual void			TakeScreenshot( int width, int height, idFile* outFile, int blends, renderView_t* ref );
	virtual void			CropRenderSize( int width, int height );
	virtual void			CaptureRenderToImage( const char* imageName, bool clearColorAfterCopy = false );
	virtual void			CaptureRenderToFile( const char* fileName, bool fixAlpha );
	virtual void			UnCrop();
	virtual bool			UploadImage( const char* imageName, const byte* data, int width, int height );

	/// get current vulkan device
	crVulkanRenderDevicep	GetRenderDevice( void ) { return dynamic_cast<crVulkanRenderDevicep>( m_renderDevice ); }
	vkSwapchainp 			Swapchain( void ) const { return m_swapchain; }
	vkCommandbufferp 		GraphicCommandBuffer( void ) const { return m_graphicCommandBuffer; }

public:
	// internal functions
	idRenderSystemLocal();
	~idRenderSystemLocal();
	void					InitDevice( void );
	void					CheckPortableExtensions( void );
	
	/// Present render to screen
	void					Present( void );

	/// Begin register frame commands
	// inserts a timing mark for the start of the GPU frame
	void					StartFrame( const uint64_t in_frame ); 

	/// End registr frame commands
	// inserts a timing mark for the end of the GPU frame
	void					EndFrame( void );

	void					Clear( void );
	void					GetCroppedViewport( idScreenRect* viewport );
	void					PerformResolutionScaling( int& newWidth, int& newHeight );
	int						GetFrameCount() const
	{
		return frameCount;
	};

	void OnFrame();
	
public:
	// renderer globals
	bool					registered;		// cleared at shutdown, set at InitOpenGL
	
	bool					takingScreenshot;
	
	int						frameCount;		// incremented every frame
	int						viewCount;		// incremented every view (twice a scene if subviewed)
	// and every R_MarkFragments call
	
	float					frameShaderTime;	// shader time for all non-world 2D rendering
	
	idVec4					ambientLightVector;	// used for "ambient bump mapping"
	
	idList<idRenderWorldLocal*>worlds;
	
// BEATO Begin:
	// list that store render devices list
	idList<crRenderDevicep>			m_renderDeviceList;
	crRenderDevicep					m_renderDevice;
	vkSwapchain*					m_swapchain;
	vkCommandbuffer*				m_graphicCommandBuffer;
	vkTimeQueries*					m_timerQuery;
// BEATO End

	idRenderWorldLocal* 	primaryWorld;
	renderView_t			primaryRenderView;
	viewDef_t* 				primaryView;

	// many console commands need to know which world they should operate on
	const idMaterial* 		whiteMaterial;
	const idMaterial* 		charSetMaterial;
	const idMaterial* 		defaultPointLight;
	const idMaterial* 		defaultProjectedLight;
	const idMaterial* 		defaultMaterial;
	idImage* 				testImage;
	idCinematic* 			testVideo;
	int						testVideoStartTime;
	

	idImage* 				ambientCubeImage;	// hack for testing dependent ambient lighting
	
	viewDef_t* 				viewDef;
	
	performanceCounters_t	pc;					// performance counters
	
	viewEntity_t			identitySpace;		// can use if we don't know viewDef->worldSpace is valid
	
	idScreenRect			renderCrops[MAX_RENDER_CROPS];
	int						currentRenderCrop;
	
	// GUI drawing variables for surface creation
	int						guiRecursionLevel;		// to prevent infinite overruns
	uint32_t				currentColorNativeBytesOrder;
	uint64_t				currentGLState;
	class idGuiModel* 		guiModel;
	
	idList<idFont*, TAG_FONT>		fonts;
	
	unsigned short			gammaTable[256];	// brightness / gamma modify this
	
	crDrawGeometry* 		unitSquareTriangles;
	crDrawGeometry* 		zeroOneCubeTriangles;
	crDrawGeometry* 		testImageTriangles;
	
	// these are allocated at buffer swap time, but
	// the back end should only use the ones in the backEnd stucture,
	// which are copied over from the frame that was just swapped.
	drawSurf_t				unitSquareSurface_;
	drawSurf_t				zeroOneCubeSurface_;
	drawSurf_t				testImageSurface_;
	
	idParallelJobList* 		frontEndJobList;

	// foresthale 2014-03-01: screenshots need to override the results of GetWidth() and GetHeight()
	int						screenshotOverrideWidth;
	int						screenshotOverrideHeight;
};

extern idRenderSystemLocal	tr;

#endif //!__RENDERER_LOCAL_H__