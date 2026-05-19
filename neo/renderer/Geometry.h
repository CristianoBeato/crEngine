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

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__



// our only drawing geometry type
class crDrawGeometry
{
public:
	crDrawGeometry( void );
    ~crDrawGeometry( void );

	void	Clear( void );

	void	AllocStaticTriSurfVerts( const uint32_t numVerts );
	void	AllocStaticTriSurfIndexes( const uint32_t numIndexes );
	void	AllocStaticTriSurfPreLightShadowVerts( const uint32_t numVerts );
	void	AllocStaticTriSurfSilIndexes( const uint32_t numIndexes );
	void	AllocStaticTriSurfDominantTris( const uint32_t numVerts );
	void	AllocStaticTriSurfSilEdges( const uint32_t numSilEdges );
	void	AllocStaticTriSurfMirroredVerts( const uint32_t numMirroredVerts );
	void	AllocStaticTriSurfDupVerts( const uint32_t numDupVerts );

	void	ResizeStaticTriSurfVerts( const uint32_t numVerts );
	void	ResizeStaticTriSurfIndexes( const uint32_t numIndexes );
	void	ReferenceStaticTriSurfVerts( const crDrawGeometry* reference );
	void	ReferenceStaticTriSurfIndexes( const crDrawGeometry* reference );

	void	FreeStaticTriSurf( void );
	void	FreeStaticTriSurfSilIndexes( void );
	void	FreeStaticTriSurfVerts( void );
	void	FreeStaticTriSurfVertexCaches( void );
	void	FreeDominantTris( void );

	void	CreateIndexCache( const triIndex_t* newIndexes );
	void	CreateAmbientCache( const idDrawVert* newVerts );
	void	CreateShadowCache( const idShadowVert* newVerts );
	
	void	FreeIndexCache( void );
	void	FreeAmbientCache( void );
	void	FreeShadowCache( void );
	
	/// @brief For memory profiling
	size_t	TriSurfMemory( void ) const;

	/// @brief 
	void	BoundTriSurf( void );
	
	/// @brief SilIndexes must have already been calculated, silIndexes are used instead of indexes, because duplicated
	/// triangles could have different texture coordinates.
	void	RemoveDuplicatedTriangles( void );
	
	/// @brief Uniquing vertexes only on xyz before creating sil edges reduces the edge count by about 20% on Q3 models
	void	CreateSilIndexes( void );
	
	/// @brief SilIndexes must have already been calculated
	void	RemoveDegenerateTriangles( void );
	
	/// @brief
	void	RemoveUnusedVerts( void );
	
	/// @brief Check for syntactically incorrect indexes, like out of range values. Does not check for semantics, like degenerate triangles.
	/// No vertexes is acceptable if no indexes. No indexes is acceptable. More vertexes than are referenced by indexes are acceptable.
	void	RangeCheckIndexes( void );

	/// @brief  Averages together the contributions of all faces that are used by a vertex, creating drawVert->normal
	void	CreateVertexNormals( void );		// also called by dmap
	
	/// @brief Writes the facePlanes values, overwriting existing ones if present 
	void	DeriveFacePlanes( void );		// also called by renderbump

	/// @brief
	void	CleanupTriangles( const bool createNormals, const bool identifySilEdges, const bool useUnsmoothedTangents );
	
	/// @brief
	void	ReverseTriangles( void );

	/// @brief if the deformed verts have significant enough texture coordinate changes to reverse the texture
	/// polarity of a triangle, the tangents will be incorrect
	void	DeriveTangents( void );

	/// @brief copy data from a front-end crDrawGeometry to a back-end drawSurf_t
	void	InitDrawSurfFromTri( struct drawSurf_t* ds );

	/// @brief For static surfaces, the indexes, ambient, and shadow buffers can be pre-created at load
	/// time, rather than being re-created each frame in the frame temporary buffers.
	void	CreateStaticBuffersForTri( void );

	// Only deals with vertexes and indexes, not silhouettes, planes, etc.
	// Does NOT perform a cleanup triangles, so there may be duplicated verts in the result.
	static crDrawGeometry* MergeSurfaceList( const crDrawGeometry** surfaces, const uint32_t numSurfaces );
	static crDrawGeometry* MergeTriangles( const crDrawGeometry* tri1, const crDrawGeometry* tri2 );

	/// @brief This only duplicates the indexes and verts, not any of the derived data.
	static crDrawGeometry* CopyStaticTriSurf( const crDrawGeometry* tri );

	void	IndexCache( const vertCacheHandle_t in_cacheHandle, const triIndex_t* in_indexes );
	void 	AmbientCache( const vertCacheHandle_t in_cacheHandle, const idDrawVert* in_vertexes );
	void	ShadowCache( const vertCacheHandle_t in_cacheHandle );

	void	ReferenceAmbientCache( const vertCacheHandle_t in_ambientCache, const idDrawVert* in_vertexes );
	void	ReferenceIndexCache( const vertCacheHandle_t in_ambientCache, const triIndex_t* in_indexes  );

	// TODO: set ReferencedVerts
	// TODO: set ReferencedIndexes
	ID_INLINE void	SetReferencedVerts( const bool in_referenced ) { referencedVerts = in_referenced; }
	ID_INLINE void	SetReferencedIndexes( const bool in_referenced ) { referencedIndexes = in_referenced; }

	bool						&FacePlanesCalculated( void ) { return facePlanesCalculated; }
	bool						&GenerateNormals( void ) { return generateNormals; }
	bool						&TangentsCalculated( void ) { return tangentsCalculated; }
	bool						&PerfectHull( void ) { return perfectHull; }

	// TODO: Set num verts and indexes throug  AllocStaticTriSurfVerts and AllocStaticTriSurfIndexes
	uint32_t					&NumVerts( void ) { return numVerts;}
	uint32_t					&NumIndexes( void ) { return numIndexes; }	
	uint32_t					&NumMirroredVerts( void ) { return numMirroredVerts; }
	uint32_t					&NumSilEdges( void ) { return numSilEdges; }
	uint32_t					&NumDupVerts( void ) { return numDupVerts; }
	uint32_t					&NumShadowIndexesNoCaps( void ) { return numShadowIndexesNoCaps; }
	uint32_t					&NumShadowIndexesNoFrontCaps( void ) { return numShadowIndexesNoFrontCaps; }
	int							&ShadowCapPlaneBits( void ) { return shadowCapPlaneBits; }
//	vertCacheHandle_t			&IndexCache( void ) { return indexCache; };				
//	vertCacheHandle_t			&AmbientCache( void ) { return ambientCache; };			
//	vertCacheHandle_t			&ShadowCache( void ) { return shadowCache; };			
	idBounds 					&Bounds( void ) { return bounds; };
    idDrawVert* 				&Verts( void ) { return verts;};
	triIndex_t* 				&Indexes( void ) { return indexes; }
	triIndex_t* 				&SilIndexes( void ) { return silIndexes; }
	silEdge_t* 					&SilEdges( void ) { return silEdges; }
	int* 						&MirroredVerts( void ) { return mirroredVerts; }
	int* 						&DupVerts( void ) { return dupVerts; }
	dominantTri_t* 				&DominantTris( void ) { return dominantTris; }
	idShadowVert* 				&PreLightShadowVertexes( void ) { return preLightShadowVertexes; };
	idShadowVert* 				&StaticShadowVertexes( void ) { return staticShadowVertexes; }	
	crDrawGeometry*				&AmbientSurface( void ){ return ambientSurface; }	
	crDrawGeometry*				&NextDeferredFree( void ){ return nextDeferredFree; }
	idRenderModelStatic*		&StaticModelWithJoints( void ) { return staticModelWithJoints; }
	idPlane* 					&FacePlanes( void ) { return facePlanes; }

	ID_INLINE const bool		FacePlanesCalculated( void ) const { return facePlanesCalculated; }
	ID_INLINE const bool		ReferencedVerts( void ) const { return referencedVerts; }
	ID_INLINE const bool		ReferencedIndexes( void ) const { return referencedIndexes; }
	ID_INLINE const uint32_t	NumVerts( void ) const { return numVerts;}
	ID_INLINE const uint32_t	NumIndexes( void ) const { return numIndexes; }
	ID_INLINE const uint32_t	NumSilEdges( void ) const { return numSilEdges; }
	ID_INLINE const uint32_t	NumShadowIndexesNoCaps( void ) const { return numShadowIndexesNoCaps; }
	ID_INLINE const uint32_t	NumShadowIndexesNoFrontCaps( void ) const { return numShadowIndexesNoFrontCaps; }
	ID_INLINE const int			ShadowCapPlaneBits( void ) const { return shadowCapPlaneBits; } 
	ID_INLINE const idBounds	Bounds( void ) const { return bounds; };
	ID_INLINE const idDrawVert*	Verts( void ) const { return verts; };
	ID_INLINE const triIndex_t*	Indexes( void ) const { return indexes; }
	ID_INLINE const triIndex_t*	SilIndexes( void ) const { return silIndexes; }
	ID_INLINE const silEdge_t*	SilEdges( void ) const { return silEdges; }
	const dominantTri_t*		DominantTris( void ) const { return dominantTris; }
	const idShadowVert* 		PreLightShadowVertexes( void ) const { return preLightShadowVertexes; };
	const idRenderModelStatic*	StaticModelWithJoints( void ) const { return staticModelWithJoints; }
	const idPlane*				FacePlanes( void ) const { return facePlanes; }

	ID_INLINE const vertCacheHandle_t	IndexCache( void ) const { return indexCache; };				
	ID_INLINE const vertCacheHandle_t	AmbientCache( void ) const { return ambientCache; };			
	ID_INLINE const vertCacheHandle_t	ShadowCache( void ) const { return shadowCache; };			

private:
	bool		facePlanesCalculated;	// set when the face planes have been calculated
	bool		generateNormals;		// create normals from geometry, instead of using explicit ones
	bool		tangentsCalculated;		// set when the vertex tangents have been calculated
	bool		perfectHull;			// true if there aren't any dangling edges
	bool		referencedVerts;		// if true the 'verts' are referenced and should not be freed
	bool		referencedIndexes;		// if true, indexes, silIndexes, mirrorVerts, and silEdges are pointers into the original surface, and should not be freed

	uint32_t	numVerts;				// number of vertices
	uint32_t	numIndexes;				// for shadows, this has both front and rear end caps and silhouette planes
	uint32_t	numMirroredVerts;		// this many verts at the end of the vert list are tangent mirrors
	uint32_t	numSilEdges;			// number of silhouette edges
	uint32_t	numDupVerts;			// number of duplicate vertexes
	uint32_t	numShadowIndexesNoFrontCaps;	// shadow volumes with front caps omitted
	uint32_t	numShadowIndexesNoCaps;	// shadow volumes with the front and rear caps omitted
	int			shadowCapPlaneBits;		// bits 0-5 are set when that plane of the interacting light has triangles

	// data in vertex object space, not directly readable by the CPU
	vertCacheHandle_t			indexCache;				// GL_INDEX_TYPE
	vertCacheHandle_t			ambientCache;			// idDrawVert
	vertCacheHandle_t			shadowCache;			// idVec4

	idBounds					bounds;					// for culling
	
    idDrawVert* 				verts;					// vertices, allocated with special allocator
	triIndex_t* 				indexes;				// indexes, allocated with special allocator
	triIndex_t* 				silIndexes;				// indexes changed to be the first vertex with same XYZ, ignoring normal and texcoords
	int* 						mirroredVerts;			// tri->mirroredVerts[0] is the mirror of tri->NumVerts() - tri->numMirroredVerts + 0
	int* 						dupVerts;				// pairs of the number of the first vertex and the number of the duplicate vertex
	silEdge_t* 					silEdges;				// silhouette edges
	dominantTri_t* 				dominantTris;			// [numVerts] for deformed surface fast tangent calculation
	
	// projected on it, which means that if the view is on the outside of that
	// plane, we need to draw the rear caps of the shadow volume
	// dynamic shadows will have SHADOW_CAP_INFINITE
	idShadowVert* 				preLightShadowVertexes;	// shadow vertices in CPU memory for pre-light shadow volumes
	idShadowVert* 				staticShadowVertexes;	// shadow vertices in CPU memory for static shadow volumes
	
	crDrawGeometry* 			ambientSurface;			// for light interactions, point back at the original surface that generated
	// the interaction, which we will get the ambientCache from
	
	crDrawGeometry* 			nextDeferredFree;		// chain of tris to free next frame
	
	// for deferred normal / tangent transformations by joints
	// the jointsInverted list / buffer object on md5WithJoints may be
	// shared by multiple crDrawGeometry
	idRenderModelStatic* 		staticModelWithJoints;
	
	idPlane* facePlanes;

public:
	void	AllocStaticTriSurfPlanes( const uint32_t numIndexes );

	int*	CreateSilRemap( void );

	/// @brief If the surface will not deform, coplanar edges (polygon interiors) can never create silhouette plains, and can be omited
	/// @param omitCoplanarEdges true to omit coplanar edges (polygon interiors)
	void	IdentifySilEdges( bool omitCoplanarEdges );

	void	TestDegenerateTextureSpace( void );

	void	DuplicateMirroredVertexes( void );

	/// @brief Returns true if the texture polarity of the face is negative, false if it is positive or zero 
	bool 	FaceNegativePolarity( const uint32_t firstIndex );

	void	CreateDupVerts( void );

	/// @brief Find the largest triangle that uses each vertex
	void	BuildDominantTris( void );

	/// @brief This is called once for static surfaces, and every frame for deforming surfaces.
	/// Builds tangents, normals, and face planes
	//void	DeriveTangents( void );

	void	DeriveUnsmoothedNormalsAndTangents( void );

	void	DeriveTangentsWithoutNormals( void );

	void	DeriveNormalsAndTangents( void );

	crDrawGeometry(const crDrawGeometry&) = delete;
	crDrawGeometry operator = (const crDrawGeometry&) = delete;
};

#endif //__GEOMETRY_H__