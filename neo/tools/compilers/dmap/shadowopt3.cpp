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

#define TRIANGLE_CULLED(p1,p2,p3) ( pointCull[p1] & pointCull[p2] & pointCull[p3] & 0x3f )
#define TRIANGLE_CLIPPED(p1,p2,p3) ( ( ( pointCull[p1] & pointCull[p2] & pointCull[p3] ) & 0xfc0 ) != 0xfc0 )
#define	POINT_CULLED( p1 ) ( ( pointCull[p1] & 0xfc0 ) != 0xfc0 ) // a point that is on the plane is NOT culled
#define EDGE_CULLED(p1,p2) ( ( pointCull[p1] ^ 0xfc0 ) & ( pointCull[p2] ^ 0xfc0 ) & 0xfc0 ) // an edge that is on the plane is NOT culled
#define EDGE_CLIPPED(p1,p2) ( ( pointCull[p1] & pointCull[p2] & 0xfc0 ) != 0xfc0 )

constexpr int 	MAX_CLIP_SIL_EDGES = 2048;
constexpr int 	MAX_SHADOW_INDEXES = 0x18000;
constexpr int 	MAX_SHADOW_VERTS = 0x18000;
constexpr int	MAX_CLIPPED_POINTS = 20;

typedef enum 
{
	SG_DYNAMIC,		// use infinite projections
	SG_STATIC,		// clip to bounds
	SG_OFFLINE		// perform very time consuming optimizations
} shadowGen_t;


typedef struct 
{
	int		numVerts;
	idVec3	verts[MAX_CLIPPED_POINTS];
	int		edgeFlags[MAX_CLIPPED_POINTS];
} clipTri_t;

typedef struct 
{
	int		frontCapStart;
	int		rearCapStart;
	int		silStart;
	int		end;
} indexRef_t;

static bool 	overflowed = false;
static bool		callOptimizer = false;			// call the preprocessor optimizer after clipping occluders

static int		numShadowIndexes = 0; 
static int		numShadowVerts = 0;
static int		numClipSilEdges = 0;
static int 		indexFrustumNumber = 0;		// which shadow generating side of a light the indexRef is for
static int		c_caps = 0;
static int		c_sils = 0;

static uint16_t		shadowIndexes[MAX_SHADOW_INDEXES];
static idVec4		shadowVerts[MAX_SHADOW_VERTS];
static int			clipSilEdges[MAX_CLIP_SIL_EDGES][2];
static indexRef_t	indexRef[6];

static byte*	globalFacing; // facing will be 0 if forward facing, 1 if backwards facing grabbed with alloca
static byte*	faceCastsShadow; // faceCastsShadow will be 1 if the face is in the projection and facing the apropriate direction
static int*		remap = nullptr;

#include "dmap.h"
#include "renderer/tr_local.h"

/*

  given a set of faces that are clipped to the required frustum

  make 2D projection for each vertex

  for each edge
	add edge, generating new points at each edge intersection

  ?add all additional edges to make a full triangulation

  make full triangulation

  for each triangle
	find midpoint
	find original triangle with midpoint closest to view
	annotate triangle with that data
	project all vertexes to that plane
	output the triangle as a front cap

  snap all vertexes
  make a back plane projection for all vertexes

  for each edge
	if one side doesn't have a triangle
		make a sil edge to back plane projection
		continue
	if triangles on both sides have two verts in common
		continue
	make a sil edge from one triangle to the other
		



  classify triangles on common planes, so they can be optimized 

  what about interpenetrating triangles???

  a perfect shadow volume will have every edge exactly matched with
  an opposite, and no two triangles covering the same area on either
  the back projection or a silhouette edge.

  Optimizing the triangles on the projected plane can give a significant
  improvement, but the quadratic time nature of the optimization process
  probably makes it untenable.

  There exists some small room for further triangle count optimizations of the volumes
  by collapsing internal surface geometry in some cases, or allowing original triangles
  to extend outside the exactly light frustum without being clipped, but it probably
  isn't worth it.

  Triangle count optimizations at the expense of a slight fill rate cost
  may be apropriate in some cases.


  Perform the complete clipping on all triangles
  for each vertex
	project onto the apropriate plane and mark plane bit as in use
for each triangle
	if points project onto different planes, clip 
*/
typedef struct 
{
	idVec3	v[3];
	idVec3	edge[3];	// positive side is inside the triangle
	uint	index[3];
	idPlane	plane;		// positive side is forward for the triangle, which is away from the light
	int		planeNum;	// from original triangle, not calculated from the clipped verts
} shadowTri_t;

static const int MAX_SHADOW_TRIS = 32768;

static	shadowTri_t	outputTris[MAX_SHADOW_TRIS];
static	int		numOutputTris;

typedef struct shadowOptEdge_s {
	uint	index[2];
	struct shadowOptEdge_s	*nextEdge;
} shadowOptEdge_t;

static const int MAX_SIL_EDGES = MAX_SHADOW_TRIS*3;
static	shadowOptEdge_t	silEdges[MAX_SIL_EDGES];
static	int		numSilEdges;

typedef struct silQuad_s {
	int		nearV[2];
	int		farV[2];		// will always be a projection of near[]
	struct silQuad_s	*nextQuad;
} silQuad_t;

static const int MAX_SIL_QUADS = MAX_SHADOW_TRIS*3;
static	silQuad_t	silQuads[MAX_SIL_QUADS];
static int		numSilQuads;


typedef struct {
	idVec3	normal;	// all sil planes go through the projection origin
	shadowOptEdge_t	*edges;
	silQuad_t		*fragmentedQuads;
} silPlane_t;

static float EDGE_PLANE_EPSILON	= 0.1f;
static float UNIQUE_EPSILON = 0.1f;

static int		numSilPlanes;
static silPlane_t	*silPlanes;

// the uniqued verts are still in projection centered space, not global space
static	int		numUniqued;
static	int		numUniquedBeforeProjection;
static	int		maxUniqued;
static	idVec3	*uniqued;

static	optimizedShadow_t	ret;
static	int		maxRetIndexes;

static int FindUniqueVert( idVec3 &v );

//=====================================================================================

/*
=================
CreateEdgesForTri
=================
*/
static void CreateEdgesForTri( shadowTri_t *tri ) {
	for ( int j = 0 ; j < 3 ; j++ ) {
		idVec3 &v1 = tri->v[j];
		idVec3 &v2 = tri->v[(j+1)%3];

		tri->edge[j].Cross( v2, v1 );
		tri->edge[j].Normalize();
	}
}


static const float EDGE_EPSILON = 0.1f;

static bool TriOutsideTri( const shadowTri_t *a, const shadowTri_t *b ) {
#if 0
	if ( a->v[0] * b->edge[0] <= EDGE_EPSILON
		&& a->v[1] * b->edge[0] <= EDGE_EPSILON
		&& a->v[2] * b->edge[0] <= EDGE_EPSILON ) {
			return true;
		}
	if ( a->v[0] * b->edge[1] <= EDGE_EPSILON
		&& a->v[1] * b->edge[1] <= EDGE_EPSILON
		&& a->v[2] * b->edge[1] <= EDGE_EPSILON ) {
			return true;
		}
	if ( a->v[0] * b->edge[2] <= EDGE_EPSILON
		&& a->v[1] * b->edge[2] <= EDGE_EPSILON
		&& a->v[2] * b->edge[2] <= EDGE_EPSILON ) {
			return true;
		}
#else
	for ( int i = 0 ; i < 3 ; i++ ) {
		int		j;
		for ( j = 0 ; j < 3 ; j++ ) {
			float	d = a->v[j] * b->edge[i];
			if ( d > EDGE_EPSILON ) {
				break;
			}
		}
		if ( j == 3 ) {
			return true;
		}
	}
#endif
	return false;
}

static bool TriBehindTri( const shadowTri_t *a, const shadowTri_t *b ) {
	float	d;
	
	d = b->plane.Distance( a->v[0] );
	if ( d > 0 ) {
		return true;
	}
	d = b->plane.Distance( a->v[1] );
	if ( d > 0 ) {
		return true;
	}
	d = b->plane.Distance( a->v[2] );
	if ( d > 0 ) {
		return true;
	}

	return false;
}

/*
===================
ClipTriangle_r
===================
*/
static int c_removedFragments;
static void ClipTriangle_r( const shadowTri_t *tri, int startTri, int skipTri, int numTris, const shadowTri_t *tris ) {
	// create edge planes for this triangle

	// compare against all the other triangles
	for ( int i = startTri ; i < numTris ; i++ ) {
		if ( i == skipTri ) {
			continue;
		}
		const shadowTri_t	*other = &tris[i];

		if ( TriOutsideTri( tri, other ) ) {
			continue;
		}
		if ( TriOutsideTri( other, tri ) ) {
			continue;
		}
		// they overlap to some degree

		// if other is behind tri, it doesn't clip it
		if ( !TriBehindTri( tri, other ) ) {
			continue;
		}

		// clip it
		idWinding	*w = new idWinding( tri->v, 3 );

		for ( int j = 0 ; j < 4 && w ; j++ ) {
			idWinding	*front, *back;

			// keep any portion in front of other's plane
			if ( j == 0 ) {
				w->Split( other->plane, ON_EPSILON, &front, &back );
			} else {
				w->Split( idPlane( other->edge[j-1], 0.0f ), ON_EPSILON, &front, &back );
			}
			if ( back ) {
				// recursively clip these triangles to all subsequent triangles
				for ( int k = 2 ; k < back->GetNumPoints() ; k++ ) {
					shadowTri_t	fragment = *tri;
					
					fragment.v[0] = (*back)[0].ToVec3();
					fragment.v[1] = (*back)[k-1].ToVec3();
					fragment.v[2] = (*back)[k].ToVec3();
					CreateEdgesForTri( &fragment );
					ClipTriangle_r( &fragment, i + 1, skipTri, numTris, tris );
				}
				delete back;
			}

			delete w;
			w = front;
		}
		if ( w ) {
			delete w;
		}

		c_removedFragments++;
		// any fragments will have been added recursively
		return;
	}

	// this fragment is frontmost, so add it to the output list
	if ( numOutputTris == MAX_SHADOW_TRIS ) {
		common->Error( "numOutputTris == MAX_SHADOW_TRIS" );
	}

	outputTris[numOutputTris] = *tri;
	numOutputTris++;
}


