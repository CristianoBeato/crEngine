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
#include "precompiled.h"
#include "renderer_common.h"
#include "Frontend.hpp"

crFrontend::crFrontend( void ) : 
    frameData( nullptr )
{
	REGISTER_PARALLEL_JOB( R_AddSingleLight, "R_AddSingleLight" );
	REGISTER_PARALLEL_JOB( R_AddSingleModel, "R_AddSingleModel" );
}

crFrontend::~crFrontend( void )
{

}

crFrontend *crFrontend::Get(void)
{
	static crFrontend gFrontend = crFrontend();
    return &gFrontend;
}

/*
=================
crFrontend::Init
=================
*/
void crFrontend::Init(void)
{
// foresthale 2014-05-28: due to increased MAX_INTERACTIONS_PER_LIGHT the job limit also has to be increased
#ifdef ID_ALLOW_TOOLS
	frontEndJobList = parallelJobManager->AllocJobList( JOBLIST_RENDERER_FRONTEND, JOBLIST_PRIORITY_MEDIUM, 16384, 0, nullptr );	
#else
	frontEndJobList = parallelJobManager->AllocJobList( JOBLIST_RENDERER_FRONTEND, JOBLIST_PRIORITY_MEDIUM, 2048, 0, nullptr );
#endif
}

/*
=================
crFrontend::Clear
=================
*/
void crFrontend::Clear(void)
{
	///
	ShutdownFrameData();

	viewDef = nullptr;

	/// Clear performance counters
	std::memset( &pc, 0, sizeof( pc ) );

	/// Clear front end job list
	parallelJobManager->FreeJobList( frontEndJobList );
	frontEndJobList = nullptr;
}

/*
==========================================================================================

FRAME MEMORY ALLOCATION

==========================================================================================
*/

constexpr uint32_t FRAME_ALLOC_ALIGNMENT = 128u;
constexpr uint32_t MAX_FRAME_MEMORY = 64u * 1024u * 1024u;	// larger so that we can noclip on PC for dev purposes

//#define TRACK_FRAME_ALLOCS

#if defined( TRACK_FRAME_ALLOCS )
idSysInterlockedInteger frameAllocTypeCount[FRAME_ALLOC_MAX];
int frameHighWaterTypeCount[FRAME_ALLOC_MAX];
#endif

/*
====================
crFrontend::ToggleSmpFrame
====================
*/
void crFrontend::ToggleSmpFrame( void )
{
	// update the highwater mark
	if( frameData->frameMemoryAllocated.GetValue() > frameData->highWaterAllocated )
	{
		frameData->highWaterAllocated = frameData->frameMemoryAllocated.GetValue();
#if defined( TRACK_FRAME_ALLOCS )
		frameData->highWaterUsed = frameData->frameMemoryUsed.GetValue();
		for( int i = 0; i < FRAME_ALLOC_MAX; i++ )
		{
			frameHighWaterTypeCount[i] = frameAllocTypeCount[i].GetValue();
		}
#endif
	}
	
	// switch to the next frame
	smpFrame++;
	frameData = &smpFrameData[smpFrame % NUM_FRAME_DATA];
	
	// reset the memory allocation
	
	// RB: 64 bit fixes, changed unsigned int to uintptr_t
	const uintptr_t bytesNeededForAlignment = FRAME_ALLOC_ALIGNMENT - ( ( uintptr_t )frameData->frameMemory & ( FRAME_ALLOC_ALIGNMENT - 1 ) );
	// RB end
	
	frameData->frameMemoryAllocated.SetValue( bytesNeededForAlignment );
	frameData->frameMemoryUsed.SetValue( 0 );
	
#if defined( TRACK_FRAME_ALLOCS )
	for( int i = 0; i < FRAME_ALLOC_MAX; i++ )
	{
		frameAllocTypeCount[i].SetValue( 0 );
	}
#endif
	
	// clear the command chain and make a RC_NOP command the only thing on the list
	frameData->cmdHead = frameData->cmdTail = ( emptyCommand_t* )FrameAlloc( sizeof( *frameData->cmdHead ), FRAME_ALLOC_DRAW_COMMAND );
	frameData->cmdHead->commandId = RC_NOP;
	frameData->cmdHead->next = nullptr;
}

