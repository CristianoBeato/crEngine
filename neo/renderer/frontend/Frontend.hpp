
#ifndef __FRONTEND_HPP__
#define __FRONTEND_HPP__

class crDrawGeometry;
class idMaterial;
class idRenderEntityLocal;
struct viewEntity_t;

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
	uint32_t				numIndexes;
	vertCacheHandle_t		indexCache;			// triIndex_t
	vertCacheHandle_t		ambientCache;		// idDrawVert
	vertCacheHandle_t		shadowCache;		// idShadowVert / idShadowVertSkinned
	joint_cache_t           jointCache;			// idJointMat
	const viewEntity_t* 	space;
	const idMaterial* 		material;			// may be nullptr for shadow volumes
	float					sort;				// material->sort, modified by gui / entity sort offsets
	const float*            shaderRegisters;	// evaluated and adjusted for referenceShaders
	drawSurf_t* 			nextOnLight;		// viewLight chains
	drawSurf_t** 			linkChain;			// defer linking to lights to a serial section to avoid a mutex
	idScreenRect			scissorRect;		// for scissor clipping, local inside renderView viewport
	int						renderZFail;
	volatile shadowVolumeState_t shadowVolumeState;
};

constexpr uint32_t NUM_FRAME_DATA = MAX_SMP_FRAMES;

class crFrontend
{
public:
    crFrontend( void );
    ~crFrontend( void );
    static crFrontend* Get( void );

    void    AddDrawViewCmd( viewDef_t* parms, const bool guiOnly );
    void    AddDrawPostProcess( viewDef_t* parms );
    void*   GetCommandBuffer( const size_t bytes );

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

    void    RenderView( viewDef_t* parms );
    void    RenderPostProcess( viewDef_t* parms );

    void    InitMaterials( void );

    // ============================================================
    // TR_FRONTEND_ADDLIGHTS
    // ============================================================
    void    ShadowBounds( const idBounds& modelBounds, const idBounds& lightBounds, const idVec3& lightOrigin, idBounds& shadowBounds );
    void    AddLights( void );
    void    OptimizeViewLightsList( void );

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
    void SurfaceToTextureAxis( const crDrawGeometry* tri, idVec3& origin, idVec3 axis[3] );
    void AddInGameGuis( const drawSurf_t* const drawSurfs[], const int numDrawSurfs );

    // ============================================================
    // TR_FRONTEND_SUBVIEW
    // ============================================================
    bool PreciseCullSurface( const drawSurf_t* drawSurf, idBounds& ndcBounds );
    bool GenerateSubViews( const drawSurf_t* const drawSurfs[], const int numDrawSurfs );

private:
    uint32_t                smpFrame;
    idFrameData		        smpFrameData[NUM_FRAME_DATA];
    idFrameData*            frameData;
    viewDef_t* 				viewDef;
    idParallelJobList* 		frontEndJobList;
    performanceCounters_t   pc;					// frontend performance counters
	
    void    ViewStatistics( viewDef_t* parms );
    void    ToggleSmpFrame( void );
    void    SortDrawSurfs( drawSurf_t** drawSurfs, const int numDrawSurfs );

    void    SetupSplitFrustums( viewDef_t* viewDef );
    void    AddSingleLight( viewLight_t* vLight );

    void    AddSingleModel( viewEntity_t* vEntity );

    static void R_AddSingleLight( viewLight_t* vLight );
    static void R_AddSingleModel( viewEntity_t* vEntity );
    static void ShadowBounds( const idBounds& modelBounds, const idBounds& lightBounds, const idVec3& lightOrigin, idBounds& shadowBounds );
};



#endif //!__FRONTEND_HPP__