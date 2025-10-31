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

#include "dmap.h"

/*

  After parsing, there will be a list of entities that each has
  a list of primitives.
  
  Primitives are either brushes, triangle soups, or model references.

  Curves are tesselated to triangle soups at load time, but model
  references are 
  Brushes will have 
  
	brushes, each of which has a side definition.

*/

//
// private declarations
//
constexpr int	MAX_BUILD_SIDES = 300;
constexpr int	MAX_POLYTOPE_PLANES = 6;
constexpr float NORMAL_EPSILON = 0.00001f;
constexpr float DIST_EPSILON = 0.01f;

static	int		entityPrimitive;		// to track editor brush numbers
static	int		c_numMapPatches;
static	int		c_areaportals;

static	uEntity_t	*uEntity;

// brushes are parsed into a temporary array of sides,
// which will have duplicates removed before the final brush is allocated
static	uBrush_t	*buildBrush;

/*
===========
FindFloatPlane
===========
*/
int FindFloatPlane( const idPlane &plane, bool *fixedDegeneracies ) {
	idPlane p = plane;
	bool fixed = p.FixDegeneracies( DIST_EPSILON );
	if ( fixed && fixedDegeneracies ) {
		*fixedDegeneracies = true;
	}
	return dmapGlobals.mapPlanes.FindPlane( p, NORMAL_EPSILON, DIST_EPSILON );
}

/*
===========
SetBrushContents

The contents on all sides of a brush should be the same
Sets contentsShader, contents, opaque
===========
*/
static void SetBrushContents( uBrush_t *b ) {
	int			contents, c2;
	side_t		*s;
	int			i;
	bool	mixed;

	s = &b->sides[0];
	contents = s->material->GetContentFlags();

	b->contentShader = s->material;
	mixed = false;

	// a brush is only opaque if all sides are opaque
	b->opaque = true;

	for ( i=1 ; i<b->numsides ; i++, s++ ) {
		s = &b->sides[i];

		if ( !s->material ) {
			continue;
		}

		c2 = s->material->GetContentFlags();
		if (c2 != contents) {
			mixed = true;
			contents |= c2;
		}

		if ( s->material->Coverage() != MC_OPAQUE ) {
			b->opaque = false;
		}
	}

	if ( contents & CONTENTS_AREAPORTAL ) {
		c_areaportals++;
	}

	b->contents = contents;
}


//============================================================================

/*
===============
FreeBuildBrush
===============
*/
static void FreeBuildBrush( void ) {
	int		i;

	for ( i = 0 ; i < buildBrush->numsides ; i++ ) {
		if ( buildBrush->sides[i].winding ) {
			delete buildBrush->sides[i].winding;
		}
	}
	buildBrush->numsides = 0;
}

/*
===============
FinishBrush

Produces a final brush based on the buildBrush->sides array
and links it to the current entity
===============
*/
static uBrush_t *FinishBrush( void ) {
	uBrush_t	*b;
	primitive_t	*prim;

	// create windings for sides and bounds for brush
	if ( !CreateBrushWindings( buildBrush ) ) {
		// don't keep this brush
		FreeBuildBrush();
		return NULL;
	}

	if ( buildBrush->contents & CONTENTS_AREAPORTAL ) {
		if (dmapGlobals.num_entities != 1) {
			common->Printf("Entity %i, Brush %i: areaportals only allowed in world\n"
				,  dmapGlobals.num_entities - 1, entityPrimitive);
			FreeBuildBrush();
			return NULL;
		}
	}

	// keep it
	b = CopyBrush( buildBrush );

	FreeBuildBrush();

	b->entitynum = dmapGlobals.num_entities-1;
	b->brushnum = entityPrimitive;

	b->original = b;

	prim = (primitive_t *)Mem_Alloc( sizeof( *prim ), TAG_DMAP );
	memset( prim, 0, sizeof( *prim ) );
	prim->next = uEntity->primitives;
	uEntity->primitives = prim;

	prim->brush = b;

	return b;
}

