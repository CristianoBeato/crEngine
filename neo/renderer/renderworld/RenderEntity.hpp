/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2025 Cristiano B. Santos

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

#ifndef __RENDER_ENTITY_HPP__
#define __RENDER_ENTITY_HPP__

class idRenderEntityLocal : public idRenderEntity
{
public:
	idRenderEntityLocal();
	
	virtual void			FreeRenderEntity();
	virtual void			UpdateRenderEntity( const renderEntity_t* re, bool forceUpdate = false );
	virtual void			GetRenderEntity( renderEntity_t* re );
	virtual void			ForceUpdate();
	virtual int				GetIndex();
	
	// overlays are extra polygons that deform with animating models for blood and damage marks
	virtual void			ProjectOverlay( const idPlane localTextureAxis[2], const idMaterial* material );
	virtual void			RemoveDecals();
	
	bool					IsDirectlyVisible() const;
	void					ReadFromDemoFile( class idDemoFile* f );
	void					WriteToDemoFile( class idDemoFile* f ) const;
	renderEntity_t			parms;
	
	float					modelMatrix[16];		// this is just a rearrangement of parms.axis and parms.origin
	idRenderMatrix			modelRenderMatrix;
	idRenderMatrix			inverseBaseModelProject;// transforms the unit cube to exactly cover the model in world space
	
	idRenderWorldLocal* 	world;
	int						index;					// in world entityDefs
	
	int						lastModifiedFrameNum;	// to determine if it is constantly changing,
	// and should go in the dynamic frame memory, or kept
	// in the cached memory
	bool					archived;				// for demo writing
	
	idRenderModel* 			dynamicModel;			// if parms.model->IsDynamicModel(), this is the generated data
	int						dynamicModelFrameCount;	// continuously animating dynamic models will recreate
	// dynamicModel if this doesn't == tr.viewCount
	idRenderModel* 			cachedDynamicModel;
	
	
	// the local bounds used to place entityRefs, either from parms for dynamic entities, or a model bounds
	idBounds				localReferenceBounds;
	
	// axis aligned bounding box in world space, derived from refernceBounds and
	// modelMatrix in R_CreateEntityRefs()
	idBounds				globalReferenceBounds;
	
	// a viewEntity_t is created whenever a idRenderEntityLocal is considered for inclusion
	// in a given view, even if it turns out to not be visible
	int						viewCount;				// if tr.viewCount == viewCount, viewEntity is valid,
	// but the entity may still be off screen
	viewEntity_t* 			viewEntity;				// in frame temporary memory
	
	idRenderModelDecal* 	decals;					// decals that have been projected on this model
	idRenderModelOverlay* 	overlays;				// blood overlays on animated models
	
	areaReference_t* 		entityRefs;				// chain of all references
	idInteraction* 			firstInteraction;		// doubly linked list
	idInteraction* 			lastInteraction;
	
	bool					needsPortalSky;
};

class idRenderLightLocal : public idRenderLight
{
public:
	idRenderLightLocal();
	
	virtual void			FreeRenderLight();
	virtual void			UpdateRenderLight( const renderLight_t* re, bool forceUpdate = false );
	virtual void			GetRenderLight( renderLight_t* re );
	virtual void			ForceUpdate();
	virtual int				GetIndex();
	
	bool					LightCastsShadows() const
	{
		return parms.forceShadows || ( !parms.noShadows && lightShader->LightCastsShadows() );
	}
	
	renderLight_t			parms;					// specification
	
	bool					lightHasMoved;			// the light has changed its position since it was
	// first added, so the prelight model is not valid
	idRenderWorldLocal* 	world;
	int						index;					// in world lightdefs
	
	int						areaNum;				// if not -1, we may be able to cull all the light's
	// interactions if !viewDef->connectedAreas[areaNum]
	
	int						lastModifiedFrameNum;	// to determine if it is constantly changing,
	// and should go in the dynamic frame memory, or kept
	// in the cached memory
	bool					archived;				// for demo writing
	
	
	// derived information
	idPlane					lightProject[4];		// old style light projection where Z and W are flipped and projected lights lightProject[3] is divided by ( zNear + zFar )
	idRenderMatrix			baseLightProject;		// global xyz1 to projected light strq
	idRenderMatrix			inverseBaseLightProject;// transforms the zero-to-one cube to exactly cover the light in world space
	
	const idMaterial* 		lightShader;			// guaranteed to be valid, even if parms.shader isn't
	idImage* 				falloffImage;
	
	idVec3					globalLightOrigin;		// accounting for lightCenter and parallel
	idBounds				globalLightBounds;
	
	int						viewCount;				// if == tr.viewCount, the light is on the viewDef->viewLights list
	viewLight_t* 			viewLight;
	
	areaReference_t* 		references;				// each area the light is present in will have a lightRef
	idInteraction* 			firstInteraction;		// doubly linked list
	idInteraction* 			lastInteraction;
	
	struct doublePortal_s* 	foggedPortals;

// BEATO Begin: dmap structures for map compilation

	// in global space, positive side facing out, last two are front/back
	idPlane					frustum[6];

	// one for projected lights, usually six for point lights
	int						numShadowFrustums;
	shadowFrustum_t			shadowFrustums[6];
	
	// used for culling
	idWinding *				frustumWindings[6];
	
	// this is just a rearrangement of parms.axis and parms.origin
	float					modelMatrix[16];

	// triangulated frustumWindings[]
	crDrawGeometry *		frustumTris;
// BEATO End
};

ID_INLINE bool R_CullModelBoundsToLight( const idRenderLightLocal* light, const idBounds& localBounds, const idRenderMatrix& modelRenderMatrix )
{
	idRenderMatrix modelLightProject;
	idRenderMatrix::Multiply( light->baseLightProject, modelRenderMatrix, modelLightProject );
	return idRenderMatrix::CullBoundsToMVP( modelLightProject, localBounds, true );
}

#endif //__RENDER_ENTITY_HPP__
