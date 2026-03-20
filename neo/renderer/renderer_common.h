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
#ifndef __TR_LOCAL_H__
#define __TR_LOCAL_H__

inline constexpr uint32_t	MAX_PROG_TEXTURE_PARMS	= 16; // maximum texture units
inline constexpr uint32_t	FALLOFF_TEXTURE_SIZE = 64;
inline constexpr float		DEFAULT_FOG_DISTANCE = 500.0f;

// picky to get the bilerp correct at terminator
inline constexpr int 		FOG_ENTER_SIZE	= 64;
inline constexpr float 		FOG_ENTER		= ( FOG_ENTER_SIZE + 1.0f ) / ( FOG_ENTER_SIZE * 2 );

#include "GLState.h"
#include "Vulkan/Core.hpp"
#include "ScreenRect.h"
#include "images/Image.h"
#include "images/Image_files.hpp"
#include "images/ImageManager.hpp"
#include "Framebuffer.h"
#include "RenderTexture.h"
#include "Font.h"

enum demoCommand_t
{
	DC_BAD,
	DC_RENDERVIEW,
	DC_UPDATE_ENTITYDEF,
	DC_DELETE_ENTITYDEF,
	DC_UPDATE_LIGHTDEF,
	DC_DELETE_LIGHTDEF,
	DC_LOADMAP,
	DC_CROP_RENDER,
	DC_UNCROP_RENDER,
	DC_CAPTURE_RENDER,
	DC_END_FRAME,
	DC_DEFINE_MODEL,
	DC_SET_PORTAL_STATE,
	DC_UPDATE_SOUNDOCCLUSION,
	DC_GUI_MODEL,	
	DC_UPDATE_DECAL,
	DC_DELETE_DECAL,
	DC_UPDATE_OVERLAY,
	DC_DELETE_OVERLAY,
	DC_CACHE_SKINS,
	DC_CACHE_PARTICLES,
	DC_CACHE_MATERIALS,
};

/*
==============================================================================

SURFACES

==============================================================================
*/
#include "models/ModelDecal.h"
#include "models/ModelOverlay.h"
#include "Interaction.h"

class idRenderWorldLocal;
struct viewEntity_t;
struct viewLight_t;

// BEATO Begin:
class crDrawGeometry;
// BEATO End

// drawSurf_t structures command the back end to render surfaces
// a given crDrawGeometry may be used with multiple viewEntity_t,
// as when viewed in a subview or multiple viewport render, or
// with multiple shaders when skinned, or, possibly with multiple
// lights, although currently each lighting interaction creates
// unique crDrawGeometry
// drawSurf_t are always allocated and freed every frame, they are never cached

struct drawSurf_t
{
	const crDrawGeometry* 	frontEndGeo;		// don't use on the back end, it may be updated by the front end!
	int						numIndexes;
	vertCacheHandle_t		indexCache;			// triIndex_t
	vertCacheHandle_t		ambientCache;		// idDrawVert
	vertCacheHandle_t		shadowCache;		// idShadowVert / idShadowVertSkinned
	vertCacheHandle_t		jointCache;			// idJointMat
	const viewEntity_t* 	space;
	const idMaterial* 		material;			// may be nullptr for shadow volumes
	float					sort;				// material->sort, modified by gui / entity sort offsets
	const float* 				shaderRegisters;	// evaluated and adjusted for referenceShaders
	drawSurf_t* 			nextOnLight;		// viewLight chains
	drawSurf_t** 			linkChain;			// defer linking to lights to a serial section to avoid a mutex
	idScreenRect			scissorRect;		// for scissor clipping, local inside renderView viewport
	int						renderZFail;
	volatile shadowVolumeState_t shadowVolumeState;
};

// areas have references to hold all the lights and entities in them
struct areaReference_t
{
	areaReference_t* 		areaNext;				// chain in the area
	areaReference_t* 		areaPrev;
	areaReference_t* 		ownerNext;				// chain on either the entityDef or lightDef
	idRenderEntityLocal* 	entity;					// only one of entity / light will be non-nullptr
	idRenderLightLocal* 	light;					// only one of entity / light will be non-nullptr
	struct portalArea_s*		area;					// so owners can find all the areas they are in
};