/*
=================
RemoveDuplicateBrushPlanes

Returns false if the brush has a mirrored set of planes,
meaning it encloses no volume.
Also removes planes without any normal
=================
*/
static bool RemoveDuplicateBrushPlanes( uBrush_t * b ) 
{
	int			i, j, k;
	side_t		*sides;

	sides = b->sides;

	for ( i = 1 ; i < b->numsides ; i++ ) {

		// check for a degenerate plane
		if ( sides[i].planenum == -1) {
			common->Printf("Entity %i, Brush %i: degenerate plane\n"
				, b->entitynum, b->brushnum);
			// remove it
			for ( k = i + 1 ; k < b->numsides ; k++ ) {
				sides[k-1] = sides[k];
			}
			b->numsides--;
			i--;
			continue;
		}

		// check for duplication and mirroring
		for ( j = 0 ; j < i ; j++ ) {
			if ( sides[i].planenum == sides[j].planenum ) {
				common->Printf("Entity %i, Brush %i: duplicate plane\n"
					, b->entitynum, b->brushnum);
				// remove the second duplicate
				for ( k = i + 1 ; k < b->numsides ; k++ ) {
					sides[k-1] = sides[k];
				}
				b->numsides--;
				i--;
				break;
			}

			if ( sides[i].planenum == (sides[j].planenum ^ 1) ) {
				// mirror plane, brush is invalid
				common->Printf("Entity %i, Brush %i: mirrored plane\n"
					, b->entitynum, b->brushnum);
				return false;
			}
		}
	}
	return true;
}


/*
=================
ParseBrush
=================
*/
static void ParseBrush( const idMapBrush *mapBrush, int primitiveNum ) 
{
	uBrush_t	*b = nullptr;
	side_t		*s = nullptr;
	const idMapBrushSide	*ms;
	int			i;
	bool		fixedDegeneracies = false;

	buildBrush->entitynum = dmapGlobals.num_entities-1;
	buildBrush->brushnum = entityPrimitive;
	buildBrush->numsides = mapBrush->GetNumSides();
	for ( i = 0 ; i < mapBrush->GetNumSides() ; i++ ) {
		s = &buildBrush->sides[i];
		ms = mapBrush->GetSide(i);

		memset( s, 0, sizeof( *s ) );
		s->planenum = FindFloatPlane( ms->GetPlane(), &fixedDegeneracies );
		s->material = declManager->FindMaterial( ms->GetMaterial() );
		ms->GetTextureVectors( s->texVec.v );
		// remove any integral shift, which will help with grouping
		s->texVec.v[0][3] -= floor( s->texVec.v[0][3] );
		s->texVec.v[1][3] -= floor( s->texVec.v[1][3] );
	}

	// if there are mirrored planes, the entire brush is invalid
	if ( !RemoveDuplicateBrushPlanes( buildBrush ) ) {
		return;
	}

	// get the content for the entire brush
	SetBrushContents( buildBrush );

	b = FinishBrush();
	if ( !b ) {
		return;
	}

	if ( fixedDegeneracies && dmapGlobals.verboseentities ) {
		common->Warning( "brush %d has degenerate plane equations", primitiveNum );
	}
}

/*
================
ParseSurface
================
*/
static void ParseSurface(const idMapPatch *patch, const idSurface *surface, const idMaterial *material) 
{
	int				i;
	mapTri_t		*tri;
	primitive_t		*prim;

	prim = (primitive_t *)Mem_Alloc( sizeof( *prim ), TAG_DMAP );
	memset( prim, 0, sizeof( *prim ) );
	prim->next = uEntity->primitives;
	uEntity->primitives = prim;

	for ( i = 0; i < surface->GetNumIndexes(); i += 3 ) 
	{
		tri = AllocTri();
		tri->v[2] = (*surface)[surface->GetIndexes()[i+0]];
		tri->v[1] = (*surface)[surface->GetIndexes()[i+2]];
		tri->v[0] = (*surface)[surface->GetIndexes()[i+1]];
		tri->material = material;
		tri->next = prim->tris;
		prim->tris = tri;
	}

	// set merge groups if needed, to prevent multiple sides from being
	// merged into a single surface in the case of gui shaders, mirrors, and autosprites
	if ( material->IsDiscrete() ) {
		for ( tri = prim->tris ; tri ; tri = tri->next ) {
			tri->mergeGroup = (void *)patch;
		}
	}
}

