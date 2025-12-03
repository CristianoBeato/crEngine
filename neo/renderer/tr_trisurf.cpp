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
#pragma hdrstop
#include "precompiled.h"

#include "renderer_common.h"

/*
==============================================================================

TRIANGLE MESH PROCESSING

The functions in this file have no vertex / index count limits.

Truly identical vertexes that match in position, normal, and texcoord can
be merged away.

Vertexes that match in position and texcoord, but have distinct normals will
remain distinct for all purposes.  This is usually a poor choice for models,
as adding a bevel face will not add any more vertexes, and will tend to
look better.

Match in position and normal, but differ in texcoords are referenced together
for calculating tangent vectors for bump mapping.
Artists should take care to have identical texels in all maps (bump/diffuse/specular)
in this case

Vertexes that only match in position are merged for shadow edge finding.

Degenerate triangles.

Overlapped triangles, even if normals or texcoords differ, must be removed.
for the silhoette based stencil shadow algorithm to function properly.
Is this true???
Is the overlapped triangle problem just an example of the trippled edge problem?

Interpenetrating triangles are not currently clipped to surfaces.
Do they effect the shadows?

if vertexes are intended to deform apart, make sure that no vertexes
are on top of each other in the base frame, or the sil edges may be
calculated incorrectly.

We might be able to identify this from topology.

Dangling edges are acceptable, but three way edges are not.

Are any combinations of two way edges unacceptable, like one facing
the backside of the other?

Topology is determined by a collection of triangle indexes.

The edge list can be built up from this, and stays valid even under
deformations.

Somewhat non-intuitively, concave edges cannot be optimized away, or the
stencil shadow algorithm miscounts.

Face normals are needed for generating shadow volumes and for calculating
the silhouette, but they will change with any deformation.

Vertex normals and vertex tangents will change with each deformation,
but they may be able to be transformed instead of recalculated.

bounding volume, both box and sphere will change with deformation.

silhouette indexes
shade indexes
texture indexes

  shade indexes will only be > silhouette indexes if there is facet shading present

	lookups from texture to sil and texture to shade?

The normal and tangent vector smoothing is simple averaging, no attempt is
made to better handle the cases where the distribution around the shared vertex
is highly uneven.

  we may get degenerate triangles even with the uniquing and removal
  if the vertexes have different texcoords.

==============================================================================
*/

// this shouldn't change anything, but previously renderbumped models seem to need it
#define USE_INVA

// instead of using the texture T vector, cross the normal and S vector for an orthogonal axis
#define DERIVE_UNSMOOTHED_BITANGENT

/*
===================================================================================

DEFORMED SURFACES

===================================================================================
*/