// BEATO Begin: we need shadowFrustum_t to compute map shadows
typedef struct 
{
	int		numPlanes;		// this is always 6 for now
	idPlane	planes[6];
	// positive sides facing inward
	// plane 5 is always the plane the projection is going to, the
	// other planes are just clip planes
	// all planes are in global coordinates

	bool	makeClippedPlanes;
	// a projected light with a single frustum needs to make sil planes
	// from triangles that clip against side planes, but a point light
	// that has adjacent frustums doesn't need to
} shadowFrustum_t;

// BEATO End


struct shadowOnlyEntity_t
{
	shadowOnlyEntity_t* 	next;
	idRenderEntityLocal*		edef;
};

// viewLights are allocated on the frame temporary stack memory
// a viewLight contains everything that the back end needs out of an idRenderLightLocal,
// which the front end may be modifying simultaniously if running in SMP mode.
// a viewLight may exist even without any surfaces, and may be relevent for fogging,
// but should never exist if its volume does not intersect the view frustum
struct viewLight_t
{
	viewLight_t* 			next;
	
	// back end should NOT reference the lightDef, because it can change when running SMP
	idRenderLightLocal* 	lightDef;
	
	// for scissor clipping, local inside renderView viewport
	// scissorRect.Empty() is true if the viewEntity_t was never actually
	// seen through any portals
	idScreenRect			scissorRect;
	
	// R_AddSingleLight() determined that the light isn't actually needed
	bool					removeFromList;
	
	// R_AddSingleLight builds this list of entities that need to be added
	// to the viewEntities list because they potentially cast shadows into
	// the view, even though the aren't directly visible
	shadowOnlyEntity_t* 	shadowOnlyViewEntities;
	
	enum interactionState_t
	{
		INTERACTION_UNCHECKED,
		INTERACTION_NO,
		INTERACTION_YES
	};
	byte* 					entityInteractionState;		// [numEntities]
	
	idVec3					globalLightOrigin;			// global light origin used by backend
	idPlane					lightProject[4];			// light project used by backend
	idPlane					fogPlane;					// fog plane for backend fog volume rendering
	// RB: added for shadow mapping
	idRenderMatrix			baseLightProject;			// global xyz1 to projected light strq
	bool					pointLight;					// otherwise a projection light (should probably invert the sense of this, because points are way more common)
	bool					parallel;					// lightCenter gives the direction to the light at infinity
	idVec3					lightCenter;				// offset the lighting direction for shading and
	int						shadowLOD;					// level of detail for shadowmap selection
	// RB end
	idRenderMatrix			inverseBaseLightProject;	// the matrix for deforming the 'zeroOneCubeModel' to exactly cover the light volume in world space
	const idMaterial* 		lightShader;				// light shader used by backend
	const float*				shaderRegisters;			// shader registers used by backend
	idImage* 				falloffImage;				// falloff image used by backend
	
	drawSurf_t* 			globalShadows;				// shadow everything
	drawSurf_t* 			localInteractions;			// don't get local shadows
	drawSurf_t* 			localShadows;				// don't shadow local surfaces
	drawSurf_t* 			globalInteractions;			// get shadows from everything
	drawSurf_t* 			translucentInteractions;	// translucent interactions don't get shadows
	
	// R_AddSingleLight will build a chain of parameters here to setup shadow volumes
	preLightShadowVolumeParms_t* 	preLightShadowVolumes;
};

// a viewEntity is created whenever a idRenderEntityLocal is considered for inclusion
// in the current view, but it may still turn out to be culled.
// viewEntity are allocated on the frame temporary stack memory
// a viewEntity contains everything that the back end needs out of a idRenderEntityLocal,
// which the front end may be modifying simultaneously if running in SMP mode.
// A single entityDef can generate multiple viewEntity_t in a single frame, as when seen in a mirror
struct viewEntity_t
{
	viewEntity_t* 			next;
	
	// back end should NOT reference the entityDef, because it can change when running SMP
	idRenderEntityLocal*		entityDef;
	
	// for scissor clipping, local inside renderView viewport
	// scissorRect.Empty() is true if the viewEntity_t was never actually
	// seen through any portals, but was created for shadow casting.
	// a viewEntity can have a non-empty scissorRect, meaning that an area
	// that it is in is visible, and still not be visible.
	idScreenRect			scissorRect;
	