/*
================
ParsePatch
================
*/
static void ParsePatch(const idMapPatch *patch, int primitiveNum) {
	const idMaterial *mat;

	if ( dmapGlobals.noCurves ) {
		return;
	}

	c_numMapPatches++;

	mat = declManager->FindMaterial( patch->GetMaterial() );

	idSurface_Patch *cp = new idSurface_Patch( *patch );

	if ( patch->GetExplicitlySubdivided() ) {
		cp->SubdivideExplicit( patch->GetHorzSubdivisions(), patch->GetVertSubdivisions(), true );
	} else {
		cp->Subdivide( DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_LENGTH, true );
	}

	ParseSurface( patch, cp, mat );

	delete cp;
}

/*
================
ProcessMapEntity
================
*/
static bool	ProcessMapEntity( idMapEntity *mapEnt ) 
{
	idMapPrimitive	*prim;

	uEntity = &dmapGlobals.uEntities[dmapGlobals.num_entities];
	memset( uEntity, 0, sizeof(*uEntity) );
	uEntity->mapEntity = mapEnt;
	dmapGlobals.num_entities++;

	for ( entityPrimitive = 0; entityPrimitive < mapEnt->GetNumPrimitives(); entityPrimitive++ ) {
		prim = mapEnt->GetPrimitive(entityPrimitive);

		if ( prim->GetType() == idMapPrimitive::TYPE_BRUSH ) {
			ParseBrush( static_cast<idMapBrush*>(prim), entityPrimitive );
		}
		else if ( prim->GetType() == idMapPrimitive::TYPE_PATCH ) {
			ParsePatch( static_cast<idMapPatch*>(prim), entityPrimitive );
		}
	}

	// never put an origin on the world, even if the editor left one there
	if ( dmapGlobals.num_entities != 1 ) {
		uEntity->mapEntity->epairs.GetVector( "origin", "", uEntity->origin );
	}

	return true;
}

//===================================================================

// BEATO Begin:
/*
=====================
SetLightProject

All values are reletive to the origin
Assumes that right and up are not normalized
This is also called by dmap during map processing.
=====================
*/
static void SetLightProject( idPlane lightProject[4], const idVec3 origin, const idVec3 target, const idVec3 rightVector, const idVec3 upVector, const idVec3 start, const idVec3 stop ) 
{
	float		dist;
	float		scale;
	float		rLen, uLen;
	idVec3		normal;
	float		ofs;
	idVec3		right, up;
	idVec3		startGlobal;
	idVec4		targetGlobal;

	right = rightVector;
	rLen = right.Normalize();
	up = upVector;
	uLen = up.Normalize();
	normal = up.Cross( right );
//normal = right.Cross( up );
	normal.Normalize();

	dist = target * normal; //  - ( origin * normal );
	if ( dist < 0 ) 
	{
		dist = -dist;
		normal = -normal;
	}

	scale = ( 0.5f * dist ) / rLen;
	right *= scale;
	scale = -( 0.5f * dist ) / uLen;
	up *= scale;

	lightProject[2] = normal;
	lightProject[2][3] = -( origin * lightProject[2].Normal() );

	lightProject[0] = right;
	lightProject[0][3] = -( origin * lightProject[0].Normal() );

	lightProject[1] = up;
	lightProject[1][3] = -( origin * lightProject[1].Normal() );

	// now offset to center
	targetGlobal.ToVec3() = target + origin;
	targetGlobal[3] = 1;
	ofs = 0.5f - ( targetGlobal * lightProject[0].ToVec4() ) / ( targetGlobal * lightProject[2].ToVec4() );
	lightProject[0].ToVec4() += ofs * lightProject[2].ToVec4();
	ofs = 0.5f - ( targetGlobal * lightProject[1].ToVec4() ) / ( targetGlobal * lightProject[2].ToVec4() );
	lightProject[1].ToVec4() += ofs * lightProject[2].ToVec4();

	// set the falloff vector
	normal = stop - start;
	dist = normal.Normalize();
	if ( dist <= 0 )
		dist = 1;
	
	lightProject[3] = normal * ( 1.0f / dist );
	startGlobal = start + origin;
	lightProject[3][3] = -( startGlobal * lightProject[3].Normal() );
}