/*
=====================
crFrontend::ShutdownFrameData
=====================
*/
void crFrontend::ShutdownFrameData( void )
{
	frameData = nullptr;
	for( int i = 0; i < NUM_FRAME_DATA; i++ )
	{
		Mem_Free16( smpFrameData[i].frameMemory );
		smpFrameData[i].frameMemory = nullptr;
	}
}

/*
=====================
crFrontend::InitFrameData
=====================
*/
void crFrontend::InitFrameData( void )
{
	ShutdownFrameData();
	
	for( int i = 0; i < NUM_FRAME_DATA; i++ )
	{
		smpFrameData[i].frameMemory = ( byte* ) Mem_Alloc16( MAX_FRAME_MEMORY, TAG_RENDER );
	}
	
	// must be set before calling R_ToggleSmpFrame()
	frameData = &smpFrameData[ 0 ];
	
	ToggleSmpFrame();
}

/*
================
crFrontend::FrameAlloc

This data will be automatically freed when the
current frame's back end completes.

This should only be called by the front end.  The
back end shouldn't need to allocate memory.

All temporary data, like dynamic tesselations
and local spaces are allocated here.

All memory is cache-line-cleared for the best performance.
================
*/
void* crFrontend::FrameAlloc( const size_t in_bytes, frameAllocType_t type )
{
#if defined( TRACK_FRAME_ALLOCS )
	frameData->frameMemoryUsed.Add( bytes );
	frameAllocTypeCount[type].Add( bytes );
#endif
	
	size_t bytes = ( in_bytes + FRAME_ALLOC_ALIGNMENT - 1 ) & ~( FRAME_ALLOC_ALIGNMENT - 1 );
	
	// thread safe add
	int	end = frameData->frameMemoryAllocated.Add( bytes );
	if( end > MAX_FRAME_MEMORY )
	{
		idLib::Error( "R_FrameAlloc ran out of memory. bytes = %d, end = %d, highWaterAllocated = %d\n", bytes, end, frameData->highWaterAllocated );
	}
	
	byte* ptr = frameData->frameMemory + end - bytes;
	
	// cache line clear the memory
	for( int offset = 0; offset < bytes; offset += CACHE_LINE_SIZE )
	{
		ZeroCacheLine( ptr, offset );
	}
	
	return ptr;
}

/*
==================
crFrontend::ClearedFrameAlloc
==================
*/
void* crFrontend::ClearedFrameAlloc( const size_t bytes, frameAllocType_t type )
{
	// NOTE: every allocation is cache line cleared
	return FrameAlloc( bytes, type );
}

/*
==========================================================================================

FONT-END STATIC MEMORY ALLOCATION

==========================================================================================
*/

/*
=================
crFrontend::StaticAlloc
=================
*/
void* crFrontend::StaticAlloc( const size_t bytes, const memTag_t tag )
{
	pc.c_alloc++;
	
	void* buf = Mem_Alloc( bytes, tag );
	
	// don't exit on failure on zero length allocations since the old code didn't
	if( buf == nullptr && bytes != 0 )
		common->FatalError( "R_StaticAlloc failed on %i bytes", bytes );

	return buf;
}

/*
=================
crFrontend::ClearedStaticAlloc
=================
*/
void* crFrontend::ClearedStaticAlloc( const size_t bytes )
{
	pc.c_alloc++;
	
	void* buf = Mem_ClearedAlloc( bytes, TAG_RENDER );
	
	// don't exit on failure on zero length allocations since the old code didn't
	if( buf == nullptr && bytes != 0 )
		common->FatalError( "R_StaticAlloc failed on %i bytes", bytes );
		
	return buf;
}

/*
=================
crFrontend::StaticFree
=================
*/
void crFrontend::StaticFree( void* data )
{
	pc.c_free++;
	Mem_Free( data );
}

/*
==========================================================================================
FONT-END RENDERING
==========================================================================================
*/

/*
=================
crFrontend::ViewStatistics
=================
*/
void crFrontend::ViewStatistics( viewDef_t* parms )
{
	// report statistics about this view
	if( !r_showSurfaces.GetBool() )
		return;
	
	common->Printf( "view:%p surfs:%i\n", parms, parms->numDrawSurfs );
}

