/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

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
#ifndef __RENDERER_H__
#define __RENDERER_H__

/*
===============================================================================

	idRenderSystem is responsible for managing the screen, which can have
	multiple idRenderWorld and 2D drawing done on it.

===============================================================================
*/
enum stereo3DMode_t
{
	STEREO3D_OFF,
	
	// half-resolution, non-square pixel views
	STEREO3D_SIDE_BY_SIDE_COMPRESSED,
	STEREO3D_TOP_AND_BOTTOM_COMPRESSED,
	
	// two full resolution views side by side, as for a dual cable display
	STEREO3D_SIDE_BY_SIDE,
	
	STEREO3D_INTERLACED,
	
	// OpenGL quad buffer
	STEREO3D_QUAD_BUFFER,
	
	// two full resolution views stacked with a 30 pixel guard band
	// On the PC this can be configured as a custom video timing, but
	// it definitely isn't a consumer level task.  The quad_buffer
	// support can handle 720P-3D with apropriate driver support.
	STEREO3D_HDMI_720
};

typedef enum
{
	AUTORENDER_DEFAULTICON = 0,
	AUTORENDER_HELLICON,
	AUTORENDER_DIALOGICON,
	AUTORENDER_MAX
} autoRenderIconType_t ;

enum stereoDepthType_t
{
	STEREO_DEPTH_TYPE_NONE,
	STEREO_DEPTH_TYPE_NEAR,
	STEREO_DEPTH_TYPE_MID,
	STEREO_DEPTH_TYPE_FAR
};

enum graphicsVendor_t
{
	VENDOR_NVIDIA,
	VENDOR_AMD,
	VENDOR_INTEL
};

enum backend_t
{
	BACKEND_OPENGL,
	BACKEND_VULKAN
};

// Contains variables specific to the OpenGL configuration being run right now.
// These are constant once the OpenGL subsystem is initialized.
struct glconfig_t
{
	const char* 		renderer_string;
	const char* 		vendor_string;
	const char* 		version_string;
	const char* 		extensions_string;
	const char* 		wgl_extensions_string;
	const char* 		shading_language_string;
	
	float				glVersion;				// atof( version_string )
	graphicsVendor_t	vendor;
// BEATO Begin: 
	backend_t			backend;		
// BEATO End

	int					maxTextureSize;			// queried from GL
	int					maxTextureCoords;
	int					maxTextureImageUnits;
	int					uniformBufferOffsetAlignment;
	float				maxTextureAnisotropy;
	
	int					colorBits;
	int					depthBits;
	int					stencilBits;
	
	bool				multitextureAvailable;
	bool				directStateAccess;
	bool				textureCompressionAvailable;
	bool				S3TCtextureCompressionAvailable;
	bool				anisotropicFilterAvailable;
	bool				textureLODBiasAvailable;
	bool				seamlessCubeMapAvailable;
	bool				sRGBFramebufferAvailable;
	bool				vertexBufferObjectAvailable;
	bool				mapBufferRangeAvailable;
	bool				vertexArrayObjectAvailable;
	bool				drawElementsBaseVertexAvailable;
	bool				glslAvailable;
	bool				frameBufferObjectAvailable;
	bool				textureFloatAvailable;
	bool				uniformBufferAvailable;
	bool				twoSidedStencilAvailable;
	bool				depthBoundsTestAvailable;
	bool				syncAvailable;
	bool				timerQueryAvailable;
	bool				occlusionQueryAvailable;
	bool				debugOutputAvailable;
	bool				swapControlTearAvailable;
	
	stereo3DMode_t		stereo3Dmode;
	int					nativeScreenWidth; // this is the native screen width resolution of the renderer
	int					nativeScreenHeight; // this is the native screen height resolution of the renderer
	
	int					displayFrequency;
	
	int					isFullscreen;					// monitor number
	bool				isStereoPixelFormat;
	bool				stereoPixelFormatAvailable;
	int					multisamples;
	