/*
===================
SetLightFrustum

Creates plane equations from the light projection, positive sides
face out of the light
===================
*/
static void SetLightFrustum( const idPlane lightProject[4], idPlane frustum[6] ) 
{
	int		i;

	// we want the planes of s=0, s=q, t=0, and t=q
	frustum[0] = lightProject[0];
	frustum[1] = lightProject[1];
	frustum[2] = lightProject[2] - lightProject[0];
	frustum[3] = lightProject[2] - lightProject[1];

	// we want the planes of s=0 and s=1 for front and rear clipping planes
	frustum[4] = lightProject[3];

	frustum[5] = lightProject[3];
	frustum[5][3] -= 1.0f;
	frustum[5] = -frustum[5];

	for ( i = 0 ; i < 6 ; i++ ) 
	{
		float	l;

		frustum[i] = -frustum[i];
		l = frustum[i].Normalize();
		frustum[i][3] /= l;
	}
}

/*
====================
R_FreeLightDefFrustum
====================
*/
static void FreeLightDefFrustum( idRenderLightLocal *ldef ) 
{
	int i;

	// free the frustum tris
	if ( ldef->frustumTris ) 
	{
		R_FreeStaticTriSurf( ldef->frustumTris );
		ldef->frustumTris = nullptr;
	}
	
	// free frustum windings
	for ( i = 0; i < 6; i++ ) 
	{
		if ( ldef->frustumWindings[i] ) 
		{
			delete ldef->frustumWindings[i];
			ldef->frustumWindings[i] = nullptr;
		}
	}
}