/*
====================
ClipOccluders

Generates outputTris by clipping all the triangles against each other,
retaining only those closest to the projectionOrigin
====================
*/
static void ClipOccluders( idVec4 *verts, uint16_t *indexes, int numIndexes, 
										 idVec3 projectionOrigin ) {
	int					numTris = numIndexes / 3;
	int					i;
	shadowTri_t			*tris = (shadowTri_t *)_alloca( numTris * sizeof( *tris ) );
	shadowTri_t			*tri;

	common->Printf( "ClipOccluders: %i triangles\n", numTris );

	for ( i = 0 ; i < numTris ; i++ ) {
		tri = &tris[i];

		// the indexes are in reversed order from tr_stencilshadow
		tri->v[0] = verts[indexes[i*3+2]].ToVec3() - projectionOrigin;
		tri->v[1] = verts[indexes[i*3+1]].ToVec3() - projectionOrigin;
		tri->v[2] = verts[indexes[i*3+0]].ToVec3() - projectionOrigin;

		idVec3	d1 = tri->v[1] - tri->v[0];
		idVec3	d2 = tri->v[2] - tri->v[0];
		
		tri->plane.ToVec4().ToVec3().Cross( d2, d1 );
		tri->plane.ToVec4().ToVec3().Normalize();
		tri->plane[3] = - ( tri->v[0] * tri->plane.ToVec4().ToVec3() );

		// get the plane number before any clipping
		// we should avoid polluting the regular dmap planes with these
		// that are offset from the light origin...
		tri->planeNum = FindFloatPlane( tri->plane );

		CreateEdgesForTri( tri );
	}

	// clear our output buffer
	numOutputTris = 0;

	// for each triangle, clip against all other triangles
	int	numRemoved = 0;
	int	numComplete = 0;
	int numFragmented = 0;

	for ( i = 0 ; i < numTris ; i++ ) {
		int oldOutput = numOutputTris;
		c_removedFragments = 0;
		ClipTriangle_r( &tris[i], 0, i, numTris, tris );
		if ( numOutputTris == oldOutput ) {
			numRemoved++;		// completely unused
		} else if ( c_removedFragments == 0 ) {
			// the entire triangle is visible
			numComplete++;
			shadowTri_t	*out = &outputTris[oldOutput];
			*out = tris[i];
			numOutputTris = oldOutput+1;
		} else {
			numFragmented++;
			// we made at least one fragment

			// if we are at the low optimization level, just use a single
			// triangle if it produced any fragments
			if ( dmapGlobals.shadowOptLevel == SO_CULL_OCCLUDED ) {
				shadowTri_t	*out = &outputTris[oldOutput];
				*out = tris[i];
				numOutputTris = oldOutput+1;
			}
		}
	}
	common->Printf( "%i triangles completely invisible\n", numRemoved );
	common->Printf( "%i triangles completely visible\n", numComplete );
	common->Printf( "%i triangles fragmented\n", numFragmented );
	common->Printf( "%i shadowing fragments before optimization\n", numOutputTris );
}

//=====================================================================================

/*
================
OptimizeOutputTris
================
*/
static void OptimizeOutputTris( void ) {
	int		i;

	// optimize the clipped surfaces
	optimizeGroup_t		*optGroups = NULL;
	optimizeGroup_t *checkGroup;

	for ( i = 0 ; i < numOutputTris ; i++ ) {
		shadowTri_t		*tri = &outputTris[i];

		int	planeNum = tri->planeNum;

		// add it to an optimize group
		for ( checkGroup = optGroups ; checkGroup ; checkGroup = checkGroup->nextGroup ) {
			if ( checkGroup->planeNum == planeNum ) {
				break;
			}
		}
		if ( !checkGroup ) {
			// create a new optGroup
			checkGroup = (optimizeGroup_t *)Mem_ClearedAlloc( sizeof( *checkGroup ), TAG_DMAP );
			checkGroup->planeNum = planeNum;
			checkGroup->nextGroup = optGroups;
			optGroups = checkGroup;
		}

		// create a mapTri for the optGroup
		mapTri_t	*mtri = (mapTri_t *)Mem_ClearedAlloc( sizeof( *mtri ), TAG_DMAP );
		mtri->v[0].xyz = tri->v[0];
		mtri->v[1].xyz = tri->v[1];
		mtri->v[2].xyz = tri->v[2];
		mtri->next = checkGroup->triList;
		checkGroup->triList = mtri;
	}

	OptimizeGroupList( optGroups );

	numOutputTris = 0;
	for ( checkGroup = optGroups ; checkGroup ; checkGroup = checkGroup->nextGroup ) {
		for ( mapTri_t *mtri = checkGroup->triList ; mtri ; mtri = mtri->next ) {
			shadowTri_t *tri = &outputTris[numOutputTris];
			numOutputTris++;
			tri->v[0] = mtri->v[0].xyz;
			tri->v[1] = mtri->v[1].xyz;
			tri->v[2] = mtri->v[2].xyz;
		}
	}
	FreeOptimizeGroupList( optGroups ); 
}

//==================================================================================

static int EdgeSort( const void *a,  const void *b ) {
	if ( *(unsigned *)a < *(unsigned *)b ) {
		return -1;
	}
	if ( *(unsigned *)a > *(unsigned *)b ) {
		return 1;
	}
	return 0;
}

/*
=====================
GenerateSilEdges

Output tris must be tjunction fixed and vertex uniqued
A edge that is not exactly matched is a silhouette edge
We could skip this and rely completely on the matched quad removal
for all sil edges, but this will avoid the bulk of the checks.
=====================
*/
static void GenerateSilEdges( void ) {
	int		i, j;

	unsigned	*edges = (unsigned *)_alloca( (numOutputTris*3+1)*sizeof(*edges) );
	int		numEdges = 0;

	numSilEdges = 0;

	for ( i = 0 ; i < numOutputTris ; i++ ) {
		int a = outputTris[i].index[0];
		int b = outputTris[i].index[1];
		int c = outputTris[i].index[2];
		if ( a == b || a == c || b == c ) {
			continue;		// degenerate
		}

		for ( j = 0 ; j < 3 ; j++ ) {
			int	v1, v2;

			v1 = outputTris[i].index[j];
			v2 = outputTris[i].index[(j+1)%3];
			if ( v1 == v2 ) {
				continue;		// degenerate
			}
			if ( v1 > v2 ) {
				edges[numEdges] = ( v1 << 16 ) | ( v2 << 1 );
			} else {
				edges[numEdges] = ( v2 << 16 ) | ( v1 << 1 ) | 1;
			}
			numEdges++;
		}
	}

	qsort( edges, numEdges, sizeof( edges[0] ), EdgeSort );
	edges[numEdges] = -1;	// force the last to make an edge if no matched to previous

	for ( i = 0 ; i < numEdges ; i++ ) {
		if ( ( edges[i] ^ edges[i+1] ) == 1 ) {
			// skip the next one, because we matched and
			// removed both
			i++;
			continue;
		}
		// this is an unmatched edge, so we need to generate a sil plane
		int		v1, v2;
		if ( edges[i] & 1 ) {
			v2 = edges[i] >> 16;
			v1 = ( edges[i] >> 1 ) & 0x7fff;
		} else {
			v1 = edges[i] >> 16;
			v2 = ( edges[i] >> 1 ) & 0x7fff;
		}

		if ( numSilEdges == MAX_SIL_EDGES ) {
			common->Error( "numSilEdges == MAX_SIL_EDGES" );
		}
		silEdges[numSilEdges].index[0] = v1;
		silEdges[numSilEdges].index[1] = v2;
		numSilEdges++;
	}
}

//==================================================================================

/*
=====================
GenerateSilPlanes

Groups the silEdges into common planes
=====================
*/
void GenerateSilPlanes( void ) {
	numSilPlanes = 0;
	silPlanes = (silPlane_t *)Mem_Alloc( sizeof( *silPlanes ) * numSilEdges, TAG_DMAP );

	// identify the silPlanes
	numSilPlanes = 0;
	for ( int i = 0 ; i < numSilEdges ; i++ ) {
		if ( silEdges[i].index[0] == silEdges[i].index[1] ) {
			continue;	// degenerate
		}

		idVec3 &v1 = uniqued[silEdges[i].index[0]];
		idVec3 &v2 = uniqued[silEdges[i].index[1]];

		// search for an existing plane
		int	j;
		for ( j = 0 ; j < numSilPlanes ; j++ ) {
			float d = v1 * silPlanes[j].normal;
			float d2 = v2 * silPlanes[j].normal;

			if ( fabs( d ) < EDGE_PLANE_EPSILON
				&& fabs( d2 ) < EDGE_PLANE_EPSILON ) {
				silEdges[i].nextEdge = silPlanes[j].edges;
				silPlanes[j].edges = &silEdges[i];
				break;
			}
		}

		if ( j == numSilPlanes ) {
			// create a new silPlane
			silPlanes[j].normal.Cross( v2, v1 );
			silPlanes[j].normal.Normalize();
			silEdges[i].nextEdge = NULL;
			silPlanes[j].edges = &silEdges[i];
			silPlanes[j].fragmentedQuads = NULL;
			numSilPlanes++;
		}
	}
}

//==================================================================================

/*
=============
SaveQuad
=============
*/
static void SaveQuad( silPlane_t *silPlane, silQuad_t &quad ) {
	// this fragment is a final fragment
	if ( numSilQuads == MAX_SIL_QUADS ) {
		common->Error( "numSilQuads == MAX_SIL_QUADS" );
	}
	silQuads[numSilQuads] = quad;
	silQuads[numSilQuads].nextQuad = silPlane->fragmentedQuads;
	silPlane->fragmentedQuads = &silQuads[numSilQuads];
	numSilQuads++;
}