/*
============
crFrontend::GetCommandBuffer

Returns memory for a command buffer (stretchPicCommand_t,
drawSurfsCommand_t, etc) and links it to the end of the
current command chain.
============
*/
void* crFrontend::GetCommandBuffer( const size_t bytes )
{
	emptyCommand_t*	cmd = nullptr;
	cmd = ( emptyCommand_t* )FrameAlloc( bytes, FRAME_ALLOC_DRAW_COMMAND );
	cmd->next = nullptr;
	frameData->cmdTail->next = &cmd->commandId;
	frameData->cmdTail = cmd;
	return ( void* )cmd;
}

/*
=============
crFrontend::AddDrawViewCmd

This is the main 3D rendering command.  A single scene may
have multiple views if a mirror, portal, or dynamic texture is present.
=============
*/
void crFrontend::AddDrawViewCmd( viewDef_t* parms, const bool guiOnly )
{
	drawSurfsCommand_t*	cmd = nullptr;
	
	cmd = ( drawSurfsCommand_t* )GetCommandBuffer( sizeof( *cmd ) );
	cmd->commandId = ( guiOnly ) ? RC_DRAW_VIEW_GUI : RC_DRAW_VIEW_3D;
	
	cmd->viewDef = parms;
	
	pc.c_numViews++;
	
	ViewStatistics( parms );
}

/*
=============
crFrontend::AddPostProcess

This issues the command to do a post process after all the views have
been rendered.
=============
*/
void crFrontend::AddDrawPostProcess( viewDef_t* parms )
{
	postProcessCommand_t* cmd = ( postProcessCommand_t* )GetCommandBuffer( sizeof( *cmd ) );
	cmd->commandId = RC_POST_PROCESS;
	cmd->viewDef = parms;
}

/*
=================
idFrameData::SortDrawSurfs
=================
*/
void crFrontend::SortDrawSurfs( drawSurf_t** drawSurfs, const int numDrawSurfs )
{
#if 1

	uint64_t* indices = ( uint64_t* ) _alloca16( numDrawSurfs * sizeof( indices[0] ) );
	
	// sort the draw surfs based on:
	// 1. sort value (largest first)
	// 2. depth (smallest first)
	// 3. index (largest first)
	assert( numDrawSurfs <= 0xFFFF );
	for( int i = 0; i < numDrawSurfs; i++ )
	{
		float sort = SS_POST_PROCESS - drawSurfs[i]->sort;
		assert( sort >= 0.0f );
		
		uint64_t dist = 0;
		if( drawSurfs[i]->frontEndGeo != nullptr )
		{
			float min = 0.0f;
			float max = 1.0f;
			idRenderMatrix::DepthBoundsForBounds( min, max, drawSurfs[i]->space->mvp, drawSurfs[i]->frontEndGeo->Bounds() );
			dist = idMath::Ftoui16( min * 0xFFFF );
		}
		
		indices[i] = ( ( numDrawSurfs - i ) & 0xFFFF ) | ( dist << 16 ) | ( ( uint64_t )( *( uint32_t* )&sort ) << 32 );
	}
	
	const int64_t MAX_LEVELS = 128;
	int64_t lo[MAX_LEVELS];
	int64_t hi[MAX_LEVELS];
	
	// Keep the top of the stack in registers to avoid load-hit-stores.
	int64_t st_lo = 0;
	int64_t st_hi = numDrawSurfs - 1;
	int64_t level = 0;
	
	for( ; ; )
	{
		int64_t i = st_lo;
		int64_t j = st_hi;
		if( j - i >= 4 && level < MAX_LEVELS - 1 )
		{
			uint64_t pivot = indices[( i + j ) / 2];
			do
			{
				while( indices[i] > pivot ) i++;
				while( indices[j] < pivot ) j--;
				if( i > j ) break;
				uint64_t h = indices[i];
				indices[i] = indices[j];
				indices[j] = h;
			}
			while( ++i <= --j );
			
			// No need for these iterations because we are always sorting unique values.
			//while ( indices[j] == pivot && st_lo < j ) j--;
			//while ( indices[i] == pivot && i < st_hi ) i++;
			
			assert( level < MAX_LEVELS - 1 );
			lo[level] = i;
			hi[level] = st_hi;
			st_hi = j;
			level++;
		}
		else
		{
			for( ; i < j; j-- )
			{
				int64_t m = i;
				for( int64_t k = i + 1; k <= j; k++ )
				{
					if( indices[k] < indices[m] )
					{
						m = k;
					}
				}
				uint64_t h = indices[m];
				indices[m] = indices[j];
				indices[j] = h;
			}
			if( --level < 0 )
			{
				break;
			}
			st_lo = lo[level];
			st_hi = hi[level];
		}
	}
	
	drawSurf_t** newDrawSurfs = ( drawSurf_t** ) indices;
	for( int i = 0; i < numDrawSurfs; i++ )
	{
		newDrawSurfs[i] = drawSurfs[numDrawSurfs - ( indices[i] & 0xFFFF )];
	}
	std::memcpy( drawSurfs, newDrawSurfs, numDrawSurfs * sizeof( drawSurfs[0] ) );
	
#else
	
	struct local_t
	{
		static int R_QsortSurfaces( const void* a, const void* b )
		{
			const drawSurf_t* ea = *( drawSurf_t** )a;
			const drawSurf_t* eb = *( drawSurf_t** )b;
			if( ea->sort < eb->sort )
			{
				return -1;
			}
			if( ea->sort > eb->sort )
			{
				return 1;
			}
			return 0;
		}
	};
	
	// Add a sort offset so surfaces with equal sort orders still deterministically
	// draw in the order they were added, at least within a given model.
	float sorfOffset = 0.0f;
	for( int i = 0; i < numDrawSurfs; i++ )
	{
		drawSurf[i]->sort += sorfOffset;
		sorfOffset += 0.000001f;
	}
	
	// sort the drawsurfs
	qsort( drawSurfs, numDrawSurfs, sizeof( drawSurfs[0] ), local_t::R_QsortSurfaces );
	
#endif
}