	// Screen separation for stereoscopic rendering is set based on this.
	// PC vid code sets this, converting from diagonals / inches / whatever as needed.
	// If the value can't be determined, set something reasonable, like 50cm.
	float				physicalScreenWidthInCentimeters;
	
	float				pixelAspect;
	
	uint32_t			global_vao;
};

struct emptyCommand_t;

const int SMALLCHAR_WIDTH		= 8;
const int SMALLCHAR_HEIGHT		= 16;
const int BIGCHAR_WIDTH			= 16;
const int BIGCHAR_HEIGHT		= 16;

// all drawing is done to a 640 x 480 virtual screen size
// and will be automatically scaled to the real resolution
const int SCREEN_WIDTH			= 640;
const int SCREEN_HEIGHT			= 480;

const int TITLESAFE_LEFT		= 32;
const int TITLESAFE_RIGHT		= 608;
const int TITLESAFE_TOP			= 24;
const int TITLESAFE_BOTTOM		= 456;
const int TITLESAFE_WIDTH		= TITLESAFE_RIGHT - TITLESAFE_LEFT;
const int TITLESAFE_HEIGHT		= TITLESAFE_BOTTOM - TITLESAFE_TOP;

//
// renderer
// cvars
//
extern idCVar r_debugContext;				// enable various levels of context debug
extern idCVar r_glDriver;					// "opengl32", etc
extern idCVar r_skipIntelWorkarounds;		// skip work arounds for Intel driver bugs
extern idCVar r_vidMode;					// video mode number
extern idCVar r_displayRefresh;				// optional display refresh rate option for vid mode
extern idCVar r_fullscreen;					// 0 = windowed, 1 = full screen
extern idCVar r_multiSamples;				// number of antialiasing samples

extern idCVar r_znear;						// near Z clip plane

extern idCVar r_swapInterval;				// changes wglSwapIntarval
extern idCVar r_offsetFactor;				// polygon offset parameter
extern idCVar r_offsetUnits;				// polygon offset parameter
extern idCVar r_singleTriangle;				// only draw a single triangle per primitive
extern idCVar r_logFile;					// number of frames to emit GL logs
extern idCVar r_clear;						// force screen clear every frame
extern idCVar r_subviewOnly;				// 1 = don't render main view, allowing subviews to be debugged
extern idCVar r_lightScale;					// all light intensities are multiplied by this, which is normally 2
extern idCVar r_flareSize;					// scale the flare deforms from the material def

extern idCVar r_gamma;						// changes gamma tables
extern idCVar r_brightness;					// changes gamma tables

extern idCVar r_checkBounds;				// compare all surface bounds with precalculated ones
extern idCVar r_maxAnisotropicFiltering;	// texture filtering parameter
extern idCVar r_useTrilinearFiltering;		// Extra quality filtering
extern idCVar r_lodBias;					// lod bias
extern idCVar r_useSRGB;					// foresthale 2014-02-20: fixed r_useSRGB texture handling
extern idCVar r_useHDR;						// foresthale 2014-02-20: HDR view rendering

extern idCVar r_useLightPortalFlow;			// 1 = do a more precise area reference determination
extern idCVar r_useShadowSurfaceScissor;	// 1 = scissor shadows by the scissor rect of the interaction surfaces
extern idCVar r_useConstantMaterials;		// 1 = use pre-calculated material registers if possible
extern idCVar r_useNodeCommonChildren;		// stop pushing reference bounds early when possible
extern idCVar r_useSilRemap;				// 1 = consider verts with the same XYZ, but different ST the same for shadows
extern idCVar r_useLightPortalCulling;		// 0 = none, 1 = box, 2 = exact clip of polyhedron faces, 3 MVP to plane culling
extern idCVar r_useLightAreaCulling;		// 0 = off, 1 = on
extern idCVar r_useLightScissors;			// 1 = use custom scissor rectangle for each light
extern idCVar r_useEntityPortalCulling;		// 0 = none, 1 = box
extern idCVar r_skipPrelightShadows;		// 1 = skip the dmap generated static shadow volumes
extern idCVar r_useCachedDynamicModels;		// 1 = cache snapshots of dynamic models
extern idCVar r_useScissor;					// 1 = scissor clip as portals and lights are processed
extern idCVar r_usePortals;					// 1 = use portals to perform area culling, otherwise draw everything
extern idCVar r_useStateCaching;			// avoid redundant state changes in GL_*() calls
extern idCVar r_useEntityCallbacks;			// if 0, issue the callback immediately at update time, rather than defering
extern idCVar r_lightAllBackFaces;			// light all the back faces, even when they would be shadowed
extern idCVar r_skipROQ;
extern idCVar r_useLightDepthBounds;		// use depth bounds test on lights to reduce both shadow and interaction fill
extern idCVar r_useShadowDepthBounds;		// use depth bounds test on individual shadows to reduce shadow fill
// RB begin
extern idCVar r_useShadowMapping;			// use shadow mapping instead of stencil shadows
// RB end
extern idCVar r_shadowMapMaxDistance;

