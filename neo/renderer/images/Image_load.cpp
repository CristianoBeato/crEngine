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

#include "framework/Common_local.h"
#include "renderer_common.h"

// motorsep 05-19-2015; bool to determine current colorspace; False is YCoCg, True is RGB
bool skyboxRGBswap;

extern idCVar image_highQualityCompression;

extern idCVar r_useHightQualitySky;

idCVar r_cacheToolImages( "r_cacheToolImages", "1", CVAR_BOOL, "Enable binarization and caching of editor formatted images. Disable will not load from generated or save to generated." );

/*
// certain tools force any loaded materials to be put into low quality editor modes
textureUsage_t CheckEditorUsage( textureUsage_t usage )
{
	const int lowQualityImageTools = EDITOR_RADIANT|EDITOR_MATERIAL|EDITOR_PARTICLE;
	if( (com_editors&lowQualityImageTools)!=0 )
	{
		switch(usage)
		{
		case TD_DIFFUSE: 
			usage = TD_EDITOR_DIFFUSE;
			break;
		case TD_BUMP: 
			usage = TD_EDITOR_BUMP;
			break;
		case TD_COVERAGE: 
			usage = TD_EDITOR_COVERAGE;
			break;
		default:
			usage = TD_EDITOR_DEFAULT;
			break;
		}
	}
	return usage;
}
*/

/*
================
MakePowerOfTwo
================
*/
int MakePowerOfTwo( int num )
{
	int	pot;
	for( pot = 1; pot < num; pot <<= 1 )
	{
	}
	return pot;
}