/*
===================
MakeShadowFrustums

Called at definition derivation time
===================
*/
static void MakeShadowFrustums( idRenderLightLocal *light ) 
{
	int		i = 0, j = 0;

	if ( light->parms.pointLight ) 
	{
#if 0
		idVec3	adjustedRadius;

		// increase the light radius to cover any origin offsets.
		// this will cause some shadows to extend out of the exact light
		// volume, but is simpler than adjusting all the frustums
		adjustedRadius[0] = light->parms.lightRadius[0] + idMath::Fabs( light->parms.lightCenter[0] );
		adjustedRadius[1] = light->parms.lightRadius[1] + idMath::Fabs( light->parms.lightCenter[1] );
		adjustedRadius[2] = light->parms.lightRadius[2] + idMath::Fabs( light->parms.lightCenter[2] );

		light->numShadowFrustums = 0;
		// a point light has to project against six planes
		for ( i = 0 ; i < 6 ; i++ ) {
			shadowFrustum_t	*frust = &light->shadowFrustums[ light->numShadowFrustums ];

			frust->numPlanes = 6;
			frust->makeClippedPlanes = false;
			for ( j = 0 ; j < 6 ; j++ ) {
				idPlane &plane = frust->planes[j];
				plane[0] = pointLightFrustums[i][j][0] / adjustedRadius[0];
				plane[1] = pointLightFrustums[i][j][1] / adjustedRadius[1];
				plane[2] = pointLightFrustums[i][j][2] / adjustedRadius[2];
				plane.Normalize();
				plane[3] = -( plane.Normal() * light->globalLightOrigin );
				if ( j == 5 ) {
					plane[3] += adjustedRadius[i>>1];
				}
			}

			light->numShadowFrustums++;
		}
#else
		// exact projection,taking into account asymetric frustums when 
		// globalLightOrigin isn't centered

		static int	faceCorners[6][4] = {
			{ 7, 5, 1, 3 },		// positive X side
			{ 4, 6, 2, 0 },		// negative X side
			{ 6, 7, 3, 2 },		// positive Y side
			{ 5, 4, 0, 1 },		// negative Y side
			{ 6, 4, 5, 7 },		// positive Z side
			{ 3, 1, 0, 2 }		// negative Z side
		};
		static int	faceEdgeAdjacent[6][4] = {
			{ 4, 4, 2, 2 },		// positive X side
			{ 7, 7, 1, 1 },		// negative X side
			{ 5, 5, 0, 0 },		// positive Y side
			{ 6, 6, 3, 3 },		// negative Y side
			{ 0, 0, 3, 3 },		// positive Z side
			{ 5, 5, 6, 6 }		// negative Z side
		};

		bool	centerOutside = false;

		// if the light center of projection is outside the light bounds,
		// we will need to build the planes a little differently
		if ( fabs( light->parms.lightCenter[0] ) > light->parms.lightRadius[0]
			|| fabs( light->parms.lightCenter[1] ) > light->parms.lightRadius[1]
			|| fabs( light->parms.lightCenter[2] ) > light->parms.lightRadius[2] ) {
			centerOutside = true;
		}

		// make the corners
		idVec3	corners[8];

		for ( i = 0 ; i < 8 ; i++ ) {
			idVec3	temp;
			for ( j = 0 ; j < 3 ; j++ ) {
				if ( i & ( 1 << j ) ) {
					temp[j] = light->parms.lightRadius[j];
				} else {
					temp[j] = -light->parms.lightRadius[j];
				}
			}

			// transform to global space
			corners[i] = light->parms.origin + light->parms.axis * temp;
		}

		light->numShadowFrustums = 0;
		for ( int side = 0 ; side < 6 ; side++ ) 
		{
			shadowFrustum_t	*frust = &light->shadowFrustums[ light->numShadowFrustums ];
			idVec3 &p1 = corners[faceCorners[side][0]];
			idVec3 &p2 = corners[faceCorners[side][1]];
			idVec3 &p3 = corners[faceCorners[side][2]];
			idPlane backPlane;

			// plane will have positive side inward
			backPlane.FromPoints( p1, p2, p3 );

			// if center of projection is on the wrong side, skip
			float d = backPlane.Distance( light->globalLightOrigin );
			if ( d < 0 ) {
				continue;
			}

			frust->numPlanes = 6;
			frust->planes[5] = backPlane;
			frust->planes[4] = backPlane;	// we don't really need the extra plane

			// make planes with positive side facing inwards in light local coordinates
			for ( int edge = 0 ; edge < 4 ; edge++ ) 
			{
				idVec3 &p1 = corners[faceCorners[side][edge]];
				idVec3 &p2 = corners[faceCorners[side][(edge+1)&3]];

				// create a plane that goes through the center of projection
				frust->planes[edge].FromPoints( p2, p1, light->globalLightOrigin );

				// see if we should use an adjacent plane instead
				if ( centerOutside ) {
					idVec3 &p3 = corners[faceEdgeAdjacent[side][edge]];
					idPlane sidePlane;

					sidePlane.FromPoints( p2, p1, p3 );
					d = sidePlane.Distance( light->globalLightOrigin );
					if ( d < 0 ) {
						// use this plane instead of the edged plane
						frust->planes[edge] = sidePlane;
					}
					// we can't guarantee a neighbor, so add sill planes at edge
					light->shadowFrustums[ light->numShadowFrustums ].makeClippedPlanes = true;
				}
			}
			light->numShadowFrustums++;
		}

#endif
		return;
	}
	
	// projected light

	light->numShadowFrustums = 1;
	shadowFrustum_t	*frust = &light->shadowFrustums[ 0 ];

	// flip and transform the frustum planes so the positive side faces
	// inward in local coordinates

	// it is important to clip against even the near clip plane, because
	// many projected lights that are faking area lights will have their
	// origin behind solid surfaces.
	for ( i = 0 ; i < 6 ; i++ ) 
	{
		idPlane &plane = frust->planes[i];

		plane.SetNormal( -light->frustum[i].Normal() );
		plane.SetDist( -light->frustum[i].Dist() );
	}
	
	frust->numPlanes = 6;

	frust->makeClippedPlanes = true;
	// projected lights don't have shared frustums, so any clipped edges
	// right on the planes must have a sil plane created for them
}