	bool					isGuiSurface;			// force two sided and vertex colors regardless of material setting
	
	bool					skipMotionBlur;
	
	bool					weaponDepthHack;
	float					modelDepthHack;
	
	float					modelMatrix[16];		// local coords to global coords
	float					modelViewMatrix[16];	// local coords to eye coords
	
	idRenderMatrix			mvp;
	
	// parallelAddModels will build a chain of surfaces here that will need to
	// be linked to the lights or added to the drawsurf list in a serial code section
	drawSurf_t* 			drawSurfs;
	
	// R_AddSingleModel will build a chain of parameters here to setup shadow volumes
	staticShadowVolumeParms_t* 		staticShadowVolumes;
	dynamicShadowVolumeParms_t* 	dynamicShadowVolumes;
};

const int	MAX_CLIP_PLANES	= 1;				// we may expand this to six for some subview issues

// RB: added multiple subfrustums for cascaded shadow mapping
enum frustumPlanes_t
{
	FRUSTUM_PLANE_LEFT,
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_NEAR,
	FRUSTUM_PLANE_FAR,
	FRUSTUM_PLANES = 6,
	FRUSTUM_CLIPALL = 1 | 2 | 4 | 8 | 16 | 32
};

enum
{
	FRUSTUM_PRIMARY,
	FRUSTUM_CASCADE1,
	FRUSTUM_CASCADE2,
	FRUSTUM_CASCADE3,
	FRUSTUM_CASCADE4,
	FRUSTUM_CASCADE5,
	MAX_FRUSTUMS,
};

typedef idPlane frustum_t[FRUSTUM_PLANES];
// RB end

// viewDefs are allocated on the frame temporary stack memory
struct viewDef_t
{
	// specified in the call to DrawScene()
	renderView_t		renderView;
	
	float				projectionMatrix[16];
	idRenderMatrix		projectionRenderMatrix;	// tech5 version of projectionMatrix
	viewEntity_t		worldSpace;
	
	idRenderWorldLocal* renderWorld;
	
	idVec3				initialViewAreaOrigin;
	// Used to find the portalArea that view flooding will take place from.
	// for a normal view, the initialViewOrigin will be renderView.viewOrg,
	// but a mirror may put the projection origin outside
	// of any valid area, or in an unconnected area of the map, so the view
	// area must be based on a point just off the surface of the mirror / subview.
	// It may be possible to get a failed portal pass if the plane of the
	// mirror intersects a portal, and the initialViewAreaOrigin is on
	// a different side than the renderView.viewOrg is.
	
	bool				isSubview;				// true if this view is not the main view
	bool				isMirror;				// the portal is a mirror, invert the face culling
	bool				isXraySubview;
	
	bool				isEditor;
	bool				is2Dgui;
	bool				isObliqueProjection;

	int					numClipPlanes;			// mirrors will often use a single clip plane
	idPlane				clipPlanes[MAX_CLIP_PLANES];		// in world space, the positive side
	// of the plane is the visible side
	idScreenRect		viewport;				// in real pixels and proper Y flip
	
	idScreenRect		scissor;
	// for scissor clipping, local inside renderView viewport
	// subviews may only be rendering part of the main view
	// these are real physical pixel values, possibly scaled and offset from the
	// renderView x/y/width/height
	
	viewDef_t* 			superView;				// never go into an infinite subview loop
	const drawSurf_t* 	subviewSurface;
	
	// drawSurfs are the visible surfaces of the viewEntities, sorted
	// by the material sort parameter
	drawSurf_t** 		drawSurfs;				// we don't use an idList for this, because
	int					numDrawSurfs;			// it is allocated in frame temporary memory
	int					maxDrawSurfs;			// may be resized
	
	viewLight_t*			viewLights;			// chain of all viewLights effecting view
	viewEntity_t* 		viewEntitys;			// chain of all viewEntities effecting view, including off screen ones casting shadows
	// we use viewEntities as a check to see if a given view consists solely
	// of 2D rendering, which we can optimize in certain ways.  A 2D view will
	// not have any viewEntities
	
	// RB begin
	frustum_t			frustums[MAX_FRUSTUMS];					// positive sides face outward, [4] is the front clip plane
	float				frustumSplitDistances[MAX_FRUSTUMS];
	idRenderMatrix		frustumMVPs[MAX_FRUSTUMS];
	// RB end
	