// RB begin
void crFrontend::SetupSplitFrustums( viewDef_t* viewDef )
{
	idVec3			planeOrigin;
	
	const float zNearStart = ( viewDef->renderView.cramZNear ) ? ( r_znear.GetFloat() * 0.25f ) : r_znear.GetFloat();
	float zFarEnd = 10000;
	
	float zNear = zNearStart;
	float zFar = zFarEnd;
	
	float lambda = r_shadowMapSplitWeight.GetFloat();
	float ratio = zFarEnd / zNearStart;
	
	for( int i = 0; i < 6; i++ )
	{
		viewDef->frustumSplitDistances[i] = idMath::INFINITY;
	}
	
	for( int i = 1; i <= ( r_shadowMapSplits.GetInteger() + 1 ) && i < MAX_FRUSTUMS; i++ )
	{
		float si = i / ( float )( r_shadowMapSplits.GetInteger() + 1 );
		
		if( i > FRUSTUM_CASCADE1 )
			zNear = zFar - ( zFar * 0.005f );
		
		zFar = 1.005f * lambda * ( zNearStart * powf( ratio, si ) ) + ( 1 - lambda ) * ( zNearStart + ( zFarEnd - zNearStart ) * si );
		
		if( i <= r_shadowMapSplits.GetInteger() )
			viewDef->frustumSplitDistances[i - 1] = zFar;
		
		float projectionMatrix[16];
		R_SetupProjectionMatrix2( viewDef, zNear, zFar, projectionMatrix );
		
		// setup render matrices for faster culling
		idRenderMatrix projectionRenderMatrix;
		idRenderMatrix::Transpose( *( idRenderMatrix* )projectionMatrix, projectionRenderMatrix );
		idRenderMatrix viewRenderMatrix;
		idRenderMatrix::Transpose( *( idRenderMatrix* )viewDef->worldSpace.modelViewMatrix, viewRenderMatrix );
		idRenderMatrix::Multiply( projectionRenderMatrix, viewRenderMatrix, viewDef->frustumMVPs[i] );
		
		// the planes of the view frustum are needed for portal visibility culling
		idRenderMatrix::GetFrustumPlanes( viewDef->frustums[i], viewDef->frustumMVPs[i], false, true );
		
		// the DOOM 3 frustum planes point outside the frustum
		for( int j = 0; j < 6; j++ )
		{
			viewDef->frustums[i][j] = - viewDef->frustums[i][j];
		}
		
		// remove the Z-near to avoid portals from being near clipped
		if( i == FRUSTUM_CASCADE1 )
			viewDef->frustums[i][4][3] -= r_znear.GetFloat();
	}
}
// RB end