extern idCVar r_skipStaticInteractions;		// skip interactions created at level load
extern idCVar r_skipDynamicInteractions;	// skip interactions created after level load
extern idCVar r_skipPostProcess;			// skip all post-process renderings
extern idCVar r_skipSuppress;				// ignore the per-view suppressions
extern idCVar r_skipInteractions;			// skip all light/surface interaction drawing
extern idCVar r_skipFrontEnd;				// bypasses all front end work, but 2D gui rendering still draws
extern idCVar r_skipBackEnd;				// don't draw anything
extern idCVar r_skipCopyTexture;			// do all rendering, but don't actually copyTexSubImage2D
extern idCVar r_skipRender;					// skip 3D rendering, but pass 2D
extern idCVar r_skipRenderContext;			// nullptr the rendering context during backend 3D rendering
extern idCVar r_skipTranslucent;			// skip the translucent interaction rendering
extern idCVar r_skipAmbient;				// bypasses all non-interaction drawing
extern idCVar r_skipNewAmbient;				// bypasses all vertex/fragment program ambients
extern idCVar r_skipInk;					// foresthale 2014-04-27: r_skipInk - bypasses all shader programs using _currentDepth
extern idCVar r_skipBlendLights;			// skip all blend lights
extern idCVar r_skipFogLights;				// skip all fog lights
extern idCVar r_skipSubviews;				// 1 = don't render any mirrors / cameras / etc
extern idCVar r_skipGuiShaders;				// 1 = don't render any gui elements on surfaces
extern idCVar r_skipParticles;				// 1 = don't render any particles
extern idCVar r_skipUpdates;				// 1 = don't accept any entity or light updates, making everything static
extern idCVar r_skipDeforms;				// leave all deform materials in their original state
extern idCVar r_skipDynamicTextures;		// don't dynamically create textures
extern idCVar r_skipBump;					// uses a flat surface instead of the bump map
extern idCVar r_skipSpecular;				// use black for specular
extern idCVar r_skipDiffuse;				// use black for diffuse
extern idCVar r_skipDecals;					// skip decal surfaces
extern idCVar r_skipOverlays;				// skip overlay surfaces
extern idCVar r_skipShadows;				// disable shadows

extern idCVar r_ignoreGLErrors;
extern idCVar r_swapInterval;