/*
===================
FragmentSilQuad

Clip quads, or reconstruct?
Generate them T-junction free, or require another pass of fix-tjunc?
Call optimizer on a per-sil-plane basis?
	will this ever introduce tjunctions with the front faces?
	removal of planes can allow the rear projection to be farther optimized

For quad clipping
	PlaneThroughEdge

quad clipping introduces new vertexes

Cannot just fragment edges, must emit full indexes

what is the bounds on max indexes?
	the worst case is that all edges but one carve an existing edge in the middle,
	giving twice the input number of indexes (I think)

can we avoid knowing about projected positions and still optimize?

Fragment all edges first
Introduces T-junctions
create additional silEdges, linked to silPlanes

In theory, we should never have more than one edge clipping a given
fragment, but it is more robust if we check them all
===================
*/
static void FragmentSilQuad( silQuad_t quad, silPlane_t *silPlane, shadowOptEdge_t *startEdge, shadowOptEdge_t *skipEdge ) 
							{
	if ( quad.nearV[0] == quad.nearV[1] ) 
		return;

	for ( shadowOptEdge_t *check = startEdge ; check ; check = check->nextEdge ) 
	{
		if ( check == skipEdge ) 
			// don't clip against self
			continue;

		if ( check->index[0] == check->index[1] ) 
			continue;

		// make planes through both points of check
		for ( int i = 0 ; i < 2 ; i++ ) 
		{
			idVec3	plane;

			plane.Cross( uniqued[check->index[i]], silPlane->normal );
			plane.Normalize();

			if ( plane.Length() < 0.9 ) 
				continue;

			// if the other point on check isn't on the negative side of the plane,
			// flip the plane
			if ( uniqued[check->index[!i]] * plane > 0 ) 
				plane = -plane;

			float d1 = uniqued[quad.nearV[0]] * plane;
			float d2 = uniqued[quad.nearV[1]] * plane;

			float d3 = uniqued[quad.farV[0]] * plane;
			float d4 = uniqued[quad.farV[1]] * plane;

			// it is better to conservatively NOT split the quad, which, at worst,
			// will leave some extra overdraw

			// if the plane divides the incoming edge, split it and recurse
			// with the outside fraction before continuing with the inside fraction
			if ( ( d1 > EDGE_PLANE_EPSILON && d3 > EDGE_PLANE_EPSILON && d2 < -EDGE_PLANE_EPSILON && d4 < -EDGE_PLANE_EPSILON )
				||  ( d2 > EDGE_PLANE_EPSILON && d4 > EDGE_PLANE_EPSILON && d1 < -EDGE_PLANE_EPSILON && d3 < -EDGE_PLANE_EPSILON ) ) 
				{
				float f = d1 / ( d1 - d2 );
				float f2 = d3 / ( d3 - d4 );
				f = f2;
				if ( f <= 0.0001 || f >= 0.9999 ) {
					common->Error( "Bad silQuad fraction" );
				}

				// finding uniques may be causing problems here
				idVec3	nearMid = (1-f) * uniqued[quad.nearV[0]] + f * uniqued[quad.nearV[1]];
				int nearMidIndex = FindUniqueVert( nearMid );
				idVec3	farMid = (1-f) * uniqued[quad.farV[0]] + f * uniqued[quad.farV[1]];
				int farMidIndex = FindUniqueVert( farMid );

				silQuad_t	clipped = quad;

				if ( d1 > EDGE_PLANE_EPSILON ) 
				{
					clipped.nearV[1] = nearMidIndex;
					clipped.farV[1] = farMidIndex;
					FragmentSilQuad( clipped, silPlane, check->nextEdge, skipEdge );
					quad.nearV[0] = nearMidIndex;
					quad.farV[0] = farMidIndex;
				} 
				else 
				{
					clipped.nearV[0] = nearMidIndex;
					clipped.farV[0] = farMidIndex;
					FragmentSilQuad( clipped, silPlane, check->nextEdge, skipEdge );
					quad.nearV[1] = nearMidIndex;
					quad.farV[1] = farMidIndex;
				}
			}
		}

		// make a plane through the line of check
		idPlane	separate;

		idVec3	dir = uniqued[check->index[1]] - uniqued[check->index[0]];
		separate.Normal().Cross( dir, silPlane->normal );
		separate.Normal().Normalize();
		separate.ToVec4()[3] = -(uniqued[check->index[1]] * separate.Normal());

		// this may miss a needed separation when the quad would be
		// clipped into a triangle and a quad
		float d1 = separate.Distance( uniqued[quad.nearV[0]] );
		float d2 = separate.Distance( uniqued[quad.farV[0]] );

		if ( ( d1 < EDGE_PLANE_EPSILON && d2 < EDGE_PLANE_EPSILON )
			|| ( d1 > -EDGE_PLANE_EPSILON && d2 > -EDGE_PLANE_EPSILON ) ) {
				continue;
		}

		// split the quad at this plane
		float f = d1 / ( d1 - d2 );
		idVec3	mid0 = (1-f) * uniqued[quad.nearV[0]] + f * uniqued[quad.farV[0]];
		int mid0Index = FindUniqueVert( mid0 );

		d1 = separate.Distance( uniqued[quad.nearV[1]] );
		d2 = separate.Distance( uniqued[quad.farV[1]] );
		f = d1 / ( d1 - d2 );
		if ( f < 0 || f > 1 ) {
			continue;
		}

		idVec3	mid1 = (1-f) * uniqued[quad.nearV[1]] + f * uniqued[quad.farV[1]];
		int mid1Index = FindUniqueVert( mid1 );

		silQuad_t	clipped = quad;

		clipped.nearV[0] = mid0Index;
		clipped.nearV[1] = mid1Index;
		FragmentSilQuad( clipped, silPlane, check->nextEdge, skipEdge );
		quad.farV[0] = mid0Index;
		quad.farV[1] = mid1Index;
	}

	SaveQuad( silPlane, quad );
}


/*
===============
FragmentSilQuads
===============
*/
static void FragmentSilQuads( void ) 
{
	// group the edges into common planes
	GenerateSilPlanes();

	numSilQuads = 0;

	// fragment overlapping edges
	for ( int i = 0 ; i < numSilPlanes ; i++ ) {
		silPlane_t *sil = &silPlanes[i];

		for ( shadowOptEdge_t *e1 = sil->edges ; e1 ; e1 = e1->nextEdge ) {
			silQuad_t	quad{};

			quad.nearV[0] = e1->index[0];
			quad.nearV[1] = e1->index[1];
			if ( e1->index[0] == e1->index[1] ) {
				common->Error( "FragmentSilQuads: degenerate edge" );
			}
			quad.farV[0] = e1->index[0] + numUniquedBeforeProjection;
			quad.farV[1] = e1->index[1] + numUniquedBeforeProjection;
			FragmentSilQuad( quad, sil, sil->edges, e1 );
		}
	}
}

//=======================================================================

/*
=====================
EmitFragmentedSilQuads

=====================
*/
static void EmitFragmentedSilQuads( void ) 
{
	int		i, j, k;
	mapTri_t	*mtri;

	for ( i = 0 ; i < numSilPlanes ; i++ ) {
		silPlane_t	*sil = &silPlanes[i];

		// prepare for optimizing the sil quads on each side of the sil plane
		optimizeGroup_t	groups[2];
		memset( &groups, 0, sizeof( groups ) );
		idPlane		planes[2];
		planes[0].Normal() = sil->normal;
		planes[0][3] = 0;
		planes[1] = -planes[0];
		groups[0].planeNum = FindFloatPlane( planes[0] );
		groups[1].planeNum = FindFloatPlane( planes[1] );

		// emit the quads that aren't matched
		for ( silQuad_t *f1 = sil->fragmentedQuads ; f1 ; f1 = f1->nextQuad ) {
			silQuad_t *f2;
			for ( f2 = sil->fragmentedQuads ; f2 ; f2 = f2->nextQuad ) {
				if ( f2 == f1 ) {
					continue;
				}
				// in theory, this is sufficient, but we might
				// have some cases of tripple+ matching, or unclipped rear projections
				if ( f1->nearV[0] == f2->nearV[1] && f1->nearV[1] == f2->nearV[0] ) {
					break;
				}
			}
			// if we went through all the quads without finding a match, emit the quad
			if ( !f2 ) {
				optimizeGroup_t	*gr;
				idVec3	v1, v2, normal;

				mtri = (mapTri_t *)Mem_ClearedAlloc( sizeof( *mtri ), TAG_DMAP );
				mtri->v[0].xyz = uniqued[f1->nearV[0]];
				mtri->v[1].xyz = uniqued[f1->nearV[1]];
				mtri->v[2].xyz = uniqued[f1->farV[1]];

				v1 = mtri->v[1].xyz - mtri->v[0].xyz;
				v2 = mtri->v[2].xyz - mtri->v[0].xyz;
				normal.Cross( v2, v1 );

				if ( normal * planes[0].Normal() > 0 ) {
					gr = &groups[0];
				} else {
					gr = &groups[1];
				}

				mtri->next = gr->triList;
				gr->triList = mtri;

				mtri = (mapTri_t *)Mem_ClearedAlloc( sizeof( *mtri ), TAG_DMAP );
				mtri->v[0].xyz = uniqued[f1->farV[0]];
				mtri->v[1].xyz = uniqued[f1->nearV[0]];
				mtri->v[2].xyz = uniqued[f1->farV[1]];

				mtri->next = gr->triList;
				gr->triList = mtri;

#if 0
				// emit a sil quad all the way to the projection plane
				int index = ret.totalIndexes;
				if ( index + 6 > maxRetIndexes ) {
					common->Error( "maxRetIndexes exceeded" );
				}
				ret.indexes[index+0] = f1->nearV[0];
				ret.indexes[index+1] = f1->nearV[1];
				ret.indexes[index+2] = f1->farV[1];
				ret.indexes[index+3] = f1->farV[0];
				ret.indexes[index+4] = f1->nearV[0];
				ret.indexes[index+5] = f1->farV[1];
				ret.totalIndexes += 6;
#endif
			}
		}


		// optimize
		for ( j = 0 ; j < 2 ; j++ ) {
			if ( !groups[j].triList ) {
				continue;
			}
			if ( dmapGlobals.shadowOptLevel == SO_SIL_OPTIMIZE ) {
				OptimizeGroupList( &groups[j] );
			}
			// add as indexes
			for ( mtri = groups[j].triList ; mtri ; mtri = mtri->next ) {
				for ( k = 0 ; k < 3 ; k++ ) {
					if ( ret.totalIndexes == maxRetIndexes ) {
						common->Error( "maxRetIndexes exceeded" );
					}
					ret.indexes[ret.totalIndexes] = FindUniqueVert( mtri->v[k].xyz );
					ret.totalIndexes++;
				}
			}
			FreeTriList( groups[j].triList );
		}
	}

	// we don't need the silPlane grouping anymore
	Mem_Free( silPlanes );
}

/*
=================
EmitUnoptimizedSilEdges
=================
*/
static void EmitUnoptimizedSilEdges( void ) {
	int	i;

	for ( i = 0 ; i < numSilEdges ; i++ ) {
		int v1 = silEdges[i].index[0];
		int v2 = silEdges[i].index[1];
		int index = ret.totalIndexes;
		ret.indexes[index+0] = v1;
		ret.indexes[index+1] = v2;
		ret.indexes[index+2] = v2+numUniquedBeforeProjection;
		ret.indexes[index+3] = v1+numUniquedBeforeProjection;
		ret.indexes[index+4] = v1;
		ret.indexes[index+5] = v2+numUniquedBeforeProjection;
		ret.totalIndexes += 6;
	}
}