/*
=====================
static PolytopeSurface

Generate vertexes and indexes for a polytope,
and optionally returns the polygon windings.
The positive sides of the planes will be visible.
=====================
*/
static srfTriangles_t *PolytopeSurface( int numPlanes, const idPlane *planes, idWinding **windings ) 
{
	int i = 0, j = 0;
	srfTriangles_t *tri = nullptr;
	idFixedWinding planeWindings[MAX_POLYTOPE_PLANES];
	int numVerts = 0, numIndexes = 0;

	if ( numPlanes > MAX_POLYTOPE_PLANES ) 
	{
		common->Error( "R_PolytopeSurface: more than %d planes", MAX_POLYTOPE_PLANES );
	}

	numVerts = 0;
	numIndexes = 0;
	for ( i = 0; i < numPlanes; i++ ) 
	{
		const idPlane &plane = planes[i];
		idFixedWinding &w = planeWindings[i];

		w.BaseForPlane( plane );
		for ( j = 0; j < numPlanes; j++ ) 
		{
			const idPlane &plane2 = planes[j];
			if ( j == i ) 
				continue;
			
			if ( !w.ClipInPlace( -plane2, ON_EPSILON ) ) 
				break;
		}

		if ( w.GetNumPoints() <= 2 )
			continue;
		
		numVerts += w.GetNumPoints();
		numIndexes += ( w.GetNumPoints() - 2 ) * 3;
	}

	// allocate the surface
	tri = R_AllocStaticTriSurf();
	R_AllocStaticTriSurfVerts( tri, numVerts );
	R_AllocStaticTriSurfIndexes( tri, numIndexes );

	// copy the data from the windings
	for ( i = 0; i < numPlanes; i++ ) 
	{
		idFixedWinding &w = planeWindings[i];
		if ( !w.GetNumPoints() ) 
			continue;
		
		for ( j = 0 ; j < w.GetNumPoints() ; j++ ) 
		{
			tri->verts[tri->numVerts + j ].Clear();
			tri->verts[tri->numVerts + j ].xyz = w[j].ToVec3();
		}

		for ( j = 1 ; j < w.GetNumPoints() - 1 ; j++ ) 
		{
			tri->indexes[ tri->numIndexes + 0 ] = tri->numVerts;
			tri->indexes[ tri->numIndexes + 1 ] = tri->numVerts + j;
			tri->indexes[ tri->numIndexes + 2 ] = tri->numVerts + j + 1;
			tri->numIndexes += 3;
		}
		
		tri->numVerts += w.GetNumPoints();

		// optionally save the winding
		if ( windings ) 
		{
			windings[i] = new idWinding( w.GetNumPoints() );
			*windings[i] = w;
		}
	}

	R_BoundTriSurf( tri );

	return tri;
}

/*
=================
DeriveLightData
Fills everything in based on light->parms
=================
*/
static void DeriveLightData( idRenderLightLocal *light ) 
{
	int i = 0;

	// decide which light shader we are going to use
	if ( light->parms.shader ) 
		light->lightShader = light->parms.shader;
	
	if ( !light->lightShader ) 
	{
		if ( light->parms.pointLight ) 
			light->lightShader = declManager->FindMaterial( "lights/defaultPointLight" );
		else 
			light->lightShader = declManager->FindMaterial( "lights/defaultProjectedLight" );
	}

	// get the falloff image
	light->falloffImage = light->lightShader->LightFalloffImage();
	if ( !light->falloffImage ) 
	{
		// use the falloff from the default shader of the correct type
		const idMaterial	*defaultShader;

		if ( light->parms.pointLight ) 
		{
			defaultShader = declManager->FindMaterial( "lights/defaultPointLight" );
			light->falloffImage = defaultShader->LightFalloffImage();
		} 
		else 
		{
			// projected lights by default don't diminish with distance
			defaultShader = declManager->FindMaterial( "lights/defaultProjectedLight" );
			light->falloffImage = defaultShader->LightFalloffImage();
		}
	}

	// set the projection
	if ( !light->parms.pointLight ) 
	{
		// projected light
		SetLightProject( light->lightProject, vec3_origin /* light->parms.origin */, light->parms.target, 
			light->parms.right, light->parms.up, light->parms.start, light->parms.end);
	} 
	else 
	{
		// point light
		std::memset( light->lightProject, 0, sizeof( light->lightProject ) );
		light->lightProject[0][0] = 0.5f / light->parms.lightRadius[0];
		light->lightProject[1][1] = 0.5f / light->parms.lightRadius[1];
		light->lightProject[3][2] = 0.5f / light->parms.lightRadius[2];
		light->lightProject[0][3] = 0.5f;
		light->lightProject[1][3] = 0.5f;
		light->lightProject[2][3] = 1.0f;
		light->lightProject[3][3] = 0.5f;
	}

	// set the frustum planes
	SetLightFrustum( light->lightProject, light->frustum );

	// rotate the light planes and projections by the axis
	R_AxisToModelMatrix( light->parms.axis, light->parms.origin, light->modelMatrix );

	for ( i = 0 ; i < 6 ; i++ ) 
	{
		idPlane		temp;
		temp = light->frustum[i];
		R_LocalPlaneToGlobal( light->modelMatrix, temp, light->frustum[i] );
	}
	
	for ( i = 0 ; i < 4 ; i++ ) 
	{
		idPlane		temp;
		temp = light->lightProject[i];
		R_LocalPlaneToGlobal( light->modelMatrix, temp, light->lightProject[i] );
	}

	// adjust global light origin for off center projections and parallel projections
	// we are just faking parallel by making it a very far off center for now
	if ( light->parms.parallel ) 
	{
		idVec3	dir;

		dir = light->parms.lightCenter;
		if ( !dir.Normalize() )
			dir[2] = 1; // make point straight up if not specified
		
		light->globalLightOrigin = light->parms.origin + dir * 100000;
	} 
	else 
	{
		light->globalLightOrigin = light->parms.origin + light->parms.axis * light->parms.lightCenter;
	}

	FreeLightDefFrustum( light );

	light->frustumTris = PolytopeSurface( 6, light->frustum, light->frustumWindings );

	// a projected light will have one shadowFrustum, a point light will have
	// six unless the light center is outside the box
	MakeShadowFrustums( light );
}