extern idCVar r_screenFraction;				// for testing fill rate, the resolution of the entire screen can be changed
extern idCVar r_showUnsmoothedTangents;		// highlight geometry rendered with unsmoothed tangents
extern idCVar r_showSilhouette;				// highlight edges that are casting shadow planes
extern idCVar r_showVertexColor;			// draws all triangles with the solid vertex color
extern idCVar r_showUpdates;				// report entity and light updates and ref counts
extern idCVar r_showDemo;					// report reads and writes to the demo file
extern idCVar r_showDynamic;				// report stats on dynamic surface generation
extern idCVar r_showIntensity;				// draw the screen colors based on intensity, red = 0, green = 128, blue = 255
extern idCVar r_showTrace;					// show the intersection of an eye trace with the world
extern idCVar r_showDepth;					// display the contents of the depth buffer and the depth range
extern idCVar r_showTris;					// enables wireframe rendering of the world
extern idCVar r_showSurfaceInfo;			// show surface material name under crosshair
extern idCVar r_showNormals;				// draws wireframe normals
extern idCVar r_showEdges;					// draw the sil edges
extern idCVar r_showViewEntitys;			// displays the bounding boxes of all view models and optionally the index
extern idCVar r_showTexturePolarity;		// shade triangles by texture area polarity
extern idCVar r_showTangentSpace;			// shade triangles by tangent space
extern idCVar r_showDominantTri;			// draw lines from vertexes to center of dominant triangles
extern idCVar r_showTextureVectors;			// draw each triangles texture (tangent) vectors
extern idCVar r_showLights;					// 1 = print light info, 2 = also draw volumes
extern idCVar r_showLightCount;				// colors surfaces based on light count
extern idCVar r_showShadows;				// visualize the stencil shadow volumes
extern idCVar r_showLightScissors;			// show light scissor rectangles
extern idCVar r_showMemory;					// print frame memory utilization
extern idCVar r_showCull;					// report sphere and box culling stats
extern idCVar r_showAddModel;				// report stats from tr_addModel
extern idCVar r_showSurfaces;				// report surface/light/shadow counts
extern idCVar r_showPrimitives;				// report vertex/index/draw counts
extern idCVar r_showPortals;				// draw portal outlines in color based on passed / not passed
extern idCVar r_showSkel;					// draw the skeleton when model animates
extern idCVar r_showOverDraw;				// show overdraw
extern idCVar r_showNvidiaHack;				// use shaders to color wireframes on Nvidia hardware and non-shader for AMD (might be broken)use shaders to color wireframes on Nvidia hardware and non-shader for AMD (might be broken)
extern idCVar r_jointNameScale;				// size of joint names when r_showskel is set to 1
extern idCVar r_jointNameOffset;			// offset of joint names when r_showskel is set to 1

extern idCVar r_testGamma;					// draw a grid pattern to test gamma levels
extern idCVar r_testGammaBias;				// draw a grid pattern to test gamma levels

extern idCVar r_singleLight;				// suppress all but one light
extern idCVar r_singleEntity;				// suppress all but one entity
extern idCVar r_singleArea;					// only draw the portal area the view is actually in
extern idCVar r_singleSurface;				// suppress all but one surface on each entity
extern idCVar r_shadowPolygonOffset;		// bias value added to depth test for stencil shadow drawing
extern idCVar r_shadowPolygonFactor;		// scale value for stencil shadow drawing

extern idCVar r_jitter;						// randomly subpixel jitter the projection matrix
extern idCVar r_orderIndexes;				// perform index reorganization to optimize vertex use

extern idCVar r_debugLineDepthTest;			// perform depth test on debug lines
extern idCVar r_debugLineWidth;				// width of debug lines
extern idCVar r_debugArrowStep;				// step size of arrow cone line rotation in degrees
extern idCVar r_debugPolygonFilled;

extern idCVar r_materialOverride;			// override all materials

extern idCVar r_debugRenderToTexture;

extern idCVar stereoRender_deGhost;			// subtract from opposite eye to reduce ghosting

extern idCVar r_useGPUSkinning;

extern idCVar r_screenshot_png_quality; // Modifies the compression level of the png file being written.

// RB begin
extern idCVar r_shadowMapFrustumFOV;
extern idCVar r_shadowMapSingleSide;
extern idCVar r_shadowMapImageSize;
extern idCVar r_shadowMapJitterScale;
extern idCVar r_shadowMapBiasScale;
extern idCVar r_shadowMapRandomizeJitter;
extern idCVar r_shadowMapSamples;
extern idCVar r_shadowMapSplits;
extern idCVar r_shadowMapSplitWeight;
extern idCVar r_shadowMapLodScale;
extern idCVar r_shadowMapLodBias;
extern idCVar r_shadowMapPolygonFactor;
extern idCVar r_shadowMapPolygonOffset;
extern idCVar r_shadowMapOccluderFacing;
extern idCVar r_shadowMapQuality;
extern idCVar r_shadowMapCascadeScale;
extern idCVar r_shadowMapStaticShadowsDistance;
// RB end

