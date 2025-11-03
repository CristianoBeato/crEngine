/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos

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
along with crEngine Source Code. If not, see <http://www.gnu.org/licenses/>.

In addition, the crEngine Source Code is also subject to certain additional terms. 
You should have received a copy of these additional terms immediately following the 
terms and conditions of the GNU General Public License which accompanied the crEngine
Source Code.

===========================================================================
*/
#include "precompiled.h"

#include "renderer/tr_local.h"
#include "Geometry.h"

constexpr int MAX_SIL_EDGES = 0x7ffff;

struct tangentVert_t
{
	bool	polarityUsed[2];
	int		negativeRemap;
};

typedef struct
{
	int		vertexNum;
	int		faceNum;
} indexSort_t;

crDrawGeometry::crDrawGeometry( void ) :
    facePlanesCalculated( false ),
    generateNormals( false ),
    tangentsCalculated( false ),
    perfectHull( false ),
    referencedVerts( false ),
    referencedIndexes( false ),
    numVerts( 0 ),
    numIndexes( 0 ),
    numMirroredVerts( 0 ),
    numSilEdges( 0 ),
    numDupVerts( 0 ),
    numShadowIndexesNoFrontCaps( 0 ),
    numShadowIndexesNoCaps( 0 ),
    shadowCapPlaneBits( 0 ),
    indexCache( 0 ),
    ambientCache( 0 ),
    shadowCache( 0 ),
    verts( nullptr ),
    indexes( nullptr ),
    silIndexes( nullptr ),
    mirroredVerts( nullptr ),
    dupVerts( nullptr ),
    silEdges( nullptr ),
    dominantTris( nullptr ),
    preLightShadowVertexes( nullptr ),
    staticShadowVertexes( nullptr ),
    ambientSurface( nullptr ),
    nextDeferredFree( nullptr ),
    staticModelWithJoints( nullptr ),
    facePlanes( nullptr )
{
}

crDrawGeometry::~crDrawGeometry(void)
{
    Clear();

}

void crDrawGeometry::Clear(void)
{
// @CristianoBeato : just to check if we have release data before release
#if 1
	assert( facePlanes == nullptr );
	assert( preLightShadowVertexes == nullptr );
	assert( staticShadowVertexes == nullptr );
	assert( !referencedVerts && verts == nullptr );
	assert( !referencedIndexes && indexes == nullptr );
#endif


	FreeStaticTriSurfVertexCaches();
}