// BEATO End

/*
==============
CreateMapLight

==============
*/
static void CreateMapLight( const idMapEntity *mapEnt ) 
{
	bool	dynamic = false;
	mapLight_t	*light = nullptr;

	// designers can add the "noPrelight" flag to signal that
	// the lights will move around, so we don't want
	// to bother chopping up the surfaces under it or creating
	// shadow volumes
	mapEnt->epairs.GetBool( "noPrelight", "0", dynamic );
	if ( dynamic ) 
		return;

	light = new mapLight_t;
	light->name[0] = '\0';
	light->shadowTris = nullptr;

	// parse parms exactly as the game do
	// use the game's epair parsing code so
	// we can use the same renderLight generation
	gameEdit->ParseSpawnArgsToRenderLight( &mapEnt->epairs, &light->def.parms );

	DeriveLightData( &light->def );

	// get the name for naming the shadow surfaces
	const char	*name;

	mapEnt->epairs.GetString( "name", "", &name );

	idStr::Copynz( light->name, name, sizeof( light->name ) );
	if ( !light->name[0] ) {
		common->Error( "Light at (%f,%f,%f) didn't have a name",
			light->def.parms.origin[0], light->def.parms.origin[1], light->def.parms.origin[2] );
	}
#if 0
	// use the renderer code to get the bounding planes for the light
	// based on all the parameters
	R_RenderLightFrustum( light->parms, light->frustum );
	light->lightShader = light->parms.shader;
#endif

	dmapGlobals.mapLights.Append( light );

}

/*
==============
CreateMapLights

==============
*/
static void CreateMapLights( const idMapFile *dmapFile ) 
{
	int					i = 0;
	const char*			value = nullptr;
	const idMapEntity*	mapEnt = nullptr;

	for ( i = 0 ; i < dmapFile->GetNumEntities() ; i++ ) 
	{
		mapEnt = dmapFile->GetEntity(i);
		mapEnt->epairs.GetString( "classname", "", &value);
		if ( !idStr::Icmp( value, "light" ) ) 
		{
			CreateMapLight( mapEnt );
		}
	}
}