typedef struct renderEntity_s renderEntity_t;
typedef struct renderLight_s renderLight_t;

/// @brief idRenderEntity should become the new public interface replacing 
/// the qhandle_t to entity defs in the idRenderWorld interface
class idRenderEntity
{
public:
	virtual					~idRenderEntity( void ) {}
	virtual void			FreeRenderEntity( void ) = 0;
	virtual void			UpdateRenderEntity( const renderEntity_t* re, bool forceUpdate = false ) = 0;
	virtual void			GetRenderEntity( renderEntity_t* re ) = 0;
	virtual void			ForceUpdate( void ) = 0;
	virtual int				GetIndex( void ) = 0;
	
	// overlays are extra polygons that deform with animating models for blood and damage marks
	virtual void			ProjectOverlay( const idPlane localTextureAxis[2], const idMaterial* material ) = 0;
	virtual void			RemoveDecals( void ) = 0;
};

/// @brief idRenderLight should become the new public interface replacing 
/// the qhandle_t to light defs in the idRenderWorld interface
class idRenderLight
{
public:
	virtual					~idRenderLight( void ) {}
	virtual void			FreeRenderLight( void ) = 0;
	virtual void			UpdateRenderLight( const renderLight_t* re, bool forceUpdate = false ) = 0;
	virtual void			GetRenderLight( renderLight_t* re ) = 0;
	virtual void			ForceUpdate( void ) = 0;
	virtual int				GetIndex( void ) = 0;
};

///
///
///
class idImageManager
{
public:
	virtual void		Init( void ) = 0;
	virtual void		Shutdown( void ) = 0;
	virtual idImage*	ImageFromFile( const idStr &name, const textureUsage_t usage, const cubeFiles_t cubeMap = CF_2D ) = 0;
	
	/// @brief look for a loaded image, whatever the parameters
	virtual idImage*	GetImage( const idStr &name ) const;
	
	/// @brief look for a loaded image, whatever the parameters
	virtual idImage*	GetImageWithParameters( const idStr &name, const textureUsage_t usage, const cubeFiles_t cubeMap ) const;
		
	/// @brief reloads all apropriate images after a vid_restart
	virtual void	ReloadImages( const bool all );
	
	/// @brief used to clear and then write the dds conversion batch file
	virtual void	StartBuild( void ) = 0;
	virtual void	FinishBuild( const bool removeDups = false ) = 0;
	virtual void	PrintMemInfo( MemInfo_t* mi ) = 0;

	// BEATO Begin:
	virtual crBuffer*	GetPixelUnpackBuffer( void ) const = 0;
	virtual crBuffer*	GetPixelPackBuffer( void ) const = 0;
	// BEATO End
};

class idRenderWorld;
class idRenderSystem
{
public:

	virtual					~idRenderSystem( void ) {}
	
// BEATO Begin:
	/// @brief global acess to render system 
	static idRenderSystem*	Get( void );	

	static idImageManager*	GetGlobalImages( void );

	/// @brief check if render system is suceffuly initialized
	static bool IsInitialized( void );
// BEATO End

	// set up cvars and basic data structures, but don't
	// init OpenGL, so it can also be used for dedicated servers
	virtual void			Init( void ) = 0;
	
	// only called before quitting
	virtual void			Shutdown( void ) = 0;
	
	virtual void			ResetGuiModels( void ) = 0;
	
	virtual void			InitOpenGL( void ) = 0;
	virtual void			ShutdownOpenGL( void ) = 0;
	virtual bool			IsOpenGLRunning( void ) const = 0;
	