/*
================
crFrontend::RenderView

A view may be either the actual camera view,
a mirror / remote location, or a 3D view on a gui surface.

Parms will typically be allocated with R_FrameAlloc
================
*/
void crFrontend::RenderView( viewDef_t* parms )
{
	// save view in case we are a subview
	viewDef_t* oldView = viewDef;
	
	viewDef = parms;
	
	// we need to set the projection matrix before doing
	// portal-to-screen scissor calculations
	if (!parms->isObliqueProjection)
	{
		// setup the matrix for world space to eye space
		R_SetupViewMatrix(viewDef);
		R_SetupProjectionMatrix(viewDef);
	}
	
	// setup render matrices for faster culling
	idRenderMatrix::Transpose( *( idRenderMatrix* )viewDef->projectionMatrix, viewDef->projectionRenderMatrix );
	idRenderMatrix viewRenderMatrix;
	idRenderMatrix::Transpose( *( idRenderMatrix* )viewDef->worldSpace.modelViewMatrix, viewRenderMatrix );
	idRenderMatrix::Multiply( viewDef->projectionRenderMatrix, viewRenderMatrix, viewDef->worldSpace.mvp );
	
	// the planes of the view frustum are needed for portal visibility culling
	idRenderMatrix::GetFrustumPlanes( viewDef->frustums[FRUSTUM_PRIMARY], viewDef->worldSpace.mvp, false, true );
	
	// the DOOM 3 frustum planes point outside the frustum
	for( int i = 0; i < 6; i++ )
	{
		viewDef->frustums[FRUSTUM_PRIMARY][i] = - viewDef->frustums[FRUSTUM_PRIMARY][i];
	}
	// remove the Z-near to avoid portals from being near clipped
	viewDef->frustums[FRUSTUM_PRIMARY][4][3] -= r_znear.GetFloat();
	
	// RB begin
	SetupSplitFrustums( viewDef );
	// RB end
	
	// identify all the visible portal areas, and create view lights and view entities
	// for all the the entityDefs and lightDefs that are in the visible portal areas
	static_cast<idRenderWorldLocal*>( parms->renderWorld )->FindViewLightsAndEntities();
	
	// wait for any shadow volume jobs from the previous frame to finish
	frontEndJobList->Wait();
	
	// make sure that interactions exist for all light / entity combinations that are visible
	// add any pre-generated light shadows, and calculate the light shader values
	AddLights();
	
	// adds ambient surfaces and create any necessary interaction surfaces to add to the light lists
	AddModels();
	
	// build up the GUIs on world surfaces
	AddInGameGuis( viewDef->drawSurfs, viewDef->numDrawSurfs );
	
	// any viewLight that didn't have visible surfaces can have it's shadows removed
	OptimizeViewLightsList();
	
	// sort all the ambient surfaces for translucency ordering
	SortDrawSurfs( viewDef->drawSurfs, viewDef->numDrawSurfs );
	
	// generate any subviews (mirrors, cameras, etc) before adding this view
	if( GenerateSubViews( viewDef->drawSurfs, viewDef->numDrawSurfs ) )
	{
		// if we are debugging subviews, allow the skipping of the main view draw
		if( r_subviewOnly.GetBool() )
			return;
	}
	
	// write everything needed to the demo file
	if( common->WriteDemo() )
		static_cast<idRenderWorldLocal*>( parms->renderWorld )->WriteVisibleDefs( viewDef );
	
	// add the rendering commands for this viewDef
	AddDrawViewCmd( parms, false );
	
	// restore view in case we are a subview
	viewDef = oldView;
}

/*
================
crFrontend::RenderPostProcess

Because R_RenderView may be called by subviews we have to make sure the post process
pass happens after the active view and its subviews is done rendering.
================
*/
void crFrontend::RenderPostProcess( viewDef_t* parms )
{
	viewDef_t* oldView = viewDef;
	
	AddDrawPostProcess( parms );
	
	viewDef = oldView;
}