/*
================
LoadDMapFile
================
*/
bool LoadDMapFile( const char *filename ) 
{		
	primitive_t	*prim;
	idBounds	mapBounds;
	int			brushes, triSurfs;
	int			i;
	int			size;

	common->Printf( "--- LoadDMapFile ---\n" );
	common->Printf( "loading %s\n", filename ); 

	// load and parse the map file into canonical form
	dmapGlobals.dmapFile = new idMapFile();
	if ( !dmapGlobals.dmapFile->Parse(filename) ) 
	{
		delete dmapGlobals.dmapFile;
		dmapGlobals.dmapFile = nullptr;
		common->Warning( "Couldn't load map file: '%s'", filename );
		return false;
	}

	dmapGlobals.mapPlanes.Clear();
	dmapGlobals.mapPlanes.SetGranularity( 1024 );

	// process the canonical form into utility form
	dmapGlobals.num_entities = 0;
	c_numMapPatches = 0;
	c_areaportals = 0;

	size = dmapGlobals.dmapFile->GetNumEntities() * sizeof( dmapGlobals.uEntities[0] );
	dmapGlobals.uEntities = (uEntity_t *)Mem_Alloc( size, TAG_DMAP );
	memset( dmapGlobals.uEntities, 0, size );

	// allocate a very large temporary brush for building
	// the brushes as they are loaded
	buildBrush = AllocBrush( MAX_BUILD_SIDES );

	for ( i = 0 ; i < dmapGlobals.dmapFile->GetNumEntities() ; i++ ) {
		ProcessMapEntity( dmapGlobals.dmapFile->GetEntity(i) );
	}

	CreateMapLights( dmapGlobals.dmapFile );

	brushes = 0;
	triSurfs = 0;

	mapBounds.Clear();
	for ( prim = dmapGlobals.uEntities[0].primitives ; prim ; prim = prim->next ) {
		if ( prim->brush ) {
			brushes++;
			mapBounds.AddBounds( prim->brush->bounds );
		} else if ( prim->tris ) {
			triSurfs++;
		}
	}

	common->Printf( "%5i total world brushes\n", brushes );
	common->Printf( "%5i total world triSurfs\n", triSurfs );
	common->Printf( "%5i patches\n", c_numMapPatches );
	common->Printf( "%5i entities\n", dmapGlobals.num_entities );
	common->Printf( "%5i planes\n", dmapGlobals.mapPlanes.Num() );
	common->Printf( "%5i areaportals\n", c_areaportals );
	common->Printf( "size: %5.0f,%5.0f,%5.0f to %5.0f,%5.0f,%5.0f\n", mapBounds[0][0], mapBounds[0][1],mapBounds[0][2],
				mapBounds[1][0], mapBounds[1][1], mapBounds[1][2] );

	return true;
}

/*
================
FreeOptimizeGroupList
================
*/
void FreeOptimizeGroupList( optimizeGroup_t *groups ) 
{
	optimizeGroup_t	*next = nullptr;

	for ( ; groups ; groups = next ) 
	{
		next = groups->nextGroup;
		FreeTriList( groups->triList );
		Mem_Free( groups );
	}
}

/*
================
FreeDMapFile
================
*/
void FreeDMapFile( void ) 
{
	int i = 0, j = 0;

	FreeBrush( buildBrush );
	buildBrush = nullptr;

	// free the entities and brushes
	for ( i = 0 ; i < dmapGlobals.num_entities ; i++ ) {
		uEntity_t	*ent;
		primitive_t	*prim, *nextPrim;

		ent = &dmapGlobals.uEntities[i];

		FreeTree( ent->tree );

		// free primitives
		for ( prim = ent->primitives ; prim ; prim = nextPrim ) {
			nextPrim = prim->next;
			if ( prim->brush ) {
				FreeBrush( prim->brush );
			}
			if ( prim->tris ) {
				FreeTriList( prim->tris );
			}
			Mem_Free( prim );
		}

		// free area surfaces
		if ( ent->areas ) {
			for ( j = 0 ; j < ent->numAreas ; j++ ) {
				uArea_t	*area;

				area = &ent->areas[j];
				FreeOptimizeGroupList( area->groups );

			}
			Mem_Free( ent->areas );
		}
	}

	Mem_Free( dmapGlobals.uEntities );

	dmapGlobals.num_entities = 0;

	// free the map lights
	for ( i = 0; i < dmapGlobals.mapLights.Num(); i++ ) 
	{
		R_FreeLightDefDerivedData( &dmapGlobals.mapLights[i]->def );
	}
	dmapGlobals.mapLights.DeleteContents( true );
}