	virtual bool			IsFullScreen( void ) const = 0;
	virtual uint32_t		GetWidth( void ) const = 0;
	virtual uint32_t		GetHeight( void ) const = 0;
	
// BEATO Begin:
	///@brief update the frame rendering size, and update framebuffers
	virtual void			UpdateRenderSize( const uint32_t in_width, const uint32_t in_height ) = 0;
// BEAO End
	
	// return w/h of a single pixel. This will be 1.0 for normal cases.
	// A side-by-side stereo 3D frame will have a pixel aspect of 0.5.
	// A top-and-bottom stereo 3D frame will have a pixel aspect of 2.0
	virtual float			GetPixelAspect() const = 0;
	
	// This is used to calculate stereoscopic screen offset for a given interocular distance.
	virtual float			GetPhysicalScreenWidthInCentimeters() const = 0;
	
	// GetWidth() / GetHeight() return the size of a single eye
	// view, which may be replicated twice in a stereo display
	virtual stereo3DMode_t	GetStereo3DMode() const = 0;
	virtual bool			IsStereoScopicRenderingSupported() const = 0;
	virtual stereo3DMode_t	GetStereoScopicRenderingMode() const = 0;
	virtual void			EnableStereoScopicRendering( const stereo3DMode_t mode ) const = 0;
	virtual bool			HasQuadBufferSupport() const = 0;
	
	// allocate a renderWorld to be used for drawing
	virtual idRenderWorld* 	AllocRenderWorld() = 0;
	virtual	void			FreeRenderWorld( idRenderWorld* rw ) = 0;
	
	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	virtual void			BeginLevelLoad() = 0;
	virtual void			EndLevelLoad() = 0;
	virtual void			Preload( const idPreloadManifest& manifest, const char* mapName ) = 0;
	virtual void			LoadLevelImages() = 0;
	
	virtual void			BeginAutomaticBackgroundSwaps( autoRenderIconType_t icon = AUTORENDER_DEFAULTICON ) = 0;
	virtual void			EndAutomaticBackgroundSwaps() = 0;
	virtual bool			AreAutomaticBackgroundSwapsRunning( autoRenderIconType_t* icon = nullptr ) const = 0;
	
	// font support
	virtual class idFont* 	RegisterFont( const char* fontName ) = 0;
	virtual void			ResetFonts() = 0;
	
	virtual void			SetColor( const idVec4& rgba ) = 0;
	virtual void			SetColor4( float r, float g, float b, float a )
	{
		SetColor( idVec4( r, g, b, a ) );
	}
	
	virtual uint32_t			GetColor() = 0;
	
	virtual void			SetGLState( const uint64_t glState ) = 0;
	
