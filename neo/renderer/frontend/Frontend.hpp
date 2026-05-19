
#ifndef __FRONTEND_HPP__
#define __FRONTEND_HPP__

class crDrawGeometry;
class idMaterial;
class idRenderEntityLocal;
struct viewEntity_t;

// this is the inital allocation for max number of drawsurfs
// in a given view, but it will automatically grow if needed
const int INITIAL_DRAWSURFS =		2048;


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
	uint32_t			frameID;
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

//
// frontEndCounters_t
//
struct frontEndCounters_t
{
	int		c_box_cull_in;
	int		c_box_cull_out;
	int		c_createInteractions;	// number of calls to idInteraction::CreateInteraction
	int		c_createShadowVolumes;
	int		c_generateMd5;
	int		c_entityDefCallbacks;
	int		c_alloc;			// counts for R_StaticAllc/crFrontend::Get().StaticFree
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

constexpr uint32_t NUM_FRAME_DATA = MAX_SMP_FRAMES;

class crFrontend
{
public:
    crFrontend( void );
    ~crFrontend( void );
    static crFrontend& Get( void );
    void        Init( void );
    void        Clear( void );
    void        AddDrawViewCmd( viewDef_t* parms, const bool guiOnly );
    void        AddDrawPostProcess( viewDef_t* parms );
    void*       GetCommandBuffer( const size_t bytes );
    
    ID_INLINE   void                    SetViewDef( viewDef_t* newviewDef ) { viewDef = newviewDef; }
    ID_INLINE   viewDef_t*              GetViewDef( void ) const { return viewDef; }
    ID_INLINE   void                    ClearViewDef( void ) { viewDef = nullptr; }
    ID_INLINE   bool                    HasCommand( void ) const { return frameData->cmdHead->next != nullptr; }
    ID_INLINE   size_t                  FrameMemoryAllocated( void ) const { return frameData->frameMemoryAllocated.GetValue(); }
    ID_INLINE   void                    ZeroPerformanceCounters( void ) { std::memset( &pc, 0, sizeof( frontEndCounters_t ) ); }
    ID_INLINE   frontEndCounters_t      PerformanceCounters( void ) const { return pc; }
    ID_INLINE   const emptyCommand_t*   CommandBufferHead( void ) const { return frameData->cmdHead; }
    ID_INLINE   void                    IncrementEntityReferences( void ) { pc.c_entityReferences++; }
    ID_INLINE   void                    IncrementLightReferences( void ) { pc.c_lightReferences++; }
    ID_INLINE   void                    IncrementEntityUpdates( void ) { pc.c_entityUpdates++; }
    ID_INLINE   void                    IncrementLightUpdates( void ) { pc.c_lightUpdates++; }
    ID_INLINE   void                    IncrementGenerateMd5( void ) { pc.c_generateMd5++; }
    ID_INLINE   void                    AddFrontEndMicroSec( const uint64_t in_delta) { pc.frontEndMicroSec += in_delta; }
    ID_INLINE   void                    SetDeformedSurfacesCounters( int deformedVertsCount, int deformedIndexesCount )
    {
        pc.c_deformedSurfaces++;
        pc.c_deformedVerts += deformedVertsCount;
        pc.c_deformedIndexes += deformedIndexesCount;
    }

    // ====================================================================
    // TR_FRONTEND_MAIN
    // ====================================================================
    void    InitFrameData( void );
    void    ShutdownFrameData( void );
    void    ToggleSmpFrame( void );
    void*   FrameAlloc( const size_t bytes, frameAllocType_t type = FRAME_ALLOC_UNKNOWN );
    void*   ClearedFrameAlloc( const size_t bytes, frameAllocType_t type = FRAME_ALLOC_UNKNOWN );

    void*   StaticAlloc( const size_t bytes, const memTag_t tag = TAG_RENDER_STATIC );		// just malloc with error checking
    void*   ClearedStaticAlloc( const size_t bytes );	// with std::memset
    void    StaticFree( void* data );
    void    ShowColoredScreenRect( const idScreenRect& rect, int colorIndex );
    void    GlobalToNormalizedDeviceCoordinates( const idVec3& global, idVec3& ndc );
    void        RenderView( viewDef_t* parms );
    void        RenderPostProcess( viewDef_t* parms );

    // ============================================================
    // TR_FRONTEND_ADDLIGHTS
    // ============================================================
    void        AddLights( void );
    void        OptimizeViewLightsList( void );

    // ============================================================
    // TR_FRONTEND_ADDMODELS
    // ============================================================ 
    bool            IssueEntityDefCallback( idRenderEntityLocal* def );
    idRenderModel*  EntityDefDynamicModel( idRenderEntityLocal* def );
    void            ClearEntityDefDynamicModel( idRenderEntityLocal* def );
    void            SetupDrawSurfShader( drawSurf_t* drawSurf, const idMaterial* shader, const renderEntity_t* renderEntity );
    void            SetupDrawSurfJoints( drawSurf_t* drawSurf, const crDrawGeometry* tri, const idMaterial* shader );
    void            LinkDrawSurfToView( drawSurf_t* drawSurf, viewDef_t* viewDef );
    void            AddModels( void );

