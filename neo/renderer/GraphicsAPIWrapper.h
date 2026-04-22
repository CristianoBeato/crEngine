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
#ifndef __GRAPHICSAPIWRAPPER_H__
#define __GRAPHICSAPIWRAPPER_H__

/*
================================================================================================

	Graphics API wrapper/helper functions

	This wraps platform specific graphics API functionality that is used at run-time. This
	functionality is wrapped to avoid excessive conditional compilation and/or code duplication
	throughout the run-time rendering code that is shared on all platforms.

	Most other graphics API functions are called for initialization purposes and are called
	directly from platform specific code implemented in files in the platform specific folders:

	renderer/OpenGL/
	renderer/Vulkan/

================================================================================================
*/

class idImage;
//class idTriangles;
class idRenderModelSurface;
class idDeclRenderProg;
class idRenderTexture;

inline constexpr int MAX_OCCLUSION_QUERIES = 4096;
// returned by GL_GetDeferredQueryResult() when the query is from too long ago and the result is no longer available
inline constexpr int OCCLUSION_QUERY_TOO_OLD				= -1;

inline constexpr int MAX_MULTITEXTURE_UNITS = 8;
inline constexpr uint32_t MAX_UNIFORM_BLOCKS = 4086; // these are the maximum entities by draw call 
inline constexpr uint32_t MAX_LIGHT_BLOCKS = 2048;

#endif // !__GRAPHICSAPIWRAPPER_H__