	virtual void			DrawFilled( const idVec4& color, float x, float y, float w, float h ) = 0;
	virtual void			DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const idMaterial* material ) = 0;
	void					DrawStretchPic( const idVec4& rect, const idVec4& st, const idMaterial* material )
	{
		DrawStretchPic( rect.x, rect.y, rect.z, rect.w, st[0], st[1], st.z, st.w, material );
	}
	virtual void			DrawStretchPic( const idVec4& topLeft, const idVec4& topRight, const idVec4& bottomRight, const idVec4& bottomLeft, const idMaterial* material ) = 0;
	virtual void			DrawStretchTri( const idVec2& p1, const idVec2& p2, const idVec2& p3, const idVec2& t1, const idVec2& t2, const idVec2& t3, const idMaterial* material ) = 0;
	virtual idDrawVert* 	AllocTris( int numVerts, const triIndex_t* indexes, int numIndexes, const idMaterial* material, const stereoDepthType_t stereoType = STEREO_DEPTH_TYPE_NONE ) = 0;
	
	virtual void			PrintMemInfo( MemInfo_t* mi ) = 0;
	
	virtual void			DrawSmallChar( int x, int y, int ch ) = 0;
	virtual void			DrawSmallStringExt( int x, int y, const char* string, const idVec4& setColor, bool forceColor ) = 0;
	virtual void			DrawBigChar( int x, int y, int ch ) = 0;
	virtual void			DrawBigStringExt( int x, int y, const char* string, const idVec4& setColor, bool forceColor ) = 0;
	
	// dump all 2D drawing so far this frame to the demo file
	virtual void			WriteDemoPics( void ) = 0;
	virtual void			WriteEndFrame( void ) = 0;
	
	// draw the 2D pics that were saved out with the current demo frame
	virtual void			DrawDemoPics( void ) = 0;
	
	// Performs final closeout of any gui models being defined.
	//
	// Waits for the previous GPU rendering to complete and vsync.
	//
	// Returns the head of the linked command list that was just closed off.
	//
	// Returns timing information from the previous frame.
	//
	// After this is called, new command buffers can be built up in parallel
	// with the rendering of the closed off command buffers by RenderCommandBuffers()
	virtual const emptyCommand_t* 	SwapCommandBuffers( uint64_t* frontEndMicroSec, uint64_t* backEndMicroSec, uint64_t* shadowMicroSec, uint64_t* gpuMicroSec ) = 0;
	
	// SwapCommandBuffers operation can be split in two parts for non-smp rendering
	// where the GPU is idled intentionally for minimal latency.
	virtual void			SwapCommandBuffers_FinishRendering( uint64_t* frontEndMicroSec, uint64_t* backEndMicroSec, uint64_t* shadowMicroSec, uint64_t* gpuMicroSec ) = 0;
	virtual const emptyCommand_t* 	SwapCommandBuffers_FinishCommandBuffers() = 0;
	
	// issues GPU commands to render a built up list of command buffers returned
	// by SwapCommandBuffers().  No references should be made to the current frameData,
	// so new scenes and GUIs can be built up in parallel with the rendering.
	virtual void			RenderCommandBuffers( const emptyCommand_t* commandBuffers ) = 0;
	
	// aviDemo uses this.
	// Will automatically tile render large screen shots if necessary
	// Samples is the number of jittered frames for anti-aliasing
	// If ref == nullptr, common->UpdateScreen will be used
	// This will perform swapbuffers, so it is NOT an approppriate way to
	// generate image files that happen during gameplay, as for savegame
	// markers.  Use WriteRender() instead.
	virtual void			TakeScreenshot( int width, int height, const char* fileName, int samples, struct renderView_s* ref ) = 0;
	virtual void			TakeScreenshot( int width, int height, idFile* outFile, int blends, renderView_t* ref ) = 0;
	
	// the render output can be cropped down to a subset of the real screen, as
	// for save-game reviews and split-screen multiplayer.  Users of the renderer
	// will not know the actual pixel size of the area they are rendering to
	
	// the x,y,width,height values are in virtual SCREEN_WIDTH / SCREEN_HEIGHT coordinates
	
	// to render to a texture, first set the crop size with makePowerOfTwo = true,
	// then perform all desired rendering, then capture to an image
	// if the specified physical dimensions are larger than the current cropped region, they will be cut down to fit
	virtual void			CropRenderSize( int width, int height ) = 0;
	virtual void			CaptureRenderToImage( const char* imageName, bool clearColorAfterCopy = false ) = 0;
	// fixAlpha will set all the alpha channel values to 0xff, which allows screen captures
	// to use the default tga loading code without having dimmed down areas in many places
	virtual void			CaptureRenderToFile( const char* fileName, bool fixAlpha = false ) = 0;
	virtual void			UnCrop( void ) = 0;
	
	// the image has to be already loaded ( most straightforward way would be through a FindMaterial )
	// texture filter / mipmapping / repeat won't be modified by the upload
	// returns false if the image wasn't found
	virtual bool			UploadImage( const char* imageName, const byte* data, int width, int height ) = 0;
	
	// consoles switch stereo 3D eye views each 60 hz frame
	virtual int				GetFrameCount( void ) const = 0;

	virtual void OnFrame( void ) = 0;
	
};

// used by the view shot taker
void R_ScreenshotFilename( int& lastNumber, const char* base, idStr& fileName );

#endif /* !__RENDERER_H__ */