//==================================================================================

/*
================
FindUniqueVert
================
*/
static int FindUniqueVert( idVec3 &v ) {
	int	k;

	for ( k = 0 ; k < numUniqued ; k++ ) {
		idVec3 &check = uniqued[k];
		if ( fabs( v[0] - check[0] ) < UNIQUE_EPSILON
		&& fabs( v[1] - check[1] ) < UNIQUE_EPSILON
		&& fabs( v[2] - check[2] ) < UNIQUE_EPSILON ) {
			return k;
		}
	}
	if ( numUniqued == maxUniqued ) {
		common->Error( "FindUniqueVert: numUniqued == maxUniqued" );
	}
	uniqued[numUniqued] = v;
	numUniqued++;

	return k;
}

/*
===================
UniqueVerts

Snaps all triangle verts together, setting tri->index[]
and generating numUniqued and uniqued.
These are still in projection-centered space, not global space
===================
*/
static void UniqueVerts( void ) {
	int		i, j;

	// we may add to uniqued later when splitting sil edges, so leave
	// some extra room
	maxUniqued = 100000; // numOutputTris * 10 + 1000;
	uniqued = (idVec3 *)Mem_Alloc( sizeof( *uniqued ) * maxUniqued, TAG_DMAP );
	numUniqued = 0;

	for ( i = 0 ; i < numOutputTris ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			outputTris[i].index[j] = FindUniqueVert( outputTris[i].v[j] );
		}
	}
}


/*
====================
LightProjectionMatrix
====================
*/
static void LightProjectionMatrix( const idVec3 &origin, const idPlane &rearPlane, idVec4 mat[4] ) 
{
	idVec4		lv;
	float		lg;

	// calculate the homogenious light vector
	lv.x = origin.x;
	lv.y = origin.y;
	lv.z = origin.z;
	lv.w = 1;

	lg = rearPlane.ToVec4() * lv;

	// outer product
	mat[0][0] = lg -rearPlane[0] * lv[0];
	mat[0][1] = -rearPlane[1] * lv[0];
	mat[0][2] = -rearPlane[2] * lv[0];
	mat[0][3] = -rearPlane[3] * lv[0];

	mat[1][0] = -rearPlane[0] * lv[1];
	mat[1][1] = lg -rearPlane[1] * lv[1];
	mat[1][2] = -rearPlane[2] * lv[1];
	mat[1][3] = -rearPlane[3] * lv[1];

	mat[2][0] = -rearPlane[0] * lv[2];
	mat[2][1] = -rearPlane[1] * lv[2];
	mat[2][2] = lg -rearPlane[2] * lv[2];
	mat[2][3] = -rearPlane[3] * lv[2];

	mat[3][0] = -rearPlane[0] * lv[3];
	mat[3][1] = -rearPlane[1] * lv[3];
	mat[3][2] = -rearPlane[2] * lv[3];
	mat[3][3] = lg -rearPlane[3] * lv[3];
}

/*
======================
ProjectUniqued
======================
*/
static void ProjectUniqued( idVec3 projectionOrigin, idPlane projectionPlane ) {
	// calculate the projection 
	idVec4		mat[4];

	LightProjectionMatrix( projectionOrigin, projectionPlane, mat );

	if ( numUniqued * 2 > maxUniqued ) {
		common->Error( "ProjectUniqued: numUniqued * 2 > maxUniqued" );
	}

	// this is goofy going back and forth between the spaces,
	// but I don't want to change LightProjectionMatrix righ tnow...
	for ( int i = 0 ; i < numUniqued ; i++ ) {
		// put the vert back in global space, instead of light centered space
		idVec3 in = uniqued[i] + projectionOrigin;

		// project to far plane
		float	w, oow;
		idVec3	out;

		w = in * mat[3].ToVec3() + mat[3][3];

		oow = 1.0 / w;
		out.x = ( in * mat[0].ToVec3() + mat[0][3] ) * oow;
		out.y = ( in * mat[1].ToVec3() + mat[1][3] ) * oow;
		out.z = ( in * mat[2].ToVec3() + mat[2][3] ) * oow;

		uniqued[numUniqued+i] = out - projectionOrigin;
	}
	numUniqued *= 2;
}

/*
====================
SuperOptimizeOccluders

This is the callback from the renderer shadow generation routine, after
verts have been culled against individual frustums of point lights

====================
*/
optimizedShadow_t SuperOptimizeOccluders( idVec4 *verts, uint16_t *indexes, int numIndexes, idPlane projectionPlane, idVec3 projectionOrigin )
{
	memset( &ret, 0, sizeof( ret ) );

	// generate outputTris, removing fragments that are occluded by closer fragments
	ClipOccluders( verts, indexes, numIndexes, projectionOrigin );

	if ( dmapGlobals.shadowOptLevel >= SO_CULL_OCCLUDED ) {
		OptimizeOutputTris();
	}

	// match up common verts
	UniqueVerts();

	// now that we have uniqued the vertexes, we can find unmatched
	// edges, which are silhouette planes
	GenerateSilEdges();

	// generate the projected verts
	numUniquedBeforeProjection = numUniqued;
	ProjectUniqued( projectionOrigin, projectionPlane );

	// fragment the sil edges where the overlap,
	// possibly generating some additional unique verts
	if ( dmapGlobals.shadowOptLevel >= SO_CLIP_SILS ) {
		FragmentSilQuads();
	}

	// indexes for face and projection caps
	ret.numFrontCapIndexes = numOutputTris * 3;
	ret.numRearCapIndexes = numOutputTris * 3;
	if ( dmapGlobals.shadowOptLevel >= SO_CLIP_SILS ) {
		ret.numSilPlaneIndexes = numSilQuads * 12;	// this is the worst case with clipping
	} else {
		ret.numSilPlaneIndexes = numSilEdges * 6;	// this is the worst case with clipping
	}

	ret.totalIndexes = 0;
	
	maxRetIndexes = ret.numFrontCapIndexes + ret.numRearCapIndexes + ret.numSilPlaneIndexes;

	ret.indexes = (uint16_t *)Mem_Alloc( maxRetIndexes * sizeof( ret.indexes[0] ), TAG_DMAP );
	for ( int i = 0 ; i < numOutputTris ; i++ ) {
		// flip the indexes so the surface triangle faces outside the shadow volume
		ret.indexes[i*3+0] = outputTris[i].index[2];
		ret.indexes[i*3+1] = outputTris[i].index[1];
		ret.indexes[i*3+2] = outputTris[i].index[0];

		ret.indexes[(numOutputTris+i)*3+0] = numUniquedBeforeProjection + outputTris[i].index[0];
		ret.indexes[(numOutputTris+i)*3+1] = numUniquedBeforeProjection + outputTris[i].index[1];
		ret.indexes[(numOutputTris+i)*3+2] = numUniquedBeforeProjection + outputTris[i].index[2];
	}
	// emit the sil planes
	ret.totalIndexes = ret.numFrontCapIndexes + ret.numRearCapIndexes;

	if ( dmapGlobals.shadowOptLevel >= SO_CLIP_SILS ) {
		// re-optimize the sil planes, cutting 
		EmitFragmentedSilQuads();
	} else {
		// indexes for silhouette edges
		EmitUnoptimizedSilEdges();
	}

	// we have all the verts now
	// create twice the uniqued verts
	ret.numVerts = numUniqued;
	ret.verts = (idVec3 *)Mem_Alloc( ret.numVerts * sizeof( ret.verts[0] ), TAG_DMAP );
	for ( int i = 0 ; i < numUniqued ; i++ ) {
		// put the vert back in global space, instead of light centered space
		ret.verts[i] = uniqued[i] + projectionOrigin;
	}

	// set the final index count
	ret.numSilPlaneIndexes = ret.totalIndexes - (ret.numFrontCapIndexes + ret.numRearCapIndexes);

	// free out local data
	Mem_Free( uniqued );

	return ret;
}

/*
=================
RemoveDegenerateTriangles
=================
*/
static void RemoveDegenerateTriangles( srfTriangles_t *tri ) 
{
	int		c_removed;
	int		i;
	int		a, b, c;

	// check for completely degenerate triangles
	c_removed = 0;
	for ( i = 0 ; i < tri->numIndexes ; i+=3 ) {
		a = tri->indexes[i];
		b = tri->indexes[i+1];
		c = tri->indexes[i+2];
		if ( a == b || a == c || b == c ) {
			c_removed++;
			memmove( tri->indexes + i, tri->indexes + i + 3, ( tri->numIndexes - i - 3 ) * sizeof( tri->indexes[0] ) );
			tri->numIndexes -= 3;
			if ( i < tri->numShadowIndexesNoCaps ) {
				tri->numShadowIndexesNoCaps -= 3;
			}
			if ( i < tri->numShadowIndexesNoFrontCaps ) {
				tri->numShadowIndexesNoFrontCaps -= 3;
			}
			i -= 3;
		}
	}

	// this doesn't free the memory used by the unused verts

	if ( c_removed ) {
		common->Printf( "removed %i degenerate triangles from shadow\n", c_removed );
	}
}