/*
=================
crDrawGeometry::AllocStaticTriSurfVerts
=================
*/
void crDrawGeometry::AllocStaticTriSurfVerts( const uint32_t numVerts )
{
	assert( verts == nullptr );
	verts = static_cast<idDrawVert*>( Mem_Alloc16( numVerts * sizeof( idDrawVert ), TAG_TRI_VERTS ) );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfIndexes
=================
*/
void crDrawGeometry::AllocStaticTriSurfIndexes( const uint32_t numIndexes )
{
	assert( indexes == nullptr );
	indexes = static_cast<triIndex_t*>( Mem_Alloc16( numIndexes * sizeof( triIndex_t ), TAG_TRI_INDEXES ) );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfPreLightShadowVerts
=================
*/
void crDrawGeometry::AllocStaticTriSurfPreLightShadowVerts( const uint32_t numVerts )
{
	assert( preLightShadowVertexes == nullptr );
	preLightShadowVertexes = static_cast<idShadowVert*>( Mem_Alloc16( numVerts * sizeof( idShadowVert ), TAG_TRI_SHADOW ) );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfSilIndexes
=================
*/
void crDrawGeometry::AllocStaticTriSurfSilIndexes( const uint32_t numIndexes )
{
	assert( silIndexes == nullptr );
	silIndexes = static_cast<triIndex_t*>( Mem_Alloc16( numIndexes * sizeof( triIndex_t ), TAG_TRI_SIL_INDEXES ) );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfDominantTris
=================
*/
void crDrawGeometry::AllocStaticTriSurfDominantTris( const uint32_t numVerts )
{
	assert( dominantTris == nullptr );
	dominantTris = static_cast<dominantTri_t*>( Mem_Alloc( numVerts * sizeof( dominantTri_t ), TAG_TRI_DOMINANT_TRIS ) );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfSilEdges
=================
*/
void crDrawGeometry::AllocStaticTriSurfSilEdges( const uint32_t numSilEdges )
{
	assert( silEdges == nullptr );
	silEdges = ( silEdge_t* )Mem_Alloc16( numSilEdges * sizeof( silEdge_t ), TAG_TRI_SIL_EDGE );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfMirroredVerts
=================
*/
void crDrawGeometry::AllocStaticTriSurfMirroredVerts( const uint32_t numMirroredVerts )
{
	assert( mirroredVerts == nullptr );
	mirroredVerts = static_cast<int*>( Mem_Alloc16( numMirroredVerts * sizeof( int ), TAG_TRI_MIR_VERT ) );
}

/*
=================
crDrawGeometry::AllocStaticTriSurfDupVerts
=================
*/
void crDrawGeometry::AllocStaticTriSurfDupVerts( const uint32_t numDupVerts )
{
	assert( dupVerts == nullptr );
	dupVerts = static_cast<int*>( Mem_Alloc16( numDupVerts * 2 * sizeof( int ), TAG_TRI_DUP_VERT ) );
}

/*
=================
crDrawGeometry::ResizeStaticTriSurfVerts
=================
*/
void crDrawGeometry::ResizeStaticTriSurfVerts( const uint32_t numVerts )
{
	idDrawVert* newVerts = static_cast<idDrawVert*>( Mem_Alloc16( numVerts * sizeof( idDrawVert ), TAG_TRI_VERTS ) );
	const int copy = std::min( numVerts, numVerts );
	std::memcpy( newVerts, verts, copy * sizeof( idDrawVert ) );
	Mem_Free16( verts );
	verts = newVerts;
}

/*
=================
crDrawGeometry::ResizeStaticTriSurfIndexes
=================
*/
void crDrawGeometry::ResizeStaticTriSurfIndexes( const uint32_t indexesCount )
{
	triIndex_t* newIndexes = static_cast<triIndex_t*>( Mem_Alloc16( indexesCount * sizeof( triIndex_t ), TAG_TRI_INDEXES ) );
	const uint32_t copy = std::min( indexesCount, numIndexes );
	std::memcpy( newIndexes, indexes, copy * sizeof( triIndex_t ) );
	Mem_Free16( indexes );
	indexes = newIndexes;
}

/*
=================
crDrawGeometry::ReferenceStaticTriSurfVerts
=================
*/
void crDrawGeometry::ReferenceStaticTriSurfVerts( const crDrawGeometry* reference )
{
	verts = reference->verts;
}

/*
=================
crDrawGeometry::ReferenceStaticTriSurfIndexes
=================
*/
void crDrawGeometry::ReferenceStaticTriSurfIndexes( const crDrawGeometry* reference )
{
	indexes = reference->indexes;
}


/*
==============
crDrawGeometry::FreeStaticTriSurf
==============
*/
void crDrawGeometry::FreeStaticTriSurf( void )
{
	FreeStaticTriSurfVertexCaches();
	
	if( !referencedVerts )
		FreeStaticTriSurfVerts();
	
	if ( !facePlanes ) 
	{
		Mem_Free( facePlanes );
		facePlanes = nullptr;
	}

	if( !referencedIndexes )
	{
		FreeStaticTriSurfVerts();

        FreeStaticTriSurfSilIndexes();

		if( silEdges != nullptr )
        {
			Mem_Free16( silEdges );
            silEdges = nullptr;
        }

		if( dominantTris != nullptr )
        {
			Mem_Free16( dominantTris );
            dominantTris = nullptr;
        }

		if( mirroredVerts != nullptr )
        {
			Mem_Free16( mirroredVerts );
            mirroredVerts = nullptr;
        }

		if( dupVerts != nullptr )
        {
			Mem_Free16( dupVerts );
            dupVerts = nullptr;
        }
	}
	
	if( preLightShadowVertexes != nullptr )
    {
		Mem_Free16( preLightShadowVertexes );
        preLightShadowVertexes = nullptr;
    }

	if( staticShadowVertexes != nullptr )
    {
		Mem_Free16( staticShadowVertexes );
        staticShadowVertexes = nullptr;
    }	
}

/*
=================
crDrawGeometry::FreeStaticTriSurfSilIndexes
=================
*/
void crDrawGeometry::FreeStaticTriSurfSilIndexes( void )
{
    if( silIndexes )
    {
	    Mem_Free16( silIndexes );
	    silIndexes = nullptr;
    }
}

/*
==============
crDrawGeometry::FreeStaticTriSurfVerts
==============
*/
void crDrawGeometry::FreeStaticTriSurfVerts( void )
{
	// we don't support reclaiming static geometry memory
	// without a level change
	ambientCache = 0;
	
	if( verts != nullptr )
	{
		// R_CreateLightTris points verts at the verts of the ambient surface
		if( ambientSurface == nullptr || verts != ambientSurface->verts )
		{
			Mem_Free16( verts );
            verts = nullptr;
		}
	}
}

/*
==============
crDrawGeometry::FreeStaticTriSurfVertexCaches
==============
*/
void crDrawGeometry::FreeStaticTriSurfVertexCaches( void )
{
	// we don't support reclaiming static geometry memory
	// without a level change
	FreeIndexCache();
	FreeAmbientCache();
	FreeShadowCache();
}

/*
==============
crDrawGeometry::FreeDominantTris
==============
*/
void crDrawGeometry::FreeDominantTris(void)
{
	if( dominantTris != nullptr )
	{
		Mem_Free( dominantTris );
		dominantTris = nullptr;
	}
}

/*
==============
crDrawGeometry::CreateIndexCache
==============
*/
void crDrawGeometry::CreateIndexCache( const triIndex_t* newIndexes )
{
	indexCache = vertexCache.AllocIndex( newIndexes, ALIGN( numIndexes * sizeof( triIndex_t ), INDEX_CACHE_ALIGN ) );	
}

/*
==============
crDrawGeometry::CreateAmbientCache
==============
*/
void crDrawGeometry::CreateAmbientCache( const idDrawVert* newVerts )
{
	ambientCache = vertexCache.AllocVertex( newVerts, ALIGN( numVerts * sizeof( idDrawVert ), VERTEX_CACHE_ALIGN ) );
}

/*
==============
crDrawGeometry::CreateShadowCache
==============
*/
void crDrawGeometry::CreateShadowCache( const idShadowVert* newVerts )
{
	shadowCache = vertexCache.AllocVertex( newVerts, ALIGN( numVerts * 2 * sizeof( idShadowVert ), VERTEX_CACHE_ALIGN ) );	
}

/*
==============
crDrawGeometry::FreeIndexCache
==============
*/
void crDrawGeometry::FreeIndexCache( void )
{
	indexCache = 0;
}

/*
==============
crDrawGeometry::FreeAmbientCache
==============
*/
void crDrawGeometry::FreeAmbientCache( void )
{
	ambientCache = 0;
}

/*
==============
crDrawGeometry::FreeShadowCache
==============
*/
void crDrawGeometry::FreeShadowCache( void )
{
	shadowCache = 0;
}

/*
=================
crDrawGeometry::TriSurfMemory

For memory profiling
=================
*/
size_t crDrawGeometry::TriSurfMemory( void ) const
{
	size_t total = 0;
	
	if( preLightShadowVertexes != nullptr )
		total += numVerts * 2 * sizeof( idShadowVert );
	
	if( staticShadowVertexes != nullptr )
		total += numVerts * 2 * sizeof( idShadowVert );

	if( verts != nullptr )
	{
		if( ambientSurface == nullptr || verts != ambientSurface->verts )
			total += numVerts * sizeof( idDrawVert );
	}
	if( indexes != nullptr )
	{
		if( ambientSurface == nullptr || indexes != ambientSurface->indexes )
			total += numIndexes * sizeof( triIndex_t );
	}
	if( silIndexes != nullptr ) total += numIndexes * sizeof( triIndex_t );
	if( silEdges != nullptr ) total += numSilEdges * sizeof( silEdge_t );
	if( dominantTris != nullptr ) total += numVerts * sizeof( dominantTri_t );
	if( mirroredVerts != nullptr ) total += numMirroredVerts * sizeof( int );
	if( dupVerts != nullptr ) total += numDupVerts * sizeof( int );
	
	total += sizeof( crDrawGeometry );
	
	return total;
}

/*
=================
crDrawGeometry::BoundTriSurf
=================
*/
void crDrawGeometry::BoundTriSurf( void )
{
	SIMDProcessor->MinMax( bounds[0], bounds[1], verts, numVerts );
}


/*
=================
crDrawGeometry::RemoveDuplicatedTriangles

silIndexes must have already been calculated

silIndexes are used instead of indexes, because duplicated
triangles could have different texture coordinates.
=================
*/
void crDrawGeometry::RemoveDuplicatedTriangles( void )
{
	int		c_removed = 0;
	int		i = 0, j = 0, r = 0;
	int		a = 0, b = 0, c = 0;
	
	c_removed = 0;
	
	// check for completely duplicated triangles
	// any rotation of the triangle is still the same, but a mirroring
	// is considered different
	for( i = 0; i < numIndexes; i += 3 )
	{
		for( r = 0; r < 3; r++ )
		{
			a = silIndexes[i + r];
			b = silIndexes[i + ( r + 1 ) % 3];
			c = silIndexes[i + ( r + 2 ) % 3];
			for( j = i + 3; j < numIndexes; j += 3 )
			{
				if( silIndexes[j] == a && silIndexes[j + 1] == b && silIndexes[j + 2] == c )
				{
					c_removed++;
					std::memmove( indexes + j, indexes + j + 3, ( numIndexes - j - 3 ) * sizeof( triIndex_t ) );
					std::memmove( silIndexes + j, silIndexes + j + 3, ( numIndexes - j - 3 ) * sizeof( triIndex_t ) );
					numIndexes -= 3;
					j -= 3;
				}
			}
		}
	}
	
	if( c_removed )
	{
		common->Printf( "removed %i duplicated triangles\n", c_removed );
	}
}

/*
=================
crDrawGeometry::CreateSilIndexes

Uniquing vertexes only on xyz before creating sil edges reduces
the edge count by about 20% on Q3 models
=================
*/
void crDrawGeometry::CreateSilIndexes( void )
{
	int	i = 0;
	int* remap = nullptr;
	
	if( silIndexes != nullptr )
	{
		Mem_Free16( silIndexes );
		silIndexes = nullptr;
	}

	remap = CreateSilRemap();
	
	// remap indexes to the first one
	AllocStaticTriSurfSilIndexes( numIndexes );
	assert( silIndexes != nullptr );
	for( i = 0; i < numIndexes; i++ )
	{
		silIndexes[i] = remap[indexes[i]];
	}
	
	R_StaticFree( remap );
}

/*
=================
crDrawGeometry::RemoveDegenerateTriangles

silIndexes must have already been calculated
=================
*/
void crDrawGeometry::RemoveDegenerateTriangles( void )
{
	int		c_removed = 0;
	int		i = 0;
	int		a = 0, b = 0, c = 0;
	
	assert( silIndexes != nullptr );
	
	// check for completely degenerate triangles
	c_removed = 0;
	for( i = 0; i < numIndexes; i += 3 )
	{
		a = silIndexes[i];
		b = silIndexes[i + 1];
		c = silIndexes[i + 2];
		if( a == b || a == c || b == c )
		{
			c_removed++;
			std::memmove( indexes + i, indexes + i + 3, ( numIndexes - i - 3 ) * sizeof( indexes[0] ) );
			std::memmove( silIndexes + i, silIndexes + i + 3, ( numIndexes - i - 3 ) * sizeof( silIndexes[0] ) );
			numIndexes -= 3;
			i -= 3;
		}
	}
	
	// this doesn't free the memory used by the unused verts
	if( c_removed )
		common->Printf( "removed %i degenerate triangles\n", c_removed );
}

/*
=================
crDrawGeometry::RemoveUnusedVerts
=================
*/
void crDrawGeometry::RemoveUnusedVerts( void )
{
	int		i = 0;
	int		index = 0;
	int		used = 0;
	int*	mark = nullptr;
	
	mark = static_cast<int*>( R_ClearedStaticAlloc( numVerts * sizeof( int ) ) );
	
	for( i = 0; i < numIndexes; i++ )
	{
		index = indexes[i];
		if( index < 0 || index >= numVerts )
			common->Error( "R_RemoveUnusedVerts: bad index" );
		
		mark[ index ] = 1;
		
		if( silIndexes )
		{
			index = silIndexes[i];
			if( index < 0 || index >= numVerts )
				common->Error( "R_RemoveUnusedVerts: bad index" );
			
			mark[ index ] = 1;
		}
	}
	
	used = 0;
	for( i = 0; i < numVerts; i++ )
	{
		if( !mark[i] )
			continue;
		
		mark[i] = used + 1;
		used++;
	}
	
	if( used != numVerts )
	{
		for( i = 0; i < numIndexes; i++ )
		{
			indexes[i] = mark[ indexes[i] ] - 1;
			if( silIndexes )
				silIndexes[i] = mark[ silIndexes[i] ] - 1;
			
		}
		numVerts = used;
		
		for( i = 0; i < numVerts; i++ )
		{
			index = mark[ i ];
			if( !index )
				continue;
			
			verts[ index - 1 ] = verts[i];
		}
		
		// this doesn't realloc the arrays to save the memory used by the unused verts
	}
	
	R_StaticFree( mark );
}

/*
===============
crDrawGeometry::RangeCheckIndexes

Check for syntactically incorrect indexes, like out of range values.
Does not check for semantics, like degenerate triangles.

No vertexes is acceptable if no indexes.
No indexes is acceptable.
More vertexes than are referenced by indexes are acceptable.
===============
*/
void crDrawGeometry::RangeCheckIndexes( void )
{
	int i = 0;
	
	if( numIndexes < 0 )
		common->Error( "crDrawGeometry::RangeCheckIndexes: numIndexes < 0" );
	
	if( numVerts < 0 )
		common->Error( "crDrawGeometry::RangeCheckIndexes: numVerts < 0" );
	
	// must specify an integral number of triangles
	if( numIndexes % 3 != 0 )
		common->Error( "crDrawGeometry::RangeCheckIndexes: numIndexes %% 3" );
	
	for( i = 0; i < numIndexes; i++ )
	{
		if( indexes[i] >= numVerts )
			common->Error( "crDrawGeometry::RangeCheckIndexes: index out of range" );
	}
	
	// this should not be possible unless there are unused verts
	if( numVerts > numIndexes )
	{
		// FIXME: find the causes of these
		// common->Printf( "crDrawGeometry::RangeCheckIndexes: numVerts > numIndexes\n" );
	}
}

/*
=====================
crDrawGeometry::CreateVertexNormals

Averages together the contributions of all faces that are
used by a vertex, creating drawVert->normal
=====================
*/
void crDrawGeometry::CreateVertexNormals( void )
{
	if( silIndexes == nullptr )
		CreateSilIndexes();
	
	idTempArray< idVec3 > vertexNormals( numVerts );
	vertexNormals.Zero();
	
	assert( silIndexes != nullptr );
	for( int i = 0; i < numIndexes; i += 3 )
	{
		const int i0 = silIndexes[i + 0];
		const int i1 = silIndexes[i + 1];
		const int i2 = silIndexes[i + 2];
		
		const idDrawVert& v0 = verts[i0];
		const idDrawVert& v1 = verts[i1];
		const idDrawVert& v2 = verts[i2];
		
		const idPlane plane( v0.xyz, v1.xyz, v2.xyz );
		
		vertexNormals[i0] += plane.Normal();
		vertexNormals[i1] += plane.Normal();
		vertexNormals[i2] += plane.Normal();
	}
	
	// replicate from silIndexes to all indexes
	for( int i = 0; i < numIndexes; i++ )
	{
		vertexNormals[indexes[i]] = vertexNormals[silIndexes[i]];
	}
	
	// normalize
	for( int i = 0; i < numVerts; i++ )
	{
		vertexNormals[i].Normalize();
	}
	
	// compress the normals
	for( int i = 0; i < numVerts; i++ )
	{
		verts[i].SetNormal( vertexNormals[i] );
	}
}



/*
=====================
crDrawGeometry::DeriveFacePlanes

Writes the facePlanes values, overwriting existing ones if present
=====================
*/
void crDrawGeometry::DeriveFacePlanes( void ) 
{
	idPlane* planes = nullptr;

	if ( !facePlanes )
		AllocStaticTriSurfPlanes( numIndexes );

	planes = facePlanes;

#if 0
	//TODO: Port SIMD Processor from DHEWM3.

	SIMDProcessor->DeriveTriPlanes(planes, verts, numVerts, indexes, numIndexes);

#else

	for ( int i = 0; i < numIndexes; i += 3, planes++) 
	{
		int		i1 = 0, i2 = 0, i3 = 0;
		idVec3	d1, d2, normal;
		idVec3 *v1 = nullptr, *v2 = nullptr, *v3 = nullptr;

		i1 = indexes[i + 0];
		i2 = indexes[i + 1];
		i3 = indexes[i + 2];

		v1 = &verts[i1].xyz;
		v2 = &verts[i2].xyz;
		v3 = &verts[i3].xyz;

		d1[0] = v2->x - v1->x;
		d1[1] = v2->y - v1->y;
		d1[2] = v2->z - v1->z;

		d2[0] = v3->x - v1->x;
		d2[1] = v3->y - v1->y;
		d2[2] = v3->z - v1->z;

		normal[0] = d2.y * d1.z - d2.z * d1.y;
		normal[1] = d2.z * d1.x - d2.x * d1.z;
		normal[2] = d2.x * d1.y - d2.y * d1.x;

		float sqrLength, invLength;

		sqrLength = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
		invLength = idMath::RSqrt(sqrLength);

		(*planes)[0] = normal[0] * invLength;
		(*planes)[1] = normal[1] * invLength;
		(*planes)[2] = normal[2] * invLength;

		planes->FitThroughPoint(*v1);
	}

#endif

	facePlanesCalculated = true;
}


/*
=================
crDrawGeometry::CleanupTriangles

FIXME: allow createFlat and createSmooth normals, as well as explicit
=================
*/
void crDrawGeometry::CleanupTriangles( const bool createNormals, const bool identifySilEdges, const bool useUnsmoothedTangents )
{
	RangeCheckIndexes();
	
	CreateSilIndexes();
	
//	RemoveDuplicatedTriangles();	// this may remove valid overlapped transparent triangles

	RemoveDegenerateTriangles();
	
	TestDegenerateTextureSpace();
	
//	RemoveUnusedVerts();

	if( identifySilEdges )
		IdentifySilEdges( true );	// assume it is non-deformable, and omit coplanar edges
	
	// bust vertexes that share a mirrored edge into separate vertexes
	DuplicateMirroredVertexes();
	
	CreateDupVerts();
	
	BoundTriSurf();
	
	if( useUnsmoothedTangents )
	{
		BuildDominantTris();
		DeriveTangents();
	}
	else if( !createNormals )
		DeriveTangentsWithoutNormals();
	else
		DeriveTangents();
}

/*
=================
R_ReverseTriangles

Lit two sided surfaces need to have the triangles actually duplicated,
they can't just turn on two sided lighting, because the normal and tangents
are wrong on the other sides.

This should be called before R_CleanupTriangles
=================
*/
void crDrawGeometry::ReverseTriangles( void )
{
	int i = 0;
	
	// flip the normal on each vertex
	// If the surface is going to have generated normals, this won't matter,
	// but if it has explicit normals, this will keep it on the correct side
	for( i = 0; i < numVerts; i++ )
	{
		verts[i].SetNormal( vec3_origin - verts[i].GetNormal() );
	}
	
	// flip the index order to make them back sided
	for( i = 0; i < numIndexes; i += 3 )
	{
		triIndex_t	temp;
		
		temp = indexes[ i + 0 ];
		indexes[ i + 0 ] = indexes[ i + 1 ];
		indexes[ i + 1 ] = temp;
	}
}


/*
===================
crDrawGeometry::InitDrawSurfFromTri
===================
*/
void crDrawGeometry::InitDrawSurfFromTri( drawSurf_t* ds )
{
	if( numIndexes == 0 )
	{
		ds->numIndexes = 0;
		return;
	}
	
	// copy verts and indexes to this frame's hardware memory if they aren't already there
	//
	// deformed surfaces will not have any vertices but the ambient cache will have already
	// been created for them.
	if( ( verts == nullptr ) && !referencedIndexes )
		ambientCache = 0; // pre-generated shadow models will not have any verts, just shadowVerts

	else if( !vertexCache.CacheIsCurrent( ambientCache ) )
		ambientCache = vertexCache.AllocVertex( verts, ALIGN( numVerts * sizeof( verts[0] ), VERTEX_CACHE_ALIGN ) );

	if( !vertexCache.CacheIsCurrent( indexCache ) )
		indexCache = vertexCache.AllocIndex( indexes, ALIGN( numIndexes * sizeof( indexes[0] ), INDEX_CACHE_ALIGN ) );
	
	ds->numIndexes = numIndexes;
	ds->ambientCache = ambientCache;
	ds->indexCache = indexCache;
	ds->shadowCache = shadowCache;
	ds->jointCache = 0;
}

/*
===================
crDrawGeometry::CreateStaticBuffersForTri

For static surfaces, the indexes, ambient, and shadow buffers can be pre-created at load
time, rather than being re-created each frame in the frame temporary buffers.
===================
*/
void crDrawGeometry::CreateStaticBuffersForTri( void )
{
	indexCache = 0;
	ambientCache = 0;
	shadowCache = 0;
	
	// index cache
	if( indexes != nullptr )
		indexCache = vertexCache.AllocStaticIndex( indexes, ALIGN( numIndexes * sizeof( triIndex_t ), INDEX_CACHE_ALIGN ) );
	
	// vertex cache
	if( verts != nullptr )
		ambientCache = vertexCache.AllocStaticVertex( verts, ALIGN( numVerts * sizeof( idDrawVert ), VERTEX_CACHE_ALIGN ) );
	
	// shadow cache
	if( preLightShadowVertexes != nullptr )
	{
		// this should only be true for the _prelight<NAME> pre-calculated shadow volumes
		assert( verts == nullptr );	// pre-light shadow volume surfaces don't have ambient vertices
		const int shadowSize = ALIGN( numVerts * 2 * sizeof( idShadowVert ), VERTEX_CACHE_ALIGN );
		shadowCache = vertexCache.AllocStaticVertex( preLightShadowVertexes, shadowSize );
	}
	else if( verts != nullptr )
	{
		// the shadowVerts for normal models include all the xyz values duplicated
		// for a W of 1 (near cap) and a W of 0 (end cap, projected to infinity)
		const int shadowSize = ALIGN( numVerts * 2 * sizeof( idShadowVert ), VERTEX_CACHE_ALIGN );
		if( staticShadowVertexes == nullptr )
		{
			staticShadowVertexes = ( idShadowVert* ) Mem_Alloc16( shadowSize, TAG_TEMP );
			idShadowVert::CreateShadowCache( staticShadowVertexes, verts, numVerts );
		}
		shadowCache = vertexCache.AllocStaticVertex( staticShadowVertexes, shadowSize );
		
#if !defined( KEEP_INTERACTION_CPU_DATA )
		Mem_Free( staticShadowVertexes );
		staticShadowVertexes = NULL;
#endif
	}
}

/*
=================
crDrawGeometry::MergeSurfaceList

Only deals with vertexes and indexes, not silhouettes, planes, etc.
Does NOT perform a cleanup triangles, so there may be duplicated verts in the result.
=================
*/
crDrawGeometry* crDrawGeometry::MergeSurfaceList( const crDrawGeometry** surfaces, const uint32_t numSurfaces )
{
	uint32_t					i = 0, j = 0;
	uint32_t					totalVerts = 0;
	uint32_t					totalIndexes= 0;
	crDrawGeometry*			newTri = nullptr;
	const crDrawGeometry*	tri = nullptr;
	
	totalVerts = 0;
	totalIndexes = 0;
	for( i = 0; i < numSurfaces; i++ )
	{
		totalVerts += surfaces[i]->numVerts;
		totalIndexes += surfaces[i]->numIndexes;
	}
	
	newTri = new crDrawGeometry();
	newTri->numVerts = totalVerts;
	newTri->numIndexes = totalIndexes;
	newTri->AllocStaticTriSurfVerts( newTri->numVerts );
	newTri->AllocStaticTriSurfIndexes( newTri->numIndexes );
	
	totalVerts = 0;
	totalIndexes = 0;
	for( i = 0; i < numSurfaces; i++ )
	{
		tri = surfaces[i];
		std::memcpy( newTri->verts + totalVerts, tri->Verts(), tri->NumVerts() * sizeof( *tri->Verts() ) );
		for( j = 0; j < tri->NumIndexes(); j++ )
		{
			newTri->indexes[ totalIndexes + j ] = totalVerts + tri->Indexes()[j];
		}
		totalVerts += tri->NumVerts();
		totalIndexes += tri->NumIndexes();
	}
	
	return newTri;
}

/*
=================
crDrawGeometry::MergeTriangles

Only deals with vertexes and indexes, not silhouettes, planes, etc.
Does NOT perform a cleanup triangles, so there may be duplicated verts in the result.
=================
*/
crDrawGeometry* crDrawGeometry::MergeTriangles( const crDrawGeometry* tri1, const crDrawGeometry* tri2 )
{
	const crDrawGeometry*	tris[2];
	
	tris[0] = tri1;
	tris[1] = tri2;
	
	return MergeSurfaceList( tris, 2 );
}

/*
=================
crDrawGeometry::CopyStaticTriSurf

This only duplicates the indexes and verts, not any of the derived data.
=================
*/
crDrawGeometry* crDrawGeometry::CopyStaticTriSurf( const crDrawGeometry* tri )
{
	crDrawGeometry*	newTri = new crDrawGeometry();
	newTri->AllocStaticTriSurfVerts( tri->NumVerts() );
	newTri->AllocStaticTriSurfIndexes( tri->NumIndexes() );
	newTri->NumVerts() = tri->NumVerts();
	newTri->NumIndexes() = tri->NumIndexes();
	std::memcpy( newTri->Verts(), tri->Verts(), tri->NumVerts() * sizeof( idDrawVert ) );
	std::memcpy( newTri->Indexes(), tri->Indexes(), tri->NumIndexes() * sizeof( triIndex_t ) );

	return newTri;
}

/*
=================
crDrawGeometry::SetIndexCache
=================
*/
void crDrawGeometry::IndexCache( const vertCacheHandle_t in_indexCache, const triIndex_t* in_indexes )
{
	indexCache = in_indexCache;
	indexes = const_cast<triIndex_t*>( in_indexes );
}

/*
=================
crDrawGeometry::AmbientCache
=================
*/
void crDrawGeometry::AmbientCache( const vertCacheHandle_t in_cacheHandle, const idDrawVert *in_vertexes )
{
	ambientCache = in_cacheHandle;
	verts = const_cast<idDrawVert*>( in_vertexes );
}

/*
=================
crDrawGeometry::ShadowCache
=================
*/
void crDrawGeometry::ShadowCache( const vertCacheHandle_t in_cacheHandle )
{
	shadowCache = in_cacheHandle;
}

/*
=================
crDrawGeometry::ReferenceAmbientCache
=================
*/
void crDrawGeometry::ReferenceAmbientCache(const vertCacheHandle_t in_ambientCache, const idDrawVert *in_vertexes )
{
	ambientCache = in_ambientCache;
	verts = const_cast<idDrawVert*>( in_vertexes );
	referencedVerts = true;  
}

/*
=================
crDrawGeometry::ReferenceIndexCache
=================
*/
void crDrawGeometry::ReferenceIndexCache(const vertCacheHandle_t in_ambientCache, const triIndex_t* in_indexes )
{
	indexCache = in_ambientCache;
	indexes = const_cast<triIndex_t*>( in_indexes );
	referencedIndexes = true;  
}

/*
=================
R_AllocStaticTriSurfPlanes
=================
*/
void crDrawGeometry::AllocStaticTriSurfPlanes( const uint32_t numIndexes )
{
	if ( facePlanes != nullptr ) 
		Mem_Free( facePlanes );
	
	facePlanes = static_cast<idPlane*>( Mem_Alloc(numIndexes / 3 * sizeof(idPlane), TAG_TRI_PLANES) );
}

/*
=================
crDrawGeometry:: CreateSilRemap
=================
*/
int* crDrawGeometry::CreateSilRemap( void )
{
	int c_removed = 0, c_unique = 0;
	int	i = 0, j = 0, hashKey = 0;
	int* remap = nullptr;
	const idDrawVert *v1 = nullptr, *v2 = nullptr;
	
	remap = static_cast<int*>( R_ClearedStaticAlloc( numVerts * sizeof( int ) ) );
	
	if( !r_useSilRemap.GetBool() )
	{
		for( i = 0; i < numVerts; i++ )
		{
			remap[i] = i;
		}

		return remap;
	}
	
	idHashIndex hash( 1024, numVerts );
	
	c_removed = 0;
	c_unique = 0;
	for( i = 0; i < numVerts; i++ )
	{
		v1 = &verts[i];
		
		// see if there is an earlier vert that it can map to
		hashKey = hash.GenerateKey( v1->xyz );
		for( j = hash.First( hashKey ); j >= 0; j = hash.Next( j ) )
		{
			v2 = &verts[j];
			if( v2->xyz[0] == v1->xyz[0]
					&& v2->xyz[1] == v1->xyz[1]
					&& v2->xyz[2] == v1->xyz[2] )
			{
				c_removed++;
				remap[i] = j;
				break;
			}
		}

		if( j < 0 )
		{
			c_unique++;
			remap[i] = i;
			hash.Add( hashKey, i );
		}
	}
	
	return remap;
}

static void R_DefineEdge( const int v1, const int v2, const int planeNum, const int numPlanes, idList<silEdge_t>& silEdges, idHashIndex& silEdgeHash, int &c_duplicatedEdges, int &c_tripledEdges )
{
	int		i = 0, hashKey = 0;
	
	// check for degenerate edge
	if( v1 == v2 )
		return;

	hashKey = silEdgeHash.GenerateKey( v1, v2 );
	// search for a matching other side
	for( i = silEdgeHash.First( hashKey ); i >= 0 && i < MAX_SIL_EDGES; i = silEdgeHash.Next( i ) )
	{
		if( silEdges[i].v1 == v1 && silEdges[i].v2 == v2 )
		{
			c_duplicatedEdges++;

			// allow it to still create a new edge
			continue;
		}

		if( silEdges[i].v2 == v1 && silEdges[i].v1 == v2 )
		{
			if( silEdges[i].p2 != numPlanes )
			{
				c_tripledEdges++;

				// allow it to still create a new edge
				continue;
			}

			// this is a matching back side
			silEdges[i].p2 = planeNum;
			return;
		}
		
	}
	
	// define the new edge
	silEdgeHash.Add( hashKey, silEdges.Num() );
	
	silEdge_t silEdge;
	
	silEdge.p1 = planeNum;
	silEdge.p2 = numPlanes;
	silEdge.v1 = v1;
	silEdge.v2 = v2;
	
	silEdges.Append( silEdge );
}

/*
=================
SilEdgeSort
=================
*/
static int SilEdgeSort( const void* a, const void* b )
{
	if( ( ( silEdge_t* )a )->p1 < ( ( silEdge_t* )b )->p1 )
		return -1;
	
	if( ( ( silEdge_t* )a )->p1 > ( ( silEdge_t* )b )->p1 )
		return 1;

	if( ( ( silEdge_t* )a )->p2 < ( ( silEdge_t* )b )->p2 )
		return -1;
	
	if( ( ( silEdge_t* )a )->p2 > ( ( silEdge_t* )b )->p2 )
		return 1;

	return 0;
}

/*
=================
crDrawGeometry::IdentifySilEdges

If the surface will not deform, coplanar edges (polygon interiors)
can never create silhouette plains, and can be omited
=================
*/
void crDrawGeometry::IdentifySilEdges( bool omitCoplanarEdges )
{
	static int c_coplanarSilEdges = 0;
	static int c_totalSilEdges = 0;
	int		i = 0;
	int		shared = 0, single = 0;
	
	omitCoplanarEdges = false;	// optimization doesn't work for some reason
	
	static const int SILEDGE_HASH_SIZE		= 1024;
	
	const int numTris = numIndexes / 3;
	
	idList<silEdge_t>	silEdges( MAX_SIL_EDGES );
	idHashIndex	silEdgeHash( SILEDGE_HASH_SIZE, MAX_SIL_EDGES );
	int			numPlanes = numTris;
	
	
	silEdgeHash.Clear();
	
	int c_duplicatedEdges = 0;
	int c_tripledEdges = 0;
	
	for( i = 0; i < numTris; i++ )
	{
		int		i1, i2, i3;
		
		i1 = silIndexes[ i * 3 + 0 ];
		i2 = silIndexes[ i * 3 + 1 ];
		i3 = silIndexes[ i * 3 + 2 ];
		
		// create the edges
		R_DefineEdge( i1, i2, i, numPlanes, silEdges, silEdgeHash, c_duplicatedEdges, c_tripledEdges );
		R_DefineEdge( i2, i3, i, numPlanes, silEdges, silEdgeHash, c_duplicatedEdges, c_tripledEdges );
		R_DefineEdge( i3, i1, i, numPlanes, silEdges, silEdgeHash, c_duplicatedEdges, c_tripledEdges );
	}
	
	if( c_duplicatedEdges || c_tripledEdges )
	{
		common->DWarning( "%i duplicated edge directions, %i tripled edges", c_duplicatedEdges, c_tripledEdges );
	}
	
	// if we know that the vertexes aren't going
	// to deform, we can remove interior triangulation edges
	// on otherwise planar polygons.
	// I earlier believed that I could also remove concave
	// edges, because they are never silhouettes in the conventional sense,
	// but they are still needed to balance out all the true sil edges
	// for the shadow algorithm to function
	int c_coplanarCulled;
	
	c_coplanarCulled = 0;
	if( omitCoplanarEdges )
	{
		for( i = 0; i < silEdges.Num(); i++ )
		{
			int			i1, i2, i3;
			idPlane		plane;
			int			base;
			int			j;
			float		d;
			
			if( silEdges[i].p2 == numPlanes )  	// the fake dangling edge
			{
				continue;
			}
			
			base = silEdges[i].p1 * 3;
			i1 = silIndexes[ base + 0 ];
			i2 = silIndexes[ base + 1 ];
			i3 = silIndexes[ base + 2 ];
			
			plane.FromPoints( verts[i1].xyz, verts[i2].xyz, verts[i3].xyz );
			
			// check to see if points of second triangle are not coplanar
			base = silEdges[i].p2 * 3;
			for( j = 0; j < 3; j++ )
			{
				i1 = silIndexes[ base + j ];
				d = plane.Distance( verts[i1].xyz );
				if( d != 0 )  		// even a small epsilon causes problems
					break;
			}
			
			if( j == 3 )
			{
				// we can cull this sil edge
				std::memmove( &silEdges[i], &silEdges[i + 1], ( silEdges.Num() - i - 1 ) * sizeof( silEdges[i] ) );
				c_coplanarCulled++;
				silEdges.SetNum( silEdges.Num() - 1 );
				i--;
			}
		}
		if( c_coplanarCulled )
		{
			c_coplanarSilEdges += c_coplanarCulled;
//			common->Printf( "%i of %i sil edges coplanar culled\n", c_coplanarCulled,
//				c_coplanarCulled + numSilEdges );
		}
	}
	c_totalSilEdges += silEdges.Num();
	
	// sort the sil edges based on plane number
	qsort( silEdges.Ptr(), silEdges.Num(), sizeof( silEdges[0] ), SilEdgeSort );
	
	// count up the distribution.
	// a perfectly built model should only have shared
	// edges, but most models will have some interpenetration
	// and dangling edges
	shared = 0;
	single = 0;
	for( i = 0; i < silEdges.Num(); i++ )
	{
		if( silEdges[i].p2 == numPlanes )
			single++;
		else
			shared++;
	}
	
	if( !single )
		perfectHull = true;
	else
		perfectHull = false;
	
	numSilEdges = silEdges.Num();
	AllocStaticTriSurfSilEdges( silEdges.Num() );
	std::memcpy( silEdges.Ptr(), silEdges.Ptr(), silEdges.Num() * sizeof( silEdges[0] ) );
}

/*
=================
crDrawGeometry::TestDegenerateTextureSpace
=================
*/
void crDrawGeometry::TestDegenerateTextureSpace( void )
{
	int c_degenerate = 0;
	int i = 0;
	
	// check for triangles with a degenerate texture space
	c_degenerate = 0;
	for( i = 0; i < numIndexes; i += 3 )
	{
		const idDrawVert& a = verts[indexes[i + 0]];
		const idDrawVert& b = verts[indexes[i + 1]];
		const idDrawVert& c = verts[indexes[i + 2]];
		
		if( a.st == b.st || b.st == c.st || c.st == a.st )
		{
			c_degenerate++;
		}
	}
	
	if( c_degenerate )
	{
//		common->Printf( "%d triangles with a degenerate texture space\n", c_degenerate );
	}
}

/*
===================
crDrawGeometry::DuplicateMirroredVertexes

Modifies the surface to bust apart any verts that are shared by both positive and
negative texture polarities, so tangent space smoothing at the vertex doesn't
degenerate.

This will create some identical vertexes (which will eventually get different tangent
vectors), so never optimize the resulting mesh, or it will get the mirrored edges back.

Reallocates verts and changes indexes in place
Silindexes are unchanged by this.

sets mirroredVerts and mirroredVerts[]
===================
*/
void crDrawGeometry::DuplicateMirroredVertexes( void )
{
	tangentVert_t*	vert = nullptr;
	uint32_t 		i = 0, j = 0;
	uint32_t		totalVerts = 0;
	uint32_t		numMirror = 0;
	
	idTempArray<tangentVert_t> tverts( numVerts );
	tverts.Zero();
	
	// determine texture polarity of each surface
	
	// mark each vert with the polarities it uses
	for( i = 0; i < numIndexes; i += 3 )
	{
		int	polarity = FaceNegativePolarity( i );
		for( j = 0; j < 3; j++ )
		{
			tverts[indexes[i + j]].polarityUsed[ polarity ] = true;
		}
	}
	
	// now create new vertex indices as needed
	totalVerts = numVerts;
	for( i = 0; i < numVerts; i++ )
	{
		vert = &tverts[i];
		if( vert->polarityUsed[0] && vert->polarityUsed[1] )
		{
			vert->negativeRemap = totalVerts;
			totalVerts++;
		}
	}
	
	numMirroredVerts = totalVerts - numVerts;
	
	if( numMirroredVerts == 0 )
	{
		mirroredVerts = nullptr;
		return;
	}
	
	// now create the new list
	AllocStaticTriSurfMirroredVerts( numMirroredVerts );
	ResizeStaticTriSurfVerts( totalVerts );
	
	// create the duplicates
	numMirror = 0;
	for( i = 0; i < numVerts; i++ )
	{
		j = tverts[i].negativeRemap;
		if( j )
		{
			verts[j] = verts[i];
			mirroredVerts[numMirror] = i;
			numMirror++;
		}
	}
	numVerts = totalVerts;
	
	// change the indexes
	for( i = 0; i < numIndexes; i++ )
	{
		if( tverts[indexes[i]].negativeRemap && FaceNegativePolarity( 3 * ( i / 3 ) ) )
		{
			indexes[i] = tverts[indexes[i]].negativeRemap;
		}
	}
}

/*
===============
crDrawGeometry::FaceNegativePolarity

Returns true if the texture polarity of the face is negative, false if it is positive or zero
===============
*/
bool crDrawGeometry::FaceNegativePolarity( const uint32_t firstIndex )
{
	const idDrawVert* a = verts + indexes[firstIndex + 0];
	const idDrawVert* b = verts + indexes[firstIndex + 1];
	const idDrawVert* c = verts + indexes[firstIndex + 2];
	
	const idVec2 aST = a->GetTexCoord();
	const idVec2 bST = b->GetTexCoord();
	const idVec2 cST = c->GetTexCoord();
	
	float d0[5];
	d0[3] = bST[0] - aST[0];
	d0[4] = bST[1] - aST[1];
	
	float d1[5];
	d1[3] = cST[0] - aST[0];
	d1[4] = cST[1] - aST[1];
	
	const float area = d0[3] * d1[4] - d0[4] * d1[3];
	if( area >= 0 )
		return false;
	
	return true;
}

/*
=====================
crDrawGeometry::CreateDupVerts
=====================
*/
void crDrawGeometry::CreateDupVerts( void )
{
	int i;
	
	idTempArray<int> remap( numVerts );
	
	// initialize vertex remap in case there are unused verts
	for( i = 0; i < numVerts; i++ )
	{
		remap[i] = i;
	}
	
	// set the remap based on how the silhouette indexes are remapped
	for( i = 0; i < numIndexes; i++ )
	{
		remap[indexes[i]] = silIndexes[i];
	}
	
	// create duplicate vertex index based on the vertex remap
	idTempArray<int> tempDupVerts( numVerts * 2 );
	numDupVerts = 0;
	for( i = 0; i < numVerts; i++ )
	{
		if( remap[i] != i )
		{
			tempDupVerts[numDupVerts * 2 + 0] = i;
			tempDupVerts[numDupVerts * 2 + 1] = remap[i];
			numDupVerts++;
		}
	}
	
	AllocStaticTriSurfDupVerts( numDupVerts );
	memcpy( dupVerts, tempDupVerts.Ptr(), numDupVerts * 2 * sizeof( dupVerts[0] ) );
}

static int IndexSort( const void* a, const void* b )
{
	if( ( ( indexSort_t* )a )->vertexNum < ( ( indexSort_t* )b )->vertexNum )
		return -1;
	
	if( ( ( indexSort_t* )a )->vertexNum > ( ( indexSort_t* )b )->vertexNum )
		return 1;
	
	return 0;
}


/*
===================
crDrawGeometry::BuildDominantTris

Find the largest triangle that uses each vertex
===================
*/
void crDrawGeometry::BuildDominantTris( void )
{
	int i = 0, j = 0;
	const int numIndexes = numIndexes;
	dominantTri_t* dt = nullptr;
	indexSort_t* ind = static_cast<indexSort_t*>( R_StaticAlloc( numIndexes * sizeof( indexSort_t ) ) );
	if( ind == nullptr )
	{
		idLib::Error( "Couldn't allocate index sort array" );
		return;
	}
	
	for( i = 0; i < numIndexes; i++ )
	{
		ind[i].vertexNum = indexes[i];
		ind[i].faceNum = i / 3;
	}
	std::qsort( ind, numIndexes, sizeof( indexSort_t ), IndexSort );
	
	AllocStaticTriSurfDominantTris( numVerts );
	dt = dominantTris;
	std::memset( dt, 0, numVerts * sizeof( dt[0] ) );
	
	for( i = 0; i < numIndexes; i += j )
	{
		float	maxArea = 0;
//#pragma warning( disable: 6385 ) // This is simply to get pass a false defect for /analyze -- if you can figure out a better way, please let Shawn know...
		int		vertNum = ind[i].vertexNum;
//#pragma warning( default: 6385 )
		for( j = 0; i + j < numIndexes && ind[i + j].vertexNum == vertNum; j++ )
		{
			float		d0[5], d1[5];
			idDrawVert*	a, *b, *c;
			idVec3		normal, tangent, bitangent;
			
			int	i1 = indexes[ind[i + j].faceNum * 3 + 0];
			int	i2 = indexes[ind[i + j].faceNum * 3 + 1];
			int	i3 = indexes[ind[i + j].faceNum * 3 + 2];
			
			a = verts + i1;
			b = verts + i2;
			c = verts + i3;
			
			const idVec2 aST = a->GetTexCoord();
			const idVec2 bST = b->GetTexCoord();
			const idVec2 cST = c->GetTexCoord();
			
			d0[0] = b->xyz[0] - a->xyz[0];
			d0[1] = b->xyz[1] - a->xyz[1];
			d0[2] = b->xyz[2] - a->xyz[2];
			d0[3] = bST[0] - aST[0];
			d0[4] = bST[1] - aST[1];
			
			d1[0] = c->xyz[0] - a->xyz[0];
			d1[1] = c->xyz[1] - a->xyz[1];
			d1[2] = c->xyz[2] - a->xyz[2];
			d1[3] = cST[0] - aST[0];
			d1[4] = cST[1] - aST[1];
			
			normal[0] = ( d1[1] * d0[2] - d1[2] * d0[1] );
			normal[1] = ( d1[2] * d0[0] - d1[0] * d0[2] );
			normal[2] = ( d1[0] * d0[1] - d1[1] * d0[0] );
			
			float area = normal.Length();
			
			// if this is smaller than what we already have, skip it
			if( area < maxArea )
			{
				continue;
			}
			maxArea = area;
			
			if( i1 == vertNum )
			{
				dt[vertNum].v2 = i2;
				dt[vertNum].v3 = i3;
			}
			else if( i2 == vertNum )
			{
				dt[vertNum].v2 = i3;
				dt[vertNum].v3 = i1;
			}
			else
			{
				dt[vertNum].v2 = i1;
				dt[vertNum].v3 = i2;
			}
			
			float	len = area;
			if( len < 0.001f )
				len = 0.001f;
			
			dt[vertNum].normalizationScale[2] = 1.0f / len;		// normal
			
			// texture area
			area = d0[3] * d1[4] - d0[4] * d1[3];
			
			tangent[0] = ( d0[0] * d1[4] - d0[4] * d1[0] );
			tangent[1] = ( d0[1] * d1[4] - d0[4] * d1[1] );
			tangent[2] = ( d0[2] * d1[4] - d0[4] * d1[2] );
			len = tangent.Length();
			if( len < 0.001f )
			{
				len = 0.001f;
			}
			dt[vertNum].normalizationScale[0] = ( area > 0 ? 1 : -1 ) / len;	// tangents[0]
			
			bitangent[0] = ( d0[3] * d1[0] - d0[0] * d1[3] );
			bitangent[1] = ( d0[3] * d1[1] - d0[1] * d1[3] );
			bitangent[2] = ( d0[3] * d1[2] - d0[2] * d1[3] );
			len = bitangent.Length();
			if( len < 0.001f )
			{
				len = 0.001f;
			}
#ifdef DERIVE_UNSMOOTHED_BITANGENT
			dt[vertNum].normalizationScale[1] = ( area > 0 ? 1 : -1 );
#else
			dt[vertNum].normalizationScale[1] = ( area > 0 ? 1 : -1 ) / len;	// tangents[1]
#endif
		}
	}
	
	R_StaticFree( ind );
}


/*
==================
crDrawGeometry::DeriveTangents

This is called once for static surfaces, and every frame for deforming surfaces

Builds tangents, normals, and face planes
==================
*/
void crDrawGeometry::DeriveTangents( void )
{
	if( tangentsCalculated )
		return;
	
	// TODO: lock or atomic increment for this
	tr.pc.c_tangentIndexes += numIndexes;
	
	if( dominantTris != nullptr )
		DeriveUnsmoothedNormalsAndTangents();
	else
		DeriveNormalsAndTangents();
	
	tangentsCalculated = true;
}

/*
============
crDrawGeometry::DeriveUnsmoothedNormalsAndTangents
============
*/
void crDrawGeometry::DeriveUnsmoothedNormalsAndTangents( void )
{
	for( int i = 0; i < numVerts; i++ )
	{
		float d0, d1, d2, d3, d4;
		float d5, d6, d7, d8, d9;
		float s0, s1, s2;
		float n0, n1, n2;
		float t0, t1, t2;
		float t3, t4, t5;
		
		const dominantTri_t& dt = dominantTris[i];
		
		idDrawVert* a = verts + i;
		idDrawVert* b = verts + dt.v2;
		idDrawVert* c = verts + dt.v3;
		
		const idVec2 aST = a->GetTexCoord();
		const idVec2 bST = b->GetTexCoord();
		const idVec2 cST = c->GetTexCoord();
		
		d0 = b->xyz[0] - a->xyz[0];
		d1 = b->xyz[1] - a->xyz[1];
		d2 = b->xyz[2] - a->xyz[2];
		d3 = bST[0] - aST[0];
		d4 = bST[1] - aST[1];
		
		d5 = c->xyz[0] - a->xyz[0];
		d6 = c->xyz[1] - a->xyz[1];
		d7 = c->xyz[2] - a->xyz[2];
		d8 = cST[0] - aST[0];
		d9 = cST[1] - aST[1];
		
		s0 = dt.normalizationScale[0];
		s1 = dt.normalizationScale[1];
		s2 = dt.normalizationScale[2];
		
		n0 = s2 * ( d6 * d2 - d7 * d1 );
		n1 = s2 * ( d7 * d0 - d5 * d2 );
		n2 = s2 * ( d5 * d1 - d6 * d0 );
		
		t0 = s0 * ( d0 * d9 - d4 * d5 );
		t1 = s0 * ( d1 * d9 - d4 * d6 );
		t2 = s0 * ( d2 * d9 - d4 * d7 );
		
#ifndef DERIVE_UNSMOOTHED_BITANGENT
		t3 = s1 * ( d3 * d5 - d0 * d8 );
		t4 = s1 * ( d3 * d6 - d1 * d8 );
		t5 = s1 * ( d3 * d7 - d2 * d8 );
#else
		t3 = s1 * ( n2 * t1 - n1 * t2 );
		t4 = s1 * ( n0 * t2 - n2 * t0 );
		t5 = s1 * ( n1 * t0 - n0 * t1 );
#endif
		
		a->SetNormal( n0, n1, n2 );
		a->SetTangent( t0, t1, t2 );
		a->SetBiTangent( t3, t4, t5 );
	}
}


/*
============
crDrawGeometry::DeriveNormalsAndTangents

Derives the normal and orthogonal tangent vectors for the triangle vertices.
For each vertex the normal and tangent vectors are derived from all triangles
using the vertex which results in smooth tangents across the mesh.
============
*/
void crDrawGeometry::DeriveNormalsAndTangents( void )
{
	idTempArray< idVec3 > vertexNormals( numVerts );
	idTempArray< idVec3 > vertexTangents( numVerts );
	idTempArray< idVec3 > vertexBitangents( numVerts );
	
	vertexNormals.Zero();
	vertexTangents.Zero();
	vertexBitangents.Zero();
	
	for( int i = 0; i < numIndexes; i += 3 )
	{
		const int v0 = indexes[i + 0];
		const int v1 = indexes[i + 1];
		const int v2 = indexes[i + 2];
		
		const idDrawVert* a = verts + v0;
		const idDrawVert* b = verts + v1;
		const idDrawVert* c = verts + v2;
		
		const idVec2 aST = a->GetTexCoord();
		const idVec2 bST = b->GetTexCoord();
		const idVec2 cST = c->GetTexCoord();
		
		float d0[5];
		d0[0] = b->xyz[0] - a->xyz[0];
		d0[1] = b->xyz[1] - a->xyz[1];
		d0[2] = b->xyz[2] - a->xyz[2];
		d0[3] = bST[0] - aST[0];
		d0[4] = bST[1] - aST[1];
		
		float d1[5];
		d1[0] = c->xyz[0] - a->xyz[0];
		d1[1] = c->xyz[1] - a->xyz[1];
		d1[2] = c->xyz[2] - a->xyz[2];
		d1[3] = cST[0] - aST[0];
		d1[4] = cST[1] - aST[1];
		
		idVec3 normal;
		normal[0] = d1[1] * d0[2] - d1[2] * d0[1];
		normal[1] = d1[2] * d0[0] - d1[0] * d0[2];
		normal[2] = d1[0] * d0[1] - d1[1] * d0[0];
		
		const float f0 = idMath::InvSqrt( normal.x * normal.x + normal.y * normal.y + normal.z * normal.z );
		
		normal.x *= f0;
		normal.y *= f0;
		normal.z *= f0;
		
		// area sign bit
		const float area = d0[3] * d1[4] - d0[4] * d1[3];
		uint32_t signBit = ( *( uint32_t* )&area ) & ( 1 << 31 );
		
		idVec3 tangent;
		tangent[0] = d0[0] * d1[4] - d0[4] * d1[0];
		tangent[1] = d0[1] * d1[4] - d0[4] * d1[1];
		tangent[2] = d0[2] * d1[4] - d0[4] * d1[2];
		
		const float f1 = idMath::InvSqrt( tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z );
		*( uint32_t* )&f1 ^= signBit;
		
		tangent.x *= f1;
		tangent.y *= f1;
		tangent.z *= f1;
		
		idVec3 bitangent;
		bitangent[0] = d0[3] * d1[0] - d0[0] * d1[3];
		bitangent[1] = d0[3] * d1[1] - d0[1] * d1[3];
		bitangent[2] = d0[3] * d1[2] - d0[2] * d1[3];
		
		const float f2 = idMath::InvSqrt( bitangent.x * bitangent.x + bitangent.y * bitangent.y + bitangent.z * bitangent.z );
		*( uint32_t* )&f2 ^= signBit;
		
		bitangent.x *= f2;
		bitangent.y *= f2;
		bitangent.z *= f2;
		
		vertexNormals[v0] += normal;
		vertexTangents[v0] += tangent;
		vertexBitangents[v0] += bitangent;
		
		vertexNormals[v1] += normal;
		vertexTangents[v1] += tangent;
		vertexBitangents[v1] += bitangent;
		
		vertexNormals[v2] += normal;
		vertexTangents[v2] += tangent;
		vertexBitangents[v2] += bitangent;
	}
	
	// add the normal of a duplicated vertex to the normal of the first vertex with the same XYZ
	for( int i = 0; i < numDupVerts; i++ )
	{
		vertexNormals[dupVerts[i * 2 + 0]] += vertexNormals[dupVerts[i * 2 + 1]];
	}
	
	// copy vertex normals to duplicated vertices
	for( int i = 0; i < numDupVerts; i++ )
	{
		vertexNormals[dupVerts[i * 2 + 1]] = vertexNormals[dupVerts[i * 2 + 0]];
	}
	
	// Project the summed vectors onto the normal plane and normalize.
	// The tangent vectors will not necessarily be orthogonal to each
	// other, but they will be orthogonal to the surface normal.
	for( int i = 0; i < numVerts; i++ )
	{
		const float normalScale = idMath::InvSqrt( vertexNormals[i].x * vertexNormals[i].x + vertexNormals[i].y * vertexNormals[i].y + vertexNormals[i].z * vertexNormals[i].z );
		vertexNormals[i].x *= normalScale;
		vertexNormals[i].y *= normalScale;
		vertexNormals[i].z *= normalScale;
		
		vertexTangents[i] -= ( vertexTangents[i] * vertexNormals[i] ) * vertexNormals[i];
		vertexBitangents[i] -= ( vertexBitangents[i] * vertexNormals[i] ) * vertexNormals[i];
		
		const float tangentScale = idMath::InvSqrt( vertexTangents[i].x * vertexTangents[i].x + vertexTangents[i].y * vertexTangents[i].y + vertexTangents[i].z * vertexTangents[i].z );
		vertexTangents[i].x *= tangentScale;
		vertexTangents[i].y *= tangentScale;
		vertexTangents[i].z *= tangentScale;
		
		const float bitangentScale = idMath::InvSqrt( vertexBitangents[i].x * vertexBitangents[i].x + vertexBitangents[i].y * vertexBitangents[i].y + vertexBitangents[i].z * vertexBitangents[i].z );
		vertexBitangents[i].x *= bitangentScale;
		vertexBitangents[i].y *= bitangentScale;
		vertexBitangents[i].z *= bitangentScale;
	}
	
	// compress the normals and tangents
	for( int i = 0; i < numVerts; i++ )
	{
		verts[i].SetNormal( vertexNormals[i] );
		verts[i].SetTangent( vertexTangents[i] );
		verts[i].SetBiTangent( vertexBitangents[i] );
	}
}

/*
=================
crDrawGeometry::DeriveTangentsWithoutNormals

Build texture space tangents for bump mapping
If a surface is deformed, this must be recalculated

This assumes that any mirrored vertexes have already been duplicated, so
any shared vertexes will have the tangent spaces smoothed across.

Texture wrapping slightly complicates this, but as long as the normals
are shared, and the tangent vectors are projected onto the normals, the
separate vertexes should wind up with identical tangent spaces.

mirroring a normalmap WILL cause a slightly visible seam unless the normals
are completely flat around the edge's full bilerp support.

Vertexes which are smooth shaded must have their tangent vectors
in the same plane, which will allow a seamless
rendering as long as the normal map is even on both sides of the
seam.

A smooth shaded surface may have multiple tangent vectors at a vertex
due to texture seams or mirroring, but it should only have a single
normal vector.

Each triangle has a pair of tangent vectors in it's plane

Should we consider having vertexes point at shared tangent spaces
to save space or speed transforms?

this version only handles bilateral symetry
=================
*/
void crDrawGeometry::DeriveTangentsWithoutNormals( void )
{
	idTempArray< idVec3 > triangleTangents( numIndexes / 3 );
	idTempArray< idVec3 > triangleBitangents( numIndexes / 3 );
	
	//
	// calculate tangent vectors for each face in isolation
	//
	int c_positive = 0;
	int c_negative = 0;
	int c_textureDegenerateFaces = 0;
	for( int i = 0; i < numIndexes; i += 3 )
	{
		idVec3	temp;
		
		idDrawVert* a = verts + indexes[i + 0];
		idDrawVert* b = verts + indexes[i + 1];
		idDrawVert* c = verts + indexes[i + 2];
		
		const idVec2 aST = a->GetTexCoord();
		const idVec2 bST = b->GetTexCoord();
		const idVec2 cST = c->GetTexCoord();
		
		float d0[5];
		d0[0] = b->xyz[0] - a->xyz[0];
		d0[1] = b->xyz[1] - a->xyz[1];
		d0[2] = b->xyz[2] - a->xyz[2];
		d0[3] = bST[0] - aST[0];
		d0[4] = bST[1] - aST[1];
		
		float d1[5];
		d1[0] = c->xyz[0] - a->xyz[0];
		d1[1] = c->xyz[1] - a->xyz[1];
		d1[2] = c->xyz[2] - a->xyz[2];
		d1[3] = cST[0] - aST[0];
		d1[4] = cST[1] - aST[1];
		
		const float area = d0[3] * d1[4] - d0[4] * d1[3];
		if( fabs( area ) < 1e-20f )
		{
			triangleTangents[i / 3].Zero();
			triangleBitangents[i / 3].Zero();
			c_textureDegenerateFaces++;
			continue;
		}
		if( area > 0.0f )
			c_positive++;
		else
			c_negative++;
		
#ifdef USE_INVA
		float inva = ( area < 0.0f ) ? -1.0f : 1.0f;		// was = 1.0f / area;
		
		temp[0] = ( d0[0] * d1[4] - d0[4] * d1[0] ) * inva;
		temp[1] = ( d0[1] * d1[4] - d0[4] * d1[1] ) * inva;
		temp[2] = ( d0[2] * d1[4] - d0[4] * d1[2] ) * inva;
		temp.Normalize();
		triangleTangents[i / 3] = temp;
		
		temp[0] = ( d0[3] * d1[0] - d0[0] * d1[3] ) * inva;
		temp[1] = ( d0[3] * d1[1] - d0[1] * d1[3] ) * inva;
		temp[2] = ( d0[3] * d1[2] - d0[2] * d1[3] ) * inva;
		temp.Normalize();
		triangleBitangents[i / 3] = temp;
#else
		temp[0] = ( d0[0] * d1[4] - d0[4] * d1[0] );
		temp[1] = ( d0[1] * d1[4] - d0[4] * d1[1] );
		temp[2] = ( d0[2] * d1[4] - d0[4] * d1[2] );
		temp.Normalize();
		triangleTangents[i / 3] = temp;
		
		temp[0] = ( d0[3] * d1[0] - d0[0] * d1[3] );
		temp[1] = ( d0[3] * d1[1] - d0[1] * d1[3] );
		temp[2] = ( d0[3] * d1[2] - d0[2] * d1[3] );
		temp.Normalize();
		triangleBitangents[i / 3] = temp;
#endif
	}
	
	idTempArray< idVec3 > vertexTangents( numVerts );
	idTempArray< idVec3 > vertexBitangents( numVerts );
	
	// clear the tangents
	for( int i = 0; i < numVerts; ++i )
	{
		vertexTangents[i].Zero();
		vertexBitangents[i].Zero();
	}
	
	// sum up the neighbors
	for( int i = 0; i < numIndexes; i += 3 )
	{
		// for each vertex on this face
		for( int j = 0; j < 3; j++ )
		{
			vertexTangents[indexes[i + j]] += triangleTangents[i / 3];
			vertexBitangents[indexes[i + j]] += triangleBitangents[i / 3];
		}
	}
	
	// Project the summed vectors onto the normal plane and normalize.
	// The tangent vectors will not necessarily be orthogonal to each
	// other, but they will be orthogonal to the surface normal.
	for( int i = 0; i < numVerts; i++ )
	{
		idVec3 normal = verts[i].GetNormal();
		normal.Normalize();
		
		vertexTangents[i] -= ( vertexTangents[i] * normal ) * normal;
		vertexTangents[i].Normalize();
		
		vertexBitangents[i] -= ( vertexBitangents[i] * normal ) * normal;
		vertexBitangents[i].Normalize();
	}
	
	for( int i = 0; i < numVerts; i++ )
	{
		verts[i].SetTangent( vertexTangents[i] );
		verts[i].SetBiTangent( vertexBitangents[i] );
	}
	
	tangentsCalculated = true;
}