	int					areaNum;				// -1 = not in a valid area
	
	// An array in frame temporary memory that lists if an area can be reached without
	// crossing a closed door.  This is used to avoid drawing interactions
	// when the light is behind a closed door.
	bool* 				connectedAreas;
};


// complex light / surface interactions are broken up into multiple passes of a
// simple interaction shader
struct drawInteraction_t
{
	const drawSurf_t* 	surf;
	
	idImage* 			bumpImage;
	vkSampler*			bumpSampler;
	idImage* 			diffuseImage;
	vkSampler*			diffuseSampler;
	idImage* 			specularImage;
	vkSampler*			specularSampler;
	idImage* 			glossImage;
	vkSampler*			glossSampler;
	
	idVec4				diffuseColor;	// may have a light color baked into it
	idVec4				specularColor;	// may have a light color baked into it
	idVec4				glossColor;		// may have a light color baked into it
	stageVertexColor_t	vertexColor;	// applies to both diffuse and specular
	
	int					ambientLight;	// use tr.ambientNormalMap instead of normalization cube map
	
	// these are loaded into the vertex program
	idVec4				bumpMatrix[2];
	idVec4				diffuseMatrix[2];
	idVec4				specularMatrix[2];
	idVec4				glossMatrix[2];
};

//=======================================================================

// this is the inital allocation for max number of drawsurfs
// in a given view, but it will automatically grow if needed
const int INITIAL_DRAWSURFS =		2048;

enum frameAllocType_t
{
	FRAME_ALLOC_VIEW_DEF,
	FRAME_ALLOC_VIEW_ENTITY,
	FRAME_ALLOC_VIEW_LIGHT,
	FRAME_ALLOC_SURFACE_TRIANGLES,
	FRAME_ALLOC_DRAW_SURFACE,
	FRAME_ALLOC_INTERACTION_STATE,
	FRAME_ALLOC_SHADOW_ONLY_ENTITY,
	FRAME_ALLOC_SHADOW_VOLUME_PARMS,
	FRAME_ALLOC_SHADER_REGISTER,
	FRAME_ALLOC_DRAW_SURFACE_POINTER,
	FRAME_ALLOC_DRAW_COMMAND,
	FRAME_ALLOC_UNKNOWN,
	FRAME_ALLOC_MAX
};

// all of the information needed by the back end must be
// contained in a idFrameData.  This entire structure is
// duplicated so the front and back end can run in parallel
// on an SMP machine.
class idFrameData
{
public:
	idSysInterlockedInteger	frameMemoryAllocated;
	idSysInterlockedInteger	frameMemoryUsed;
	byte* 					frameMemory;
	
	int						highWaterAllocated;	// max used on any frame
	int						highWaterUsed;
	
	// the currently building command list commands can be inserted
	// at the front if needed, as required for dynamically generated textures
	emptyCommand_t* 		cmdHead;	// may be of other command type based on commandId
	emptyCommand_t* 		cmdTail;
};

extern	idFrameData*	frameData;

//=======================================================================

void R_AddDrawViewCmd( viewDef_t* parms, bool guiOnly );
void R_AddDrawPostProcess( viewDef_t* parms );

void R_ReloadGuis_f( const idCmdArgs& args );
void R_ListGuis_f( const idCmdArgs& args );

void* R_GetCommandBuffer( int bytes );

// this allows a global override of all materials
bool R_GlobalShaderOverride( const idMaterial** shader );

// this does various checks before calling the idDeclSkin
const idMaterial* R_RemapShaderBySkin( const idMaterial* shader, const idDeclSkin* customSkin, const idMaterial* customShader );


//====================================================