    // =============================================================
    // TR_FRONTEND_DEFORM
    // =============================================================
    drawSurf_t* DeformDrawSurf( drawSurf_t* drawSurf );

    // =============================================================
    // TR_FRONTEND_GUISURF
    // =============================================================
    void        SurfaceToTextureAxis( const crDrawGeometry* tri, idVec3& origin, idVec3 axis[3] );
    void        AddInGameGuis( const drawSurf_t* const drawSurfs[], const uint32_t numDrawSurfs );

    // ============================================================
    // TR_FRONTEND_SUBVIEW
    // ============================================================
    bool        PreciseCullSurface( const drawSurf_t* drawSurf, idBounds& ndcBounds );
    bool        GenerateSubViews( const drawSurf_t* const drawSurfs[], const int numDrawSurfs );

    // ============================================================
    // RENDERWORLD_DEFS
    // ============================================================
    void        DeriveEntityData( idRenderEntityLocal* def );
    void        CreateEntityRefs( idRenderEntityLocal* def );
    void        FreeEntityDefDerivedData( idRenderEntityLocal* def, bool keepDecals, bool keepCachedDynamicModel );
    void        FreeEntityDefCachedDynamicModel( idRenderEntityLocal* def );
    void        FreeEntityDefDecals( idRenderEntityLocal* def );
    void        FreeEntityDefOverlay( idRenderEntityLocal* def );
    void        FreeEntityDefFadedDecals( idRenderEntityLocal* def );

    void        CreateLightRefs( idRenderLightLocal* light );
    void        FreeLightDefDerivedData( idRenderLightLocal* light );

    void        FreeDerivedData( void );
    void        ReCreateWorldReferences( void );
    void        CheckForEntityDefsUsingModel( idRenderModel* model );
    


    // ============================================================
    // RENDERWORLD_PORTALS
    // ============================================================
    viewEntity_t*   SetEntityDefViewEntity( idRenderEntityLocal* def );
    viewLight_t*    SetLightDefViewLight( idRenderLightLocal* def );

private:
    uint32_t                smpFrame;
    idFrameData		        smpFrameData[NUM_FRAME_DATA];
    idFrameData*            frameData;
    viewDef_t* 				viewDef;
    idParallelJobList* 		frontEndJobList;
    frontEndCounters_t      pc;					// frontend performance counters
	
    // tr_frontend_main.cpp
    void            ViewStatistics( viewDef_t* parms );
    void            SortDrawSurfs( drawSurf_t** drawSurfs, const int numDrawSurfs );

    // tr_frontend_addlights.cpp
    void            SetupSplitFrustums( viewDef_t* viewDef );
    void            AddSingleLight( viewLight_t* vLight );
    static void     R_AddSingleLight( viewLight_t* vLight );
    static void     ShadowBounds( const idBounds& modelBounds, const idBounds& lightBounds, const idVec3& lightOrigin, idBounds& shadowBounds );


    // tr_frontend_addmodels.cpp
    viewEntity_t*   SortViewEntities( viewEntity_t* vEntities );
    void            AddSingleModel( viewEntity_t* vEntity );
    static void     R_AddSingleModel( viewEntity_t* vEntity );

    // tr_frontend_deform.cpp
    drawSurf_t*     FinishDeform( drawSurf_t* surf, crDrawGeometry* newTri, const idDrawVert* newVerts, const triIndex_t* newIndexes );
    drawSurf_t*     AutospriteDeform( drawSurf_t* surf );
    drawSurf_t*     TubeDeform( drawSurf_t* surf );
    drawSurf_t*     FlareDeform( drawSurf_t* surf );
    drawSurf_t*     ExpandDeform( drawSurf_t* surf );
    drawSurf_t*     MoveDeform( drawSurf_t* surf );
    drawSurf_t*     TurbulentDeform( drawSurf_t* surf );
    drawSurf_t*     EyeballDeform( drawSurf_t* surf );
    drawSurf_t*     ParticleDeform( drawSurf_t* surf, bool useArea );

    // tr_frontend_subview.cpp
    viewDef_t*      MirrorViewBySurface( const drawSurf_t* drawSurf );
    viewDef_t*      XrayViewBySurface( const drawSurf_t* drawSurf );
    void            MirrorRender( const drawSurf_t* surf, textureStage_t* stage, idScreenRect scissor );
    void            XrayRender( const drawSurf_t* surf, textureStage_t* stage, idScreenRect scissor );
    void            RemoteRender( const drawSurf_t* surf, textureStage_t* stage );
    bool            GenerateSurfaceSubview( const drawSurf_t* drawSurf );

    // tr_frontend_guisurf.cpp
    void            RenderGuiSurf( idUserInterface* gui, const drawSurf_t* drawSurf );

    // Remove os operadores de cópia e atribuição
    crFrontend( const crFrontend& ) = delete;
    crFrontend& operator=( const crFrontend& ) = delete;
};

#endif //!__FRONTEND_HPP__