/*
====================
CleanupOptimizedShadowTris

Uniques all verts across the frustums
removes matched sil quads at frustum seams
removes degenerate tris
====================
*/
void CleanupOptimizedShadowTris( srfTriangles_t *tri ) 
{
	int		i;

	// unique all the verts
	maxUniqued = tri->numVerts;
	uniqued = (idVec3 *)_alloca( sizeof( *uniqued ) * maxUniqued );
	numUniqued = 0;

	uint	*remap = (uint *)_alloca( sizeof( *remap ) * tri->numVerts );

	for ( i = 0 ; i < tri->numIndexes ; i++ ) {
		if ( tri->indexes[i] > (uint)tri->numVerts || tri->indexes[i] < 0 ) {
			common->Error( "CleanupOptimizedShadowTris: index out of range" );
		}
	}

	for (i = 0; i < tri->numVerts; i++) 
	{
		//remap[i] = FindUniqueVert(tri->shadowVertexes[i].xyz.ToVec3());
		remap[i] = FindUniqueVert(tri->preLightShadowVertexes[i].xyzw.ToVec3());

	}
	
	tri->numVerts = numUniqued;
	for (i = 0; i < tri->numVerts; i++) 
	{
		//tri->shadowVertexes[i].xyz.ToVec3() = uniqued[i];
		//tri->shadowVertexes[i].xyz[3] = 1;
		tri->preLightShadowVertexes[i].xyzw.ToVec3() = uniqued[i];
		tri->preLightShadowVertexes[i].xyzw[3] = 1;
	}

	for ( i = 0 ; i < tri->numIndexes ; i++ ) 
	{
		tri->indexes[i] = remap[tri->indexes[i]];
	}

	// remove matched quads
	int	numSilIndexes = tri->numShadowIndexesNoCaps;
	for ( int i = 0 ; i < numSilIndexes ; i+=6 ) 
	{
		int	j;
		for ( j = i+6 ; j < numSilIndexes ; j+=6 ) 
		{
			// if there is a reversed quad match, we can throw both of them out
			// this is not a robust check, it relies on the exact ordering of
			// quad indexes
			if ( tri->indexes[i+0] == tri->indexes[j+1]
			&& tri->indexes[i+1] == tri->indexes[j+0]
			&& tri->indexes[i+2] == tri->indexes[j+3]
			&& tri->indexes[i+3] == tri->indexes[j+5]
			&& tri->indexes[i+4] == tri->indexes[j+1]
			&& tri->indexes[i+5] == tri->indexes[j+3] ) 
			{
				break;
			}
		}

		if ( j == numSilIndexes ) 
			continue;

		int	k = 0;

		// remove first quad
		for ( k = i+6 ; k < j ; k++ ) 
		{
			tri->indexes[k-6] = tri->indexes[k];
		}
		
		// remove second quad
		for ( k = j+6 ; k < tri->numIndexes ; k++ ) 
		{
			tri->indexes[k-12] = tri->indexes[k];
		}
		
		numSilIndexes -= 12;
		i -= 6;
	}

	int	removed = tri->numShadowIndexesNoCaps - numSilIndexes;

	tri->numIndexes -= removed;
	tri->numShadowIndexesNoCaps -= removed;
	tri->numShadowIndexesNoFrontCaps -= removed;

	// remove degenerates after we have removed quads, so the double
	// triangle pairing isn't disturbed
	RemoveDegenerateTriangles( tri );
}

/*
================
CalcPointCull

Also inits the remap[] array to all -1
================
*/
static void CalcPointCull( const srfTriangles_t *tri, const idPlane frustum[6], unsigned short *pointCull ) 
{
	int i;
	int frontBits;
	float *planeSide;
	byte *side1, *side2;

	std::memset( remap, -1, tri->numVerts * sizeof( remap[0] ) );

	for ( frontBits = 0, i = 0; i < 6; i++ ) {
		// get front bits for the whole surface
		if ( tri->bounds.PlaneDistance( frustum[i] ) >= LIGHT_CLIP_EPSILON ) {
			frontBits |= 1<<(i+6);
		}
	}

	// initialize point cull
	for ( i = 0; i < tri->numVerts; i++ ) {
		pointCull[i] = frontBits;
	}

	// if the surface is not completely inside the light frustum
	if ( frontBits == ( ( ( 1 << 6 ) - 1 ) ) << 6 ) {
		return;
	}

	planeSide = (float *) _alloca16( tri->numVerts * sizeof( float ) );
	side1 = (byte *) _alloca16( tri->numVerts * sizeof( byte ) );
	side2 = (byte *) _alloca16( tri->numVerts * sizeof( byte ) );
	SIMDProcessor->Memset( side1, 0, tri->numVerts * sizeof( byte ) );
	SIMDProcessor->Memset( side2, 0, tri->numVerts * sizeof( byte ) );

	for ( i = 0; i < 6; i++ ) 
	{

		if ( frontBits & (1<<(i+6)) ) 
			continue;

		SIMDProcessor->Dot( planeSide, frustum[i], tri->verts, tri->numVerts );
		SIMDProcessor->CmpLT( side1, i, planeSide, LIGHT_CLIP_EPSILON, tri->numVerts );
		SIMDProcessor->CmpGT( side2, i, planeSide, -LIGHT_CLIP_EPSILON, tri->numVerts );
	}

	for ( i = 0; i < tri->numVerts; i++ ) 
	{
		pointCull[i] |= side1[i] | (side2[i] << 6);
	}
}

/*
=============
ChopWinding

Clips a triangle from one buffer to another, setting edge flags
The returned buffer may be the same as inNum if no clipping is done
If entirely clipped away, clipTris[returned].numVerts == 0

I have some worries about edge flag cases when polygons are clipped
multiple times near the epsilon.
=============
*/
static int ChopWinding( clipTri_t clipTris[2], int inNum, const idPlane &plane ) 
{
	clipTri_t	*in, *out;
	float	dists[MAX_CLIPPED_POINTS];
	int		sides[MAX_CLIPPED_POINTS];
	int		counts[3];
	float	dot;
	int		i, j;
	idVec3	*p1, *p2;
	idVec3	mid;

	in = &clipTris[inNum];
	out = &clipTris[inNum^1];
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for ( i = 0 ; i < in->numVerts ; i++ ) 
	{
		dot = plane.Distance( in->verts[i] );
		dists[i] = dot;
		if ( dot < -LIGHT_CLIP_EPSILON ) 
			sides[i] = SIDE_BACK;
		else if ( dot > LIGHT_CLIP_EPSILON ) 
			sides[i] = SIDE_FRONT;
		else 
			sides[i] = SIDE_ON;
		
		counts[sides[i]]++;
	}

	// if none in front, it is completely clipped away
	if ( !counts[SIDE_FRONT] ) 
	{
		in->numVerts = 0;
		return inNum;
	}

	if ( !counts[SIDE_BACK] ) 
		return inNum;		// inout stays the same

	// avoid wrapping checks by duplicating first value to end
	sides[i] = sides[0];
	dists[i] = dists[0];
	in->verts[in->numVerts] = in->verts[0];
	in->edgeFlags[in->numVerts] = in->edgeFlags[0];

	out->numVerts = 0;
	for ( i = 0 ; i < in->numVerts ; i++ ) {
		p1 = &in->verts[i];

		if ( sides[i] != SIDE_BACK ) {
			out->verts[out->numVerts] = *p1;
			if ( sides[i] == SIDE_ON && sides[i+1] == SIDE_BACK ) {
				out->edgeFlags[out->numVerts] = 1;
			} else {
				out->edgeFlags[out->numVerts] = in->edgeFlags[i];
			}
			out->numVerts++;
		}

		if ( (sides[i] == SIDE_FRONT && sides[i+1] == SIDE_BACK)
			|| (sides[i] == SIDE_BACK && sides[i+1] == SIDE_FRONT) ) {
			// generate a split point
			p2 = &in->verts[i+1];

			dot = dists[i] / (dists[i]-dists[i+1]);
			for ( j=0 ; j<3 ; j++ ) {
				mid[j] = (*p1)[j] + dot*((*p2)[j]-(*p1)[j]);
			}

			out->verts[out->numVerts] = mid;

			// set the edge flag
			if ( sides[i+1] != SIDE_FRONT ) {
				out->edgeFlags[out->numVerts] = 1;
			} else {
				out->edgeFlags[out->numVerts] = in->edgeFlags[i];
			}

			out->numVerts++;
		}
	}

	return inNum ^ 1;
}

/*
===================
ClipTriangleToLight

Returns false if nothing is left after clipping
===================
*/
static bool	ClipTriangleToLight( const idVec3 &a, const idVec3 &b, const idVec3 &c, int planeBits,
							  const idPlane frustum[6] ) {
	int			i;
	int			base;
	clipTri_t	pingPong[2], *ct;
	int			p;

	pingPong[0].numVerts = 3;
	pingPong[0].edgeFlags[0] = 0;
	pingPong[0].edgeFlags[1] = 0;
	pingPong[0].edgeFlags[2] = 0;
	pingPong[0].verts[0] = a;
	pingPong[0].verts[1] = b;
	pingPong[0].verts[2] = c;

	p = 0;
	for ( i = 0 ; i < 6 ; i++ ) 
	{
		if ( planeBits & ( 1 << i ) ) 
		{
			p = ChopWinding( pingPong, p, frustum[i] );
			if ( pingPong[p].numVerts < 1 )
				return false;
		}
	}
	ct = &pingPong[p];

	// copy the clipped points out to shadowVerts
	if ( numShadowVerts + ct->numVerts * 2 > MAX_SHADOW_VERTS ) 
	{
		overflowed = true;
		return false;
	}

	base = numShadowVerts;
	for ( i = 0 ; i < ct->numVerts ; i++ ) {
		shadowVerts[ base + i*2 ].ToVec3() = ct->verts[i];
	}
	numShadowVerts += ct->numVerts * 2;

	if ( numShadowIndexes + 3 * ( ct->numVerts - 2 ) > MAX_SHADOW_INDEXES ) {
		overflowed = true;
		return false;
	}

	for ( i = 2 ; i < ct->numVerts ; i++ ) {
		shadowIndexes[numShadowIndexes++] = base + i * 2;
		shadowIndexes[numShadowIndexes++] = base + ( i - 1 ) * 2;
		shadowIndexes[numShadowIndexes++] = base;
	}

	// any edges that were created by the clipping process will
	// have a silhouette quad created for it, because it is one
	// of the exterior bounds of the shadow volume
	for ( i = 0 ; i < ct->numVerts ; i++ ) {
		if ( ct->edgeFlags[i] ) {
			if ( numClipSilEdges == MAX_CLIP_SIL_EDGES ) 
				break;
			
			clipSilEdges[ numClipSilEdges ][0] = base + i * 2;
			if ( i == ct->numVerts - 1 )
				clipSilEdges[ numClipSilEdges ][1] = base;
			else
				clipSilEdges[ numClipSilEdges ][1] = base + ( i + 1 ) * 2;
			
			numClipSilEdges++;
		}
	}

	return true;
}

/*
===============
PointsOrdered

To make sure the triangulations of the sil edges is consistant,
we need to be able to order two points.  We don't care about how
they compare with any other points, just that when the same two
points are passed in (in either order), they will always specify
the same one as leading.

Currently we need to have separate faces in different surfaces
order the same way, so we must look at the actual coordinates.
If surfaces are ever guaranteed to not have to edge match with
other surfaces, we could just compare indexes.
===============
*/
static bool PointsOrdered( const idVec3 &a, const idVec3 &b ) 
{
	float	i, j;

	// vectors that wind up getting an equal hash value will
	// potentially cause a misorder, which can show as a couple
	// crack pixels in a shadow

	// scale by some odd numbers so -8, 8, 8 will not be equal
	// to 8, -8, 8

	// in the very rare case that these might be equal, all that would
	// happen is an oportunity for a tiny rasterization shadow crack
	i = a[0] + a[1]*127 + a[2]*1023;
	j = b[0] + b[1]*127 + b[2]*1023;

	return (bool)(i < j);
}

/*
==================
R_AddClipSilEdges

Add sil edges for each triangle clipped to the side of
the frustum.

Only done for simple projected lights, not point lights.
==================
*/
static void AddClipSilEdges( void ) 
{
	int		v1 = 0, v2 = 0;
	int		v1_back = 0, v2_back = 0;
	int		i = 0;

	// don't allow it to overflow
	if ( numShadowIndexes + numClipSilEdges * 6 > MAX_SHADOW_INDEXES ) 
	{
		overflowed = true;
		return;
	}

	for ( i = 0 ; i < numClipSilEdges ; i++ ) 
	{
		v1 = clipSilEdges[i][0];
		v2 = clipSilEdges[i][1];
		v1_back = v1 + 1;
		v2_back = v2 + 1;
		if ( PointsOrdered( shadowVerts[ v1 ].ToVec3(), shadowVerts[ v2 ].ToVec3() ) ) 
		{
			shadowIndexes[numShadowIndexes++] = v1;
			shadowIndexes[numShadowIndexes++] = v2;
			shadowIndexes[numShadowIndexes++] = v1_back;
			shadowIndexes[numShadowIndexes++] = v2;
			shadowIndexes[numShadowIndexes++] = v2_back;
			shadowIndexes[numShadowIndexes++] = v1_back;
		} 
		else 
		{
			shadowIndexes[numShadowIndexes++] = v1;
			shadowIndexes[numShadowIndexes++] = v2;
			shadowIndexes[numShadowIndexes++] = v2_back;
			shadowIndexes[numShadowIndexes++] = v1;
			shadowIndexes[numShadowIndexes++] = v2_back;
			shadowIndexes[numShadowIndexes++] = v1_back;
		}
	}
}

/*
===================
ClipLineToLight

If neither point is clearly behind the clipping
plane, the edge will be passed unmodified.  A sil edge that
is on a border plane must be drawn.

If one point is clearly clipped by the plane and the
other point is on the plane, it will be completely removed.
===================
*/
static bool ClipLineToLight(	const idVec3 &a, const idVec3 &b, const idPlane frustum[6], idVec3 &p1, idVec3 &p2 ) 
{
	float	*clip;
	int		j;
	float	d1, d2;
	float	f;

	p1 = a;
	p2 = b;

	// clip it
	for ( j = 0 ; j < 6 ; j++ ) 
	{
		d1 = frustum[j].Distance( p1 );
		d2 = frustum[j].Distance( p2 );

		// if both on or in front, not clipped to this plane
		if ( d1 > -LIGHT_CLIP_EPSILON && d2 > -LIGHT_CLIP_EPSILON ) 
			continue;

		// if one is behind and the other isn't clearly in front, the edge is clipped off
		if ( d1 <= -LIGHT_CLIP_EPSILON && d2 < LIGHT_CLIP_EPSILON ) 
			return false;
		
		if ( d2 <= -LIGHT_CLIP_EPSILON && d1 < LIGHT_CLIP_EPSILON )
			return false;

		// clip it, keeping the negative side
		if ( d1 < 0 )
			clip = p1.ToFloatPtr();
		else 
			clip = p2.ToFloatPtr();

#if 0
		if ( idMath::Fabs(d1 - d2) < 0.001 )
			d2 = d1 - 0.1;
#endif

		f = d1 / ( d1 - d2 );
		clip[0] = p1[0] + f * ( p2[0] - p1[0] );
		clip[1] = p1[1] + f * ( p2[1] - p1[1] );
		clip[2] = p1[2] + f * ( p2[2] - p1[2] );
	}

	return true;	// retain a fragment
}

/*
=================
AddSilEdges

Add quads from the front points to the projected points
for each silhouette edge in the light
=================
*/
static void AddSilEdges( const srfTriangles_t *tri, unsigned short *pointCull, const idPlane frustum[6] ) 
{
	int		v1, v2;
	int		i;
	silEdge_t	*sil;
	int		numPlanes;

	numPlanes = tri->numIndexes / 3;

	// add sil edges for any true silhouette boundaries on the surface
	for ( i = 0 ; i < tri->numSilEdges ; i++ ) {
		sil = tri->silEdges + i;
		if ( sil->p1 < 0 || sil->p1 > numPlanes || sil->p2 < 0 || sil->p2 > numPlanes ) {
			common->Error( "Bad sil planes" );
		}

		// an edge will be a silhouette edge if the face on one side
		// casts a shadow, but the face on the other side doesn't.
		// "casts a shadow" means that it has some surface in the projection,
		// not just that it has the correct facing direction
		// This will cause edges that are exactly on the frustum plane
		// to be considered sil edges if the face inside casts a shadow.
		if ( !( faceCastsShadow[ sil->p1 ] ^ faceCastsShadow[ sil->p2 ] ) ) {
			continue;
		}

		// if the edge is completely off the negative side of
		// a frustum plane, don't add it at all.  This can still
		// happen even if the face is visible and casting a shadow
		// if it is partially clipped
		if ( EDGE_CULLED( sil->v1, sil->v2 ) ) {
			continue;
		}

		// see if the edge needs to be clipped
		if ( EDGE_CLIPPED( sil->v1, sil->v2 ) ) {
			if ( numShadowVerts + 4 > MAX_SHADOW_VERTS ) {
				overflowed = true;
				return;
			}
			v1 = numShadowVerts;
			v2 = v1 + 2;
			if ( !ClipLineToLight( tri->verts[ sil->v1 ].xyz, tri->verts[ sil->v2 ].xyz,
				frustum, shadowVerts[v1].ToVec3(), shadowVerts[v2].ToVec3() ) ) {
				continue;	// clipped away
			}

			numShadowVerts += 4;
		} 
		else 
		{
			// use the entire edge
			v1 = remap[ sil->v1 ];
			v2 = remap[ sil->v2 ];
			if ( v1 < 0 || v2 < 0 ) 
			{
				common->Error( "AddSilEdges: bad remap[]" );
			}
		}

		// don't overflow
		if ( numShadowIndexes + 6 > MAX_SHADOW_INDEXES ) 
		{
			overflowed = true;
			return;
		}

		// we need to choose the correct way of triangulating the silhouette quad
		// consistantly between any two points, no matter which order they are specified.
		// If this wasn't done, slight rasterization cracks would show in the shadow
		// volume when two sil edges were exactly coincident
		if ( faceCastsShadow[ sil->p2 ] ) 
		{
			if ( PointsOrdered( shadowVerts[ v1 ].ToVec3(), shadowVerts[ v2 ].ToVec3() ) ) 
			{
				shadowIndexes[numShadowIndexes++] = v1;
				shadowIndexes[numShadowIndexes++] = v1+1;
				shadowIndexes[numShadowIndexes++] = v2;
				shadowIndexes[numShadowIndexes++] = v2;
				shadowIndexes[numShadowIndexes++] = v1+1;
				shadowIndexes[numShadowIndexes++] = v2+1;
			} 
			else 
			{
				shadowIndexes[numShadowIndexes++] = v1;
				shadowIndexes[numShadowIndexes++] = v2+1;
				shadowIndexes[numShadowIndexes++] = v2;
				shadowIndexes[numShadowIndexes++] = v1;
				shadowIndexes[numShadowIndexes++] = v1+1;
				shadowIndexes[numShadowIndexes++] = v2+1;
			}
		} 
		else 
		{
			if ( PointsOrdered( shadowVerts[ v1 ].ToVec3(), shadowVerts[ v2 ].ToVec3() ) ) 
			{
				shadowIndexes[numShadowIndexes++] = v1;
				shadowIndexes[numShadowIndexes++] = v2;
				shadowIndexes[numShadowIndexes++] = v1+1;
				shadowIndexes[numShadowIndexes++] = v2;
				shadowIndexes[numShadowIndexes++] = v2+1;
				shadowIndexes[numShadowIndexes++] = v1+1;
			} 
			else 
			{
				shadowIndexes[numShadowIndexes++] = v1;
				shadowIndexes[numShadowIndexes++] = v2;
				shadowIndexes[numShadowIndexes++] = v2+1;
				shadowIndexes[numShadowIndexes++] = v1;
				shadowIndexes[numShadowIndexes++] = v2+1;
				shadowIndexes[numShadowIndexes++] = v1+1;
			}
		}
	}
}

/*
===================
ProjectPointsToFarPlane

make a projected copy of the even verts into the odd spots
that is on the far light clip plane
===================
*/
static void ProjectPointsToFarPlane( const idRenderEntityLocal *ent, const idRenderLightLocal *light, const idPlane &lightPlaneLocal, int firstShadowVert, int numShadowVerts ) 
{
	idVec3		lv;
	idVec4		mat[4];
	int			i;
	idVec4		*in;

	R_GlobalPointToLocal( ent->modelMatrix, light->globalLightOrigin, lv );
	LightProjectionMatrix( lv, lightPlaneLocal, mat );

#if 1
	// make a projected copy of the even verts into the odd spots
	in = &shadowVerts[firstShadowVert];
	for ( i = firstShadowVert ; i < numShadowVerts ; i+= 2, in += 2 ) {
		float	w, oow;

		in[0].w = 1;

		w = in->ToVec3() * mat[3].ToVec3() + mat[3][3];
		if ( w == 0 ) {
			in[1] = in[0];
			continue;
		}

		oow = 1.0 / w;
		in[1].x = ( in->ToVec3() * mat[0].ToVec3() + mat[0][3] ) * oow;
		in[1].y = ( in->ToVec3() * mat[1].ToVec3() + mat[1][3] ) * oow;
		in[1].z = ( in->ToVec3() * mat[2].ToVec3() + mat[2][3] ) * oow;
		in[1].w = 1;
	}

#else
	// messing with W seems to cause some depth precision problems

	// make a projected copy of the even verts into the odd spots
	in = &shadowVerts[firstShadowVert];
	for ( i = firstShadowVert ; i < numShadowVerts ; i+= 2, in += 2 ) {
		in[0].w = 1;
		in[1].x = *in * mat[0].ToVec3() + mat[0][3];
		in[1].y = *in * mat[1].ToVec3() + mat[1][3];
		in[1].z = *in * mat[2].ToVec3() + mat[2][3];
		in[1].w = *in * mat[3].ToVec3() + mat[3][3];
	}
#endif
}