//
// performanceCounters_t
//
struct performanceCounters_t
{
	int		c_box_cull_in;
	int		c_box_cull_out;
	int		c_createInteractions;	// number of calls to idInteraction::CreateInteraction
	int		c_createShadowVolumes;
	int		c_generateMd5;
	int		c_entityDefCallbacks;
	int		c_alloc;			// counts for R_StaticAllc/R_StaticFree
	int		c_free;
	int		c_visibleViewEntities;
	int		c_shadowViewEntities;
	int		c_viewLights;
	int		c_numViews;			// number of total views rendered
	int		c_deformedSurfaces;	// idMD5Mesh::GenerateSurface
	int		c_deformedVerts;	// idMD5Mesh::GenerateSurface
	int		c_deformedIndexes;	// idMD5Mesh::GenerateSurface
	int		c_tangentIndexes;	// R_DeriveTangents()
	int		c_entityUpdates;
	int		c_lightUpdates;
	int		c_entityReferences;
	int		c_lightReferences;
	int		c_guiSurfs;
	int		frontEndMicroSec;	// sum of time in all RE_RenderScene's in a frame
};

class idParallelJobList;

const int MAX_GUI_SURFACES	= 1024;		// default size of the drawSurfs list for guis, will
// be automatically expanded as needed

static const int MAX_RENDER_CROPS = 8;

class glContext;
class vkContext;
#include "RenderSystemLocal.h"

extern glconfig_t			glConfig;		// outside of TR since it shouldn't be cleared during ref re-init

/*
====================================================================

INITIALIZATION

====================================================================
*/

void R_Init();

void R_SetColorMappings();

void R_ScreenShot_f( const idCmdArgs& args );
void R_StencilShot();

extern void R_CheckCvars( void );

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

/*
============================================================

RENDERWORLD_DEFS

============================================================
*/

void R_DeriveEntityData( idRenderEntityLocal* def );
void R_CreateEntityRefs( idRenderEntityLocal* def );
void R_FreeEntityDefDerivedData( idRenderEntityLocal* def, bool keepDecals, bool keepCachedDynamicModel );
void R_FreeEntityDefCachedDynamicModel( idRenderEntityLocal* def );
void R_FreeEntityDefDecals( idRenderEntityLocal* def );
void R_FreeEntityDefOverlay( idRenderEntityLocal* def );
void R_FreeEntityDefFadedDecals( idRenderEntityLocal* def, int time );

void R_CreateLightRefs( idRenderLightLocal* light );
void R_FreeLightDefDerivedData( idRenderLightLocal* light );

void R_FreeDerivedData();
void R_ReCreateWorldReferences();
void R_CheckForEntityDefsUsingModel( idRenderModel* model );
void R_ModulateLights_f( const idCmdArgs& args );

/*
============================================================

RENDERWORLD_PORTALS

============================================================
*/

viewEntity_t* R_SetEntityDefViewEntity( idRenderEntityLocal* def );
viewLight_t* R_SetLightDefViewLight( idRenderLightLocal* def );

/*
====================================================================

TR_FRONTEND_MAIN

====================================================================
*/
void R_InitFrameData();
void R_ShutdownFrameData();
void R_ToggleSmpFrame();
void* R_FrameAlloc( int bytes, frameAllocType_t type = FRAME_ALLOC_UNKNOWN );
void* R_ClearedFrameAlloc( int bytes, frameAllocType_t type = FRAME_ALLOC_UNKNOWN );

void* R_StaticAlloc( int bytes, const memTag_t tag = TAG_RENDER_STATIC );		// just malloc with error checking
void* R_ClearedStaticAlloc( int bytes );	// with std::memset
void R_StaticFree( void* data );

void R_RenderView( viewDef_t* parms );
void R_RenderPostProcess( viewDef_t* parms );

// BEATO Begin:
void R_InitMaterials( void );
// BEATO End

/*
============================================================

TR_FRONTEND_ADDLIGHTS

============================================================
*/

void R_ShadowBounds( const idBounds& modelBounds, const idBounds& lightBounds, const idVec3& lightOrigin, idBounds& shadowBounds );

void R_AddLights();
void R_OptimizeViewLightsList();

/*
============================================================

TR_FRONTEND_ADDMODELS

============================================================
*/

bool R_IssueEntityDefCallback( idRenderEntityLocal* def );
idRenderModel* R_EntityDefDynamicModel( idRenderEntityLocal* def );
void R_ClearEntityDefDynamicModel( idRenderEntityLocal* def );

void R_SetupDrawSurfShader( drawSurf_t* drawSurf, const idMaterial* shader, const renderEntity_t* renderEntity );
void R_SetupDrawSurfJoints( drawSurf_t* drawSurf, const crDrawGeometry* tri, const idMaterial* shader );
void R_LinkDrawSurfToView( drawSurf_t* drawSurf, viewDef_t* viewDef );