/*
===================
R_BuildDeformInfo
===================
*/
deformInfo_t* R_BuildDeformInfo( int numVerts, const idDrawVert* verts, int numIndexes, const int* indexes, bool useUnsmoothedTangents )
{
	crDrawGeometry	tri;
	std::memset( &tri, 0, sizeof( crDrawGeometry ) );
	
	tri.NumVerts() = numVerts;
	tri.AllocStaticTriSurfVerts( tri.NumVerts() ); // R_AllocStaticTriSurfVerts( &tri, tri.numVerts );
	//SIMDProcessor->Memcpy( tri.Verts(), verts, tri.numVerts * sizeof( tri.Verts()[0] ) );
	std::memcpy( tri.Verts(), verts, tri.NumVerts() * sizeof( idDrawVert ) );


	tri.NumIndexes() = numIndexes;
	tri.AllocStaticTriSurfIndexes( tri.NumIndexes() ); // R_AllocStaticTriSurfIndexes( &tri, tri.NumIndexes() );
	
	// don't std::memcpy, so we can change the index type from int to short without changing the interface
	for( int i = 0; i < tri.NumIndexes(); i++ )
	{
		tri.Indexes()[i] = indexes[i];
	}
	
	tri.RangeCheckIndexes(); 
	tri.CreateSilIndexes(); 
	tri.IdentifySilEdges( false ); // we cannot remove coplanar edges, because they can deform to silhouettes
	tri.DuplicateMirroredVertexes(); // split mirror points into multiple points
	tri.CreateDupVerts(); 
	
	if( useUnsmoothedTangents )
		tri.BuildDominantTris();
		
	tri.DeriveTangents(); // R_DeriveTangents( &tri );
	
	deformInfo_t* deform = ( deformInfo_t* )R_ClearedStaticAlloc( sizeof( *deform ) );
	
	deform->numSourceVerts = numVerts;
	deform->numOutputVerts = tri.NumVerts();
	deform->verts = tri.Verts();
	
	deform->numIndexes = numIndexes;
	deform->indexes = tri.Indexes();
	
	deform->silIndexes = tri.SilIndexes();
	
	deform->numSilEdges = tri.NumSilEdges();
	deform->silEdges = tri.SilEdges();
	
	deform->numMirroredVerts = tri.NumMirroredVerts();
	deform->mirroredVerts = tri.MirroredVerts();
	
	deform->numDupVerts = tri.NumDupVerts();
	deform->dupVerts = tri.DupVerts();
	
	tri.FreeDominantTris();
	
	idShadowVertSkinned* shadowVerts = ( idShadowVertSkinned* ) Mem_Alloc16( ALIGN( deform->numOutputVerts * 2 * sizeof( idShadowVertSkinned ), 16 ), TAG_MODEL );
	idShadowVertSkinned::CreateShadowCache( shadowVerts, deform->verts, deform->numOutputVerts );
	
	deform->staticAmbientCache = vertexCache.AllocStaticVertex( deform->verts, ALIGN( deform->numOutputVerts * sizeof( idDrawVert ), VERTEX_CACHE_ALIGN ) );
	deform->staticIndexCache = vertexCache.AllocStaticIndex( deform->indexes, ALIGN( deform->numIndexes * sizeof( triIndex_t ), INDEX_CACHE_ALIGN ) );
	deform->staticShadowCache = vertexCache.AllocStaticVertex( shadowVerts, ALIGN( deform->numOutputVerts * 2 * sizeof( idShadowVertSkinned ), VERTEX_CACHE_ALIGN ) );
	
	Mem_Free( shadowVerts );
	
	return deform;
}

/*
===================
R_FreeDeformInfo
===================
*/
void R_FreeDeformInfo( deformInfo_t* deformInfo )
{
	if( deformInfo->verts != nullptr )
	{
		Mem_Free( deformInfo->verts );
		deformInfo->verts = nullptr;
	}

	if( deformInfo->indexes != nullptr )
	{
		Mem_Free( deformInfo->indexes );
		deformInfo->indexes = nullptr;
	}
	
	if( deformInfo->silIndexes != nullptr )
	{
		Mem_Free( deformInfo->silIndexes );
		deformInfo->silIndexes = nullptr;
	}
	
	if( deformInfo->silEdges != nullptr )
	{
		Mem_Free( deformInfo->silEdges );
		deformInfo->silEdges = nullptr;
	}
	
	if( deformInfo->mirroredVerts != nullptr )
	{
		Mem_Free( deformInfo->mirroredVerts );
		deformInfo->mirroredVerts = nullptr;
	}
	
	if( deformInfo->dupVerts != nullptr )
	{
		Mem_Free( deformInfo->dupVerts );
		deformInfo->dupVerts = nullptr;
	}

	R_StaticFree( deformInfo );
}

/*
===================
R_DeformInfoMemoryUsed
===================
*/
int R_DeformInfoMemoryUsed( deformInfo_t* deformInfo )
{
	int total = 0;
	
	if( deformInfo->verts != nullptr )
		total += deformInfo->numOutputVerts * sizeof( deformInfo->verts[0] );
	
	if( deformInfo->indexes != nullptr )
		total += deformInfo->numIndexes * sizeof( deformInfo->indexes[0] );
	
	if( deformInfo->mirroredVerts != nullptr )
		total += deformInfo->numMirroredVerts * sizeof( deformInfo->mirroredVerts[0] );
	
	if( deformInfo->dupVerts != nullptr )
		total += deformInfo->numDupVerts * sizeof( deformInfo->dupVerts[0] );

	if( deformInfo->silIndexes != nullptr )
		total += deformInfo->numIndexes * sizeof( deformInfo->silIndexes[0] );

	if( deformInfo->silEdges != nullptr )
		total += deformInfo->numSilEdges * sizeof( deformInfo->silEdges[0] );
	
	total += sizeof( *deformInfo );
	return total;
}

/*
===================================================================================

VERTEX / INDEX CACHING

===================================================================================
*/