/*
=================
CreateShadowVolumeInFrustum

Adds new verts and indexes to the shadow volume.

If the frustum completely defines the projected light,
makeClippedPlanes should be true, which will cause sil quads to
be added along all clipped edges.

If the frustum is just part of a point light, clipped planes don't
need to be added.
=================
*/
static void CreateShadowVolumeInFrustum( const idRenderEntityLocal *ent,
										  const srfTriangles_t *tri,
										  const idRenderLightLocal *light,
										  const idVec3 lightOrigin,
										  const idPlane frustum[6],
										  const idPlane &farPlane,
										  bool makeClippedPlanes ) 
										  {
	int		i;
	int		numTris;
	unsigned short		*pointCull;
	int		numCapIndexes;
	int		firstShadowIndex;
	int		firstShadowVert;
	int		cullBits;

	pointCull = (unsigned short *)_alloca16( tri->numVerts * sizeof( pointCull[0] ) );

	// test the vertexes for inside the light frustum, which will allow
	// us to completely cull away some triangles from consideration.
	CalcPointCull( tri, frustum, pointCull );

	// this may not be the first frustum added to the volume
	firstShadowIndex = numShadowIndexes;
	firstShadowVert = numShadowVerts;

	// decide which triangles front shadow volumes, clipping as needed
	numClipSilEdges = 0;
	numTris = tri->numIndexes / 3;
	for ( i = 0 ; i < numTris ; i++ ) 
	{
		int		i1, i2, i3;

		faceCastsShadow[i] = 0;	// until shown otherwise

		// if it isn't facing the right way, don't add it
		// to the shadow volume
		if ( globalFacing[i] ) 
			continue;

		i1 = tri->silIndexes[ i*3 + 0 ];
		i2 = tri->silIndexes[ i*3 + 1 ];
		i3 = tri->silIndexes[ i*3 + 2 ];

		// if all the verts are off one side of the frustum,
		// don't add any of them
		if ( TRIANGLE_CULLED( i1, i2, i3 ) ) 
			continue;

		// make sure the verts that are not on the negative sides
		// of the frustum are copied over.
		// we need to get the original verts even from clipped triangles
		// so the edges reference correctly, because an edge may be unclipped
		// even when a triangle is clipped.
		if ( numShadowVerts + 6 > MAX_SHADOW_VERTS ) 
		{
			overflowed = true;
			return;
		}

		if ( !POINT_CULLED(i1) && remap[i1] == -1 ) {

			remap[i1] = numShadowVerts;
			shadowVerts[ numShadowVerts ].ToVec3() = tri->verts[i1].xyz;
			numShadowVerts+=2;
		}

		if ( !POINT_CULLED(i2) && remap[i2] == -1 ) 
		{
			remap[i2] = numShadowVerts;
			shadowVerts[ numShadowVerts ].ToVec3() = tri->verts[i2].xyz;
			numShadowVerts+=2;
		}

		if ( !POINT_CULLED(i3) && remap[i3] == -1 ) 
		{
			remap[i3] = numShadowVerts;
			shadowVerts[ numShadowVerts ].ToVec3() = tri->verts[i3].xyz;
			numShadowVerts+=2;
		}

		// clip the triangle if any points are on the negative sides
		if ( TRIANGLE_CLIPPED( i1, i2, i3 ) ) 
		{
			cullBits = ( ( pointCull[ i1 ] ^ 0xfc0 ) | ( pointCull[ i2 ] ^ 0xfc0 ) | ( pointCull[ i3 ] ^ 0xfc0 ) ) >> 6;
			// this will also define clip edges that will become
			// silhouette planes
			if ( ClipTriangleToLight( tri->verts[i1].xyz, tri->verts[i2].xyz,
				tri->verts[i3].xyz, cullBits, frustum ) ) {
				faceCastsShadow[i] = 1;
			}
		} 
		else 
		{
			// instead of overflowing or drawing a streamer shadow, don't draw a shadow at all
			if ( numShadowIndexes + 3 > MAX_SHADOW_INDEXES ) 
			{
				overflowed = true;
				return;
			}
			
			if ( remap[i1] == -1 || remap[i2] == -1 || remap[i3] == -1 ) 
			{
				common->Error( "CreateShadowVolumeInFrustum: bad remap[]" );
			}
			shadowIndexes[numShadowIndexes++] = remap[i3];
			shadowIndexes[numShadowIndexes++] = remap[i2];
			shadowIndexes[numShadowIndexes++] = remap[i1];
			faceCastsShadow[i] = 1;
		}
	}

	// add indexes for the back caps, which will just be reversals of the
	// front caps using the back vertexes
	numCapIndexes = numShadowIndexes - firstShadowIndex;

	// if no faces have been defined for the shadow volume,
	// there won't be anything at all
	if ( numCapIndexes == 0 ) {
		return;
	}

	//--------------- off-line processing ------------------

	// if we are running from dmap, perform the (very) expensive shadow optimizations
	// to remove internal sil edges and optimize the caps
	if ( callOptimizer ) 
	{
		optimizedShadow_t opt;

		// project all of the vertexes to the shadow plane, generating
		// an equal number of back vertexes
		//ProjectPointsToFarPlane( ent, light, farPlane, firstShadowVert, numShadowVerts );

		opt = SuperOptimizeOccluders( shadowVerts, shadowIndexes + firstShadowIndex, numCapIndexes, farPlane, lightOrigin );

		// pull off the non-optimized data
		numShadowIndexes = firstShadowIndex;
		numShadowVerts = firstShadowVert;

		// add the optimized data
		if ( numShadowIndexes + opt.totalIndexes > MAX_SHADOW_INDEXES
			|| numShadowVerts + opt.numVerts > MAX_SHADOW_VERTS ) {
			overflowed = true;
			common->Printf( "WARNING: overflowed MAX_SHADOW tables, shadow discarded\n" );
			Mem_Free( opt.verts );
			Mem_Free( opt.indexes );
			return;
		}

		for ( i = 0 ; i < opt.numVerts ; i++ ) {
			shadowVerts[numShadowVerts+i][0] = opt.verts[i][0];
			shadowVerts[numShadowVerts+i][1] = opt.verts[i][1];
			shadowVerts[numShadowVerts+i][2] = opt.verts[i][2];
			shadowVerts[numShadowVerts+i][3] = 1;
		}
		for ( i = 0 ; i < opt.totalIndexes ; i++ ) {
			int	index = opt.indexes[i];
			if ( index < 0 || index > opt.numVerts ) {
				common->Error( "optimized shadow index out of range" );
			}
			shadowIndexes[numShadowIndexes+i] = index + numShadowVerts;
		}

		numShadowVerts += opt.numVerts;
		numShadowIndexes += opt.totalIndexes;

		// note the index distribution so we can sort all the caps after all the sils
		indexRef[indexFrustumNumber].frontCapStart = firstShadowIndex;
		indexRef[indexFrustumNumber].rearCapStart = firstShadowIndex+opt.numFrontCapIndexes;
		indexRef[indexFrustumNumber].silStart = firstShadowIndex+opt.numFrontCapIndexes+opt.numRearCapIndexes;
		indexRef[indexFrustumNumber].end = numShadowIndexes;
		indexFrustumNumber++;

		Mem_Free( opt.verts );
		Mem_Free( opt.indexes );
		return;
	}

	//--------------- real-time processing ------------------

	// the dangling edge "face" is never considered to cast a shadow,
	// so any face with dangling edges that casts a shadow will have
	// it's dangling sil edge trigger a sil plane
	faceCastsShadow[numTris] = 0;

	// instead of overflowing or drawing a streamer shadow, don't draw a shadow at all
	// if we ran out of space
	if ( numShadowIndexes + numCapIndexes > MAX_SHADOW_INDEXES ) 
	{
		overflowed = true;
		return;
	}

	for ( i = 0 ; i < numCapIndexes ; i += 3 ) 
	{
		shadowIndexes[ numShadowIndexes + i + 0 ] = shadowIndexes[ firstShadowIndex + i + 2 ] + 1;
		shadowIndexes[ numShadowIndexes + i + 1 ] = shadowIndexes[ firstShadowIndex + i + 1 ] + 1;
		shadowIndexes[ numShadowIndexes + i + 2 ] = shadowIndexes[ firstShadowIndex + i + 0 ] + 1;
	}

	numShadowIndexes += numCapIndexes;

	c_caps += numCapIndexes * 2;

	int preSilIndexes = numShadowIndexes;

	// if any triangles were clipped, we will have a list of edges
	// on the frustum which must now become sil edges
	if ( makeClippedPlanes ) 
		AddClipSilEdges();

	// any edges that are a transition between a shadowing and
	// non-shadowing triangle will cast a silhouette edge
	AddSilEdges( tri, pointCull, frustum );

	c_sils += numShadowIndexes - preSilIndexes;

	// project all of the vertexes to the shadow plane, generating
	// an equal number of back vertexes
	ProjectPointsToFarPlane( ent, light, farPlane, firstShadowVert, numShadowVerts );

	// note the index distribution so we can sort all the caps after all the sils
	indexRef[indexFrustumNumber].frontCapStart = firstShadowIndex;
	indexRef[indexFrustumNumber].rearCapStart = firstShadowIndex+numCapIndexes;
	indexRef[indexFrustumNumber].silStart = preSilIndexes;
	indexRef[indexFrustumNumber].end = numShadowIndexes;
	indexFrustumNumber++;
}

/*
================
CalcInteractionFacing

Determines which triangles of the surface are facing towards the light origin.

The facing array should be allocated with one extra index than
the number of surface triangles, which will be used to handle dangling
edge silhouettes.
================
*/
void CalcInteractionFacing( const idRenderEntityLocal *ent, const srfTriangles_t *tri, const idRenderLightLocal *light, srfCullInfo_t &cullInfo ) 
{
	idVec3 localLightOrigin;

	if ( cullInfo.facing != nullptr ) 
		return;

	R_GlobalPointToLocal( ent->modelMatrix, light->globalLightOrigin, localLightOrigin );

	int numFaces = tri->numIndexes / 3;

	if ( !tri->facePlanes || !tri->facePlanesCalculated ) {
		R_DeriveFacePlanes( const_cast<srfTriangles_t *>(tri) );
	}

	cullInfo.facing = (byte *) R_StaticAlloc( ( numFaces + 1 ) * sizeof( cullInfo.facing[0] ) );

	// calculate back face culling
	float *planeSide = (float *) _alloca16( numFaces * sizeof( float ) );

	// exact geometric cull against face
	SIMDProcessor->Dot( planeSide, localLightOrigin, tri->facePlanes, numFaces );
	SIMDProcessor->CmpGE( cullInfo.facing, planeSide, 0.0f, numFaces );

	cullInfo.facing[ numFaces ] = 1;	// for dangling edges to reference
}


/*
=================
CreateShadowVolume

The returned surface will have a valid bounds and radius for culling.

Triangles are clipped to the light frustum before projecting.

A single triangle can clip to as many as 7 vertexes, so
the worst case expansion is 2*(numindexes/3)*7 verts when counting both
the front and back caps, although it will usually only be a modest
increase in vertexes for closed modesl

The worst case index count is much larger, when the 7 vertex clipped triangle
needs 15 indexes for the front, 15 for the back, and 42 (a quad on seven sides)
for the sides, for a total of 72 indexes from the original 3.  Ouch.

NULL may be returned if the surface doesn't create a shadow volume at all,
as with a single face that the light is behind.

If an edge is within an epsilon of the border of the volume, it must be treated
as if it is clipped for triangles, generating a new sil edge, and act
as if it was culled for edges, because the sil edge will have been
generated by the triangle irregardless of if it actually was a sil edge.
=================
*/
static srfTriangles_t * CreateShadowVolume( const idRenderEntityLocal *ent, const srfTriangles_t *tri, const idRenderLightLocal *light, shadowGen_t optimize, srfCullInfo_t &cullInfo ) 
{
	int		i, j;
	idVec3	lightOrigin;
	srfTriangles_t	*newTri;
	int		capPlaneBits;

#if 0
	if ( !r_shadows.GetBool() ) 
		return nullptr;
#endif

	if ( tri->numSilEdges == 0 || tri->numIndexes == 0 || tri->numVerts == 0 ) 
		return nullptr;

	if ( tri->numIndexes < 0 ) 
		common->Error( "CreateShadowVolume: tri->numIndexes = %i", tri->numIndexes );

	if ( tri->numVerts < 0 ) 
		common->Error( "CreateShadowVolume: tri->numVerts = %i", tri->numVerts );

	//tr.pc.c_createShadowVolumes++;

#if 0
	// use the fast infinite projection in dynamic situations, which
	// trades somewhat more overdraw and no cap optimizations for
	// a very simple generation process
	if ( optimize == SG_DYNAMIC && r_useTurboShadow.GetBool() ) 
	{
		if ( tr.backEndRendererHasVertexPrograms && r_useShadowVertexProgram.GetBool() )
			return R_CreateVertexProgramTurboShadowVolume( ent, tri, light, cullInfo );
		else 
			return R_CreateTurboShadowVolume( ent, tri, light, cullInfo );
	}
#endif

	CalcInteractionFacing( ent, tri, light, cullInfo );

	int numFaces = tri->numIndexes / 3;
	int allFront = 1;
	for ( i = 0; i < numFaces && allFront; i++ ) 
	{
		allFront &= cullInfo.facing[i];
	}

	if ( allFront ) 
		return nullptr; // if no faces are the right direction, don't make a shadow at all

	// clear the shadow volume
	numShadowIndexes = 0;
	numShadowVerts = 0;
	overflowed = false;
	indexFrustumNumber = 0;
	capPlaneBits = 0;
	callOptimizer = (optimize == SG_OFFLINE);

	// the facing information will be the same for all six projections
	// from a point light, as well as for any directed lights
	globalFacing = cullInfo.facing;
	faceCastsShadow = (byte *)_alloca16( tri->numIndexes / 3 + 1 );	// + 1 for fake dangling edge face
	remap = (int *)_alloca16( tri->numVerts * sizeof( remap[0] ) );

	R_GlobalPointToLocal( ent->modelMatrix, light->globalLightOrigin, lightOrigin );

	// run through all the shadow frustums, which is one for a projected light,
	// and usually six for a point light, but point lights with centers outside
	// the box may have less
	for ( int frustumNum = 0 ; frustumNum < light->numShadowFrustums ; frustumNum++ ) 
	{
		const shadowFrustum_t	*frust = &light->shadowFrustums[frustumNum];
		ALIGN16( idPlane frustum[6] );

		// transform the planes into entity space
		// we could share and reverse some of the planes between frustums for a minor
		// speed increase

		// the cull test is redundant for a single shadow frustum projected light, because
		// the surface has already been checked against the main light frustums

		for ( j = 0 ; j < frust->numPlanes ; j++ ) 
		{
			R_GlobalPlaneToLocal( ent->modelMatrix, frust->planes[j], frustum[j] );

			// try to cull the entire surface against this frustum
			float d = tri->bounds.PlaneDistance( frustum[j] );
			if ( d < -LIGHT_CLIP_EPSILON )
				break;
		}
		
		if ( j != frust->numPlanes ) 
			continue;
		
		// we need to check all the triangles
		int oldFrustumNumber = indexFrustumNumber;

		CreateShadowVolumeInFrustum( ent, tri, light, lightOrigin, frustum, frustum[5], frust->makeClippedPlanes );

		// if we couldn't make a complete shadow volume, it is better to
		// not draw one at all, avoiding streamer problems
		if ( overflowed )
			return nullptr;

		// note that we have caps projected against this frustum,
		// which may allow us to skip drawing the caps if all projected
		// planes face away from the viewer and the viewer is outside the light volume
		if ( indexFrustumNumber != oldFrustumNumber ) 
			capPlaneBits |= 1<<frustumNum;
		
	}

	// if no faces have been defined for the shadow volume,
	// there won't be anything at all
	if ( numShadowIndexes == 0 ) 
		return nullptr;

	// this should have been prevented by the overflowed flag, so if it ever happens,
	// it is a code error
	if ( numShadowVerts > MAX_SHADOW_VERTS || numShadowIndexes > MAX_SHADOW_INDEXES )
		common->FatalError( "Shadow volume exceeded allocation" );

	// allocate a new surface for the shadow volume
	newTri = R_AllocStaticTriSurf();

	// we might consider setting this, but it would only help for
	// large lights that are partially off screen
	newTri->bounds.Clear();

	// copy off the verts and indexes
	newTri->numVerts = numShadowVerts;
	newTri->numIndexes = numShadowIndexes;

	// the shadow verts will go into a main memory buffer as well as a vertex
	// cache buffer, so they can be copied back if they are purged
#if 0
	R_AllocStaticTriSurfShadowVerts( newTri, newTri->numVerts );
	std::memcpy( newTri->shadowVertexes, shadowVerts, newTri->numVerts * sizeof( newTri->shadowVertexes[0] ) );
#endif

	R_AllocStaticTriSurfIndexes( newTri, newTri->numIndexes );

	if ( 1 /* sortCapIndexes */ ) 
	{
		newTri->shadowCapPlaneBits = capPlaneBits;

		// copy the sil indexes first
		newTri->numShadowIndexesNoCaps = 0;
		for ( i = 0 ; i < indexFrustumNumber ; i++ ) 
		{
			int	c = indexRef[i].end - indexRef[i].silStart;
			std::memcpy( newTri->indexes+newTri->numShadowIndexesNoCaps, shadowIndexes+indexRef[i].silStart, c * sizeof( newTri->indexes[0] ) );
			newTri->numShadowIndexesNoCaps += c;
		}

		// copy rear cap indexes next
		newTri->numShadowIndexesNoFrontCaps = newTri->numShadowIndexesNoCaps;
		for ( i = 0 ; i < indexFrustumNumber ; i++ ) 
		{
			int	c = indexRef[i].silStart - indexRef[i].rearCapStart;
			std::memcpy( newTri->indexes+newTri->numShadowIndexesNoFrontCaps, shadowIndexes+indexRef[i].rearCapStart, c * sizeof( newTri->indexes[0] ) );
			newTri->numShadowIndexesNoFrontCaps += c;
		}

		// copy front cap indexes last
		newTri->numIndexes = newTri->numShadowIndexesNoFrontCaps;
		for ( i = 0 ; i < indexFrustumNumber ; i++ ) 
		{
			int	c = indexRef[i].rearCapStart - indexRef[i].frontCapStart;
			std::memcpy( newTri->indexes+newTri->numIndexes, shadowIndexes+indexRef[i].frontCapStart, c * sizeof( newTri->indexes[0] ) );
			newTri->numIndexes += c;
		}

	} 
	else 
	{
		newTri->shadowCapPlaneBits = 63;	// we don't have optimized index lists
		std::memcpy( newTri->indexes, shadowIndexes, newTri->numIndexes * sizeof( newTri->indexes[0] ) );
	}

	if ( optimize == SG_OFFLINE )
		CleanupOptimizedShadowTris( newTri );

	return newTri;
}

/*
========================
CreateLightShadow

This is called from dmap in util/surface.cpp
shadowerGroups should be exactly clipped to the light frustum before calling.
shadowerGroups is optimized by this function, but the contents can be freed, because the returned
lightShadow_t list is a further culling and optimization of the data.
========================
*/
srfTriangles_t *CreateLightShadow( optimizeGroup_t *shadowerGroups, const mapLight_t *light ) 
{

	common->Printf( "----- CreateLightShadow %p -----\n", light );

	// optimize all the groups
	OptimizeGroupList( shadowerGroups );

	// combine all the triangles into one list
	mapTri_t	*combined;

	combined = NULL;
	for ( optimizeGroup_t *group = shadowerGroups ; group ; group = group->nextGroup ) {
		combined = MergeTriLists( combined, CopyTriList( group->triList ) );
	}

	if ( !combined ) {
		return NULL;
	}

	// find uniqued vertexes
	srfTriangles_t	*occluders = ShareMapTriVerts( combined );

	FreeTriList( combined );

	// find silhouette information for the triSurf
	R_CleanupTriangles( occluders, false, true, false );

	// let the renderer build the shadow volume normally
	idRenderEntityLocal space;

	space.modelMatrix[0] = 1;
	space.modelMatrix[5] = 1;
	space.modelMatrix[10] = 1;
	space.modelMatrix[15] = 1;

	srfCullInfo_t cullInfo;
	std::memset( &cullInfo, 0, sizeof( cullInfo ) );

	// call the normal shadow creation, but with the superOptimize flag set, which will
	// call back to SuperOptimizeOccluders after clipping the triangles to each frustum
	srfTriangles_t	*shadowTris;
	if ( dmapGlobals.shadowOptLevel == SO_MERGE_SURFACES )
		shadowTris = CreateShadowVolume( &space, occluders, &light->def, SG_STATIC, cullInfo );
	else
		shadowTris = CreateShadowVolume( &space, occluders, &light->def, SG_OFFLINE, cullInfo );
	
	R_FreeStaticTriSurf( occluders );

	R_FreeInteractionCullInfo( cullInfo );

	if ( shadowTris ) 
	{
		dmapGlobals.totalShadowTriangles += shadowTris->numIndexes / 3;
		dmapGlobals.totalShadowVerts += shadowTris->numVerts / 3;
	}

	return shadowTris;
}