void R_AddModels();

/*
=============================================================

TR_FRONTEND_DEFORM

=============================================================
*/

drawSurf_t* R_DeformDrawSurf( drawSurf_t* drawSurf );

/*
=============================================================

TR_FRONTEND_GUISURF

=============================================================
*/

void R_SurfaceToTextureAxis( const crDrawGeometry* tri, idVec3& origin, idVec3 axis[3] );
void R_AddInGameGuis( const drawSurf_t* const drawSurfs[], const int numDrawSurfs );

/*
============================================================

TR_FRONTEND_SUBVIEW

============================================================
*/

bool R_PreciseCullSurface( const drawSurf_t* drawSurf, idBounds& ndcBounds );
bool R_GenerateSubViews( const drawSurf_t* const drawSurfs[], const int numDrawSurfs );

/*
============================================================

TR_TRISURF

============================================================
*/
crDrawGeometry* 	R_CopyStaticTriSurf( const crDrawGeometry* tri );

// deformable meshes precalculate as much as possible from a base frame, then generate
// complete crDrawGeometry from just a new set of vertexes
struct deformInfo_t
{
	int					numSourceVerts;
	
	// numOutputVerts may be smaller if the input had duplicated or degenerate triangles
	// it will often be larger if the input had mirrored texture seams that needed
	// to be busted for proper tangent spaces
	int					numOutputVerts;
	idDrawVert* 		verts;
	
	int					numIndexes;
	triIndex_t* 		indexes;
	
	triIndex_t* 		silIndexes;				// indexes changed to be the first vertex with same XYZ, ignoring normal and texcoords
	
	int					numMirroredVerts;		// this many verts at the end of the vert list are tangent mirrors
	int* 				mirroredVerts;			// tri->mirroredVerts[0] is the mirror of tri->NumVerts() - tri->numMirroredVerts + 0
	
	int					numDupVerts;			// number of duplicate vertexes
	int* 				dupVerts;				// pairs of the number of the first vertex and the number of the duplicate vertex
	
	int					numSilEdges;			// number of silhouette edges
	silEdge_t* 			silEdges;				// silhouette edges
	
	vertCacheHandle_t	staticIndexCache;		// GL_INDEX_TYPE
	vertCacheHandle_t	staticAmbientCache;		// idDrawVert
	vertCacheHandle_t	staticShadowCache;		// idShadowCacheSkinned
};

// if outputVertexes is not nullptr, it will point to a newly allocated set of verts that includes the mirrored ones
deformInfo_t* 		R_BuildDeformInfo( int numVerts, const idDrawVert* verts, int numIndexes, const int* indexes,
									   bool useUnsmoothedTangents );
void				R_FreeDeformInfo( deformInfo_t* deformInfo );
int					R_DeformInfoMemoryUsed( deformInfo_t* deformInfo );

/*
=============================================================

TR_TRACE

=============================================================
*/

struct localTrace_t
{
	float		fraction;
	// only valid if fraction < 1.0
	idVec3		point;
	idVec3		normal;
	int			indexes[3];
};

localTrace_t R_LocalTrace( const idVec3& start, const idVec3& end, const float radius, const crDrawGeometry* tri );

/*
=============================================================

BACKEND

=============================================================
*/

#include "ResolutionScale.h"
#include "RenderLog.h"
#include "AutoRender.h"
#include "AutoRenderBink.h"
#include "jobs/ShadowShared.h"
#include "jobs/prelightshadowvolume/PreLightShadowVolume.h"
#include "jobs/staticshadowvolume/StaticShadowVolume.h"
#include "jobs/dynamicshadowvolume/DynamicShadowVolume.h"
#include "GraphicsAPIWrapper.h"
#include "GLMatrix.h"
#include "Geometry.h"

// BEATO Begin:
#include "backend/UniformManager.hpp"
#include "backend/PipelineManager.hpp"
#include "backend/Backend.hpp"
// BEATO End

#include "BufferObject.h"
#include "renderworld/RenderEntity.hpp"
#include "renderworld/RenderWorld_local.h"
#include "models/GuiModel.h"
#include "VertexCache.h"

#endif /* !__TR_LOCAL_H__ */