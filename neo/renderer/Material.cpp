/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2025-2026 Cristiano B. Santos

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
#include "Material.h"

/*
=============
crShaderStage::crShaderStage
=============
*/
crShaderStage::crShaderStage( void )
{
}

/*
=============
crShaderStage::~crShaderStage
=============
*/
crShaderStage::~crShaderStage( void )
{
}

/*
=============
crShaderStage::Clear
=============
*/
void crShaderStage::Clear( void )
{
	m_drawStateBits = 0;
	m_conditionRegister = 0;
	m_color.registers[0] = 0;
	m_color.registers[1] = 0;
	m_color.registers[2] = 0;
	m_color.registers[3] = 0;
}

/*
=============
crShaderStage::SetDrawStateBits
=============
*/
void crShaderStage::SetDrawStateBits( const uint64_t in_flags )
{
	m_drawStateBits |= in_flags;
}

/*
=================
idMaterial::ParseStage

An open brace has been parsed

{
    if <expression>
    map <imageprogram>
    "nearest" "linear" "clamp" "zeroclamp" "uncompressed" "highquality" "nopicmip"
    scroll, scale, rotate
}

=================
*/
bool crShaderStage::ParseStage( idLexer& src, idMaterial &mtr )
{
	idToken					token;
	const char*				str;
	vkSampler::filter_t		tf;
	vkSampler::wrapping_t	trp;
	textureUsage_t			td;
	cubeFiles_t			cubeMap;
	char				imageName[MAX_IMAGE_NAME];
	int					a, b;
	int					matrix[2][3];
	auto globalImages = idImageManager::Get();

	tf = vkSampler::FILTER_TRILINEAR;
	trp = vkSampler::WRAP_REPEAT;
	td = TD_DEFAULT;
	cubeMap = CF_2D;
	
	imageName[0] = 0;
	
	Clear();
	
	while( 1 )
	{
		if( mtr.TestMaterialFlag( MF_DEFAULTED ) )  	// we have a parse error
			return false;
		
		if( !src.ExpectAnyToken( &token ) )
		{
			mtr.SetMaterialFlag( MF_DEFAULTED );
			return false;
		}
		
		// the close brace for the entire material ends the draw block
		if( token == "}" )
			break;
		
		//BSM Nerve: Added for stage naming in the material editor
		if( !token.Icmp( "name" ) )
		{
			src.SkipRestOfLine();
			continue;
		}
		
		// image options
		if( !token.Icmp( "blend" ) )
		{
			ParseBlend( src, mtr );
			continue;
		}
		
		// foresthale 20140403: r_glow
		if ( !token.Icmp( "glow" ) )
		{
			m_glowStage = true;
			continue;
		}

		if( !token.Icmp( "map" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::Copynz( imageName, str, sizeof( imageName ) );
			continue;
		}
		
		if( !token.Icmp( "remoteRenderMap" ) )
		{
			m_texture.dynamic = DI_REMOTE_RENDER;
			m_texture.width = src.ParseInt();
			m_texture.height = src.ParseInt();
			continue;
		}
		
		if( !token.Icmp( "mirrorRenderMap" ) )
		{
			m_texture.dynamic = DI_MIRROR_RENDER;
			m_texture.width = src.ParseInt();
			m_texture.height = src.ParseInt();
			m_texture.texgen = TG_SCREEN;
			continue;
		}
		
		if( !token.Icmp( "xrayRenderMap" ) )
		{
			m_texture.dynamic = DI_XRAY_RENDER;
			m_texture.width = src.ParseInt();
			m_texture.height = src.ParseInt();
			m_texture.texgen = TG_SCREEN;
			continue;
		}

		if( !token.Icmp( "screen" ) )
		{
			m_texture.texgen = TG_SCREEN;
			continue;
		}

		if( !token.Icmp( "screen2" ) )
		{
			m_texture.texgen = TG_SCREEN2;
			continue;
		}

		if( !token.Icmp( "glassWarp" ) )
		{
			m_texture.texgen = TG_GLASSWARP;
			continue;
		}

		if( !token.Icmp( "videomap" ) )
		{
			// note that videomaps will always be in clamp mode, so texture
			// coordinates had better be in the 0 to 1 range
			if( !src.ReadToken( &token ) )
			{
				common->Warning( "missing parameter for 'videoMap' keyword in material '%s'", mtr.GetName() );
				continue;
			}
			bool loop = false;
			if( !token.Icmp( "loop" ) )
			{
				loop = true;
				if( !src.ReadToken( &token ) )
				{
					common->Warning( "missing parameter for 'videoMap' keyword in material '%s'", mtr.GetName() );
					continue;
				}
			}

			m_texture.cinematic = idCinematic::Alloc();
			m_texture.cinematic->InitFromFile( token.c_str(), loop );
			continue;
		}		

		if( !token.Icmp( "soundmap" ) )
		{
			if( !src.ReadToken( &token ) )
			{
				common->Warning( "missing parameter for 'soundmap' keyword in material '%s'", mtr.GetName() );
				continue;
			}

			m_texture.cinematic = new( TAG_MATERIAL ) idSndWindow();
			m_texture.cinematic->InitFromFile( token.c_str(), true );
			continue;
		}

		if( !token.Icmp( "cubeMap" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::Copynz( imageName, str, sizeof( imageName ) );
			cubeMap = CF_NATIVE;
			continue;
		}		

		if( !token.Icmp( "cameraCubeMap" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::Copynz( imageName, str, sizeof( imageName ) );
			cubeMap = CF_CAMERA;
			continue;
		}		

		if (!token.Icmp("cameraCubeSky")) // motorsep 12-30-2022; to use with cubemaps created from equirectangular panoramas in Bixorama (or perhaps any other similar software)
		{
			str = R_ParsePastImageProgram(src);
			idStr::Copynz(imageName, str, sizeof(imageName));
			cubeMap = CF_CAMERA_ALT;
			continue;
		}

		if( !token.Icmp( "ignoreAlphaTest" ) )
		{
			m_ignoreAlphaTest = true;
			continue;
		}

		if( !token.Icmp( "nearest" ) )
		{
			tf = vkSampler::FILTER_NEAREST;
			continue;
		}

		if( !token.Icmp( "linear" ) )
		{
			tf = vkSampler::FILTER_LINEAR;
			continue;
		}

		if( !token.Icmp( "clamp" ) )
		{
			trp = vkSampler::WRAP_BORDER;
			continue;
		}

		if( !token.Icmp( "noclamp" ) )
		{
			trp = vkSampler::WRAP_REPEAT;
			continue;
		}

		if( !token.Icmp( "zeroclamp" ) )
		{
			trp = vkSampler::WRAP_BORDER;
			continue;
		}

		if( !token.Icmp( "alphazeroclamp" ) )
		{
			trp = vkSampler::WRAP_BORDER;
			continue;
		}

		if( !token.Icmp( "forceHighQuality" ) )
		{
			td = TD_HIGHQUALITY;	// sikk - Added - High Quality Texture Depth (full RGBA)
			continue;
		}

		if( !token.Icmp( "highquality" ) )
		{
			td = TD_HIGHQUALITY;	// sikk - Added - High Quality Texture Depth (full RGBA)
			continue;
		}

		if( !token.Icmp( "uncompressed" ) )
		{
			td = TD_HIGHQUALITY;	// sikk - Added - High Quality Texture Depth (full RGBA)
			continue;
		}

		if( !token.Icmp( "uncompressedCubeMap" ) ) 
		{			
			if( r_useHightQualitySky.GetBool() ) 
				td = TD_HIGHQUALITY_CUBE;	// motorsep 05-17-2015; token to mark cumebap/skybox to be uncompressed texture									

			if( !r_useHightQualitySky.GetBool() ) 
				td = TD_LOWQUALITY_CUBE;
			
			continue;
		}	

		if( !token.Icmp( "nopicmip" ) )
			continue;

		if( !token.Icmp( "vertexColor" ) )
		{
			m_vertexColor = SVC_MODULATE;
			continue;
		}

		if( !token.Icmp( "inverseVertexColor" ) )
		{
			m_vertexColor = SVC_INVERSE_MODULATE;
			continue;
		}
		
		// privatePolygonOffset
		else if( !token.Icmp( "privatePolygonOffset" ) )
		{
			if( !src.ReadTokenOnLine( &token ) )
			{
				m_privatePolygonOffset = 1;
				continue;
			}
			// explict larger (or negative) offset
			src.UnreadToken( &token );
			m_privatePolygonOffset = src.ParseFloat();
			continue;
		}
		
		// texture coordinate generation
		if( !token.Icmp( "texGen" ) )
		{
			src.ExpectAnyToken( &token );
			if( !token.Icmp( "normal" ) )
			{
				m_texture.texgen = TG_DIFFUSE_CUBE;
			}
			else if( !token.Icmp( "reflect" ) )
			{
				m_texture.texgen = TG_REFLECT_CUBE;
			}
			else if( !token.Icmp( "skybox" ) )
			{
				m_texture.texgen = TG_SKYBOX_CUBE;
			}
			else if( !token.Icmp( "wobbleSky" ) )
			{
				m_texture.texgen = TG_WOBBLESKY_CUBE;
				mtr.texGenRegisters[0] = ParseExpression( src );
				mtr.texGenRegisters[1] = ParseExpression( src );
				mtr.texGenRegisters[2] = ParseExpression( src );
			}
			else if ( !token.Icmp( "scriptsky" ) )
			{
				m_texture.texgen = TG_SCRIPTSKY_CUBE;
			}
			else
			{
				common->Warning( "bad texGen '%s' in material %s", token.c_str(), mtr.GetName() );
				mtr.SetMaterialFlag( MF_DEFAULTED );
			}
			continue;
		}

		if( !token.Icmp( "scroll" ) || !token.Icmp( "translate" ) )
		{
			a = ParseExpression( src );
			MatchToken( src, "," );
			b = ParseExpression( src );
			matrix[0][0] = mtr.GetExpressionConstant( 1 );
			matrix[0][1] = mtr.GetExpressionConstant( 0 );
			matrix[0][2] = a;
			matrix[1][0] = mtr.GetExpressionConstant( 0 );
			matrix[1][1] = mtr.GetExpressionConstant( 1 );
			matrix[1][2] = b;
			
			mtr.MultiplyTextureMatrix( &m_texture, matrix );
			continue;
		}
		if( !token.Icmp( "scale" ) )
		{
			a = ParseExpression( src );
			MatchToken( src, "," );
			b = ParseExpression( src );
			// this just scales without a centering
			matrix[0][0] = a;
			matrix[0][1] = mtr.GetExpressionConstant( 0 );
			matrix[0][2] = mtr.GetExpressionConstant( 0 );
			matrix[1][0] = mtr.GetExpressionConstant( 0 );
			matrix[1][1] = b;
			matrix[1][2] = mtr.GetExpressionConstant( 0 );
			
			mtr.MultiplyTextureMatrix( &m_texture, matrix );
			continue;
		}
		if( !token.Icmp( "centerScale" ) )
		{
			a = ParseExpression( src );
			MatchToken( src, "," );
			b = ParseExpression( src );
			// this subtracts 0.5, then scales, then adds 0.5
			matrix[0][0] = a;
			matrix[0][1] = mtr.GetExpressionConstant( 0 );
			matrix[0][2] = mtr.EmitOp( mtr.GetExpressionConstant( 0.5 ), mtr.EmitOp( mtr.GetExpressionConstant( 0.5 ), a, OP_TYPE_MULTIPLY ), OP_TYPE_SUBTRACT );
			matrix[1][0] = mtr.GetExpressionConstant( 0 );
			matrix[1][1] = b;
			matrix[1][2] = mtr.EmitOp( mtr.GetExpressionConstant( 0.5 ), mtr.EmitOp( mtr.GetExpressionConstant( 0.5 ), b, OP_TYPE_MULTIPLY ), OP_TYPE_SUBTRACT );
			
			mtr.MultiplyTextureMatrix( &m_texture, matrix );
			continue;
		}
		if( !token.Icmp( "shear" ) )
		{
			a = ParseExpression( src );
			MatchToken( src, "," );
			b = ParseExpression( src );
			// this subtracts 0.5, then shears, then adds 0.5
			matrix[0][0] = mtr.GetExpressionConstant( 1 );
			matrix[0][1] = a;
			matrix[0][2] = mtr.EmitOp( mtr.GetExpressionConstant( -0.5 ), a, OP_TYPE_MULTIPLY );
			matrix[1][0] = b;
			matrix[1][1] = mtr.GetExpressionConstant( 1 );
			matrix[1][2] = mtr.EmitOp( mtr.GetExpressionConstant( -0.5 ), b, OP_TYPE_MULTIPLY );
			
			mtr.MultiplyTextureMatrix( &m_texture, matrix );
			continue;
		}
		if( !token.Icmp( "rotate" ) )
		{
			const idDeclTable* table;
			int		sinReg, cosReg;
			
			// in cycles
			a = ParseExpression( src );
			
			table = static_cast<const idDeclTable*>( declManager->FindType( DECL_TABLE, "sinTable", false ) );
			if( !table )
			{
				common->Warning( "no sinTable for rotate defined" );
				mtr.SetMaterialFlag( MF_DEFAULTED );
				return;
			}

			sinReg = mtr.EmitOp( table->Index(), a, OP_TYPE_TABLE );
			
			table = static_cast<const idDeclTable*>( declManager->FindType( DECL_TABLE, "cosTable", false ) );
			if( !table )
			{
				common->Warning( "no cosTable for rotate defined" );
				mtr.SetMaterialFlag( MF_DEFAULTED );
				return;
			}

			cosReg = mtr.EmitOp( table->Index(), a, OP_TYPE_TABLE );
			
			// this subtracts 0.5, then rotates, then adds 0.5
			matrix[0][0] = cosReg;
			matrix[0][1] = mtr.EmitOp( mtr.GetExpressionConstant( 0 ), sinReg, OP_TYPE_SUBTRACT );
			matrix[0][2] = mtr.EmitOp( mtr.EmitOp( mtr.EmitOp( mtr.GetExpressionConstant( -0.5 ), cosReg, OP_TYPE_MULTIPLY ),
										   mtr.EmitOp( mtr.GetExpressionConstant( 0.5 ), sinReg, OP_TYPE_MULTIPLY ), OP_TYPE_ADD ),
								   mtr.GetExpressionConstant( 0.5 ), OP_TYPE_ADD );
								   
			matrix[1][0] = sinReg;
			matrix[1][1] = cosReg;
			matrix[1][2] = mtr.EmitOp( mtr.EmitOp( mtr.EmitOp( mtr.GetExpressionConstant( -0.5 ), sinReg, OP_TYPE_MULTIPLY ),
									mtr.EmitOp( mtr.GetExpressionConstant( -0.5 ), cosReg, OP_TYPE_MULTIPLY ), OP_TYPE_ADD ),
									mtr.GetExpressionConstant( 0.5 ), OP_TYPE_ADD );
								   
			mtr.MultiplyTextureMatrix( &m_texture, matrix );
			continue;
		}
		
		// color mask options
		if( !token.Icmp( "maskRed" ) )
		{
			m_drawStateBits |= GLS_REDMASK;
			continue;
		}

		if( !token.Icmp( "maskGreen" ) )
		{
			m_drawStateBits |= GLS_GREENMASK;
			continue;
		}

		if( !token.Icmp( "maskBlue" ) )
		{
			m_drawStateBits |= GLS_BLUEMASK;
			continue;
		}

		if( !token.Icmp( "maskAlpha" ) )
		{
			m_drawStateBits |= GLS_ALPHAMASK;
			continue;
		}
		
		if( !token.Icmp( "maskColor" ) )
		{
			m_drawStateBits |= GLS_COLORMASK;
			continue;
		}
		
		if( !token.Icmp( "maskDepth" ) )
		{
			m_drawStateBits |= GLS_DEPTHMASK;
			continue;
		}

		if( !token.Icmp( "alphaTest" ) )
		{
			m_alphaTestRegister = true;
			m_alphaTestRegister = ParseExpression( src );
			mtr.coverage = MC_PERFORATED;
			continue;
		}
		
		// shorthand for 2D modulated
		if( !token.Icmp( "colored" ) )
		{
			m_color.registers[0] = EXP_REG_PARM0;
			m_color.registers[1] = EXP_REG_PARM1;
			m_color.registers[2] = EXP_REG_PARM2;
			m_color.registers[3] = EXP_REG_PARM3;
			mtr.pd->registersAreConstant = false;
			continue;
		}
		
		if( !token.Icmp( "color" ) )
		{
			m_color.registers[0] = ParseExpression( src );
			MatchToken( src, "," );
			m_color.registers[1] = ParseExpression( src );
			MatchToken( src, "," );
			m_color.registers[2] = ParseExpression( src );
			MatchToken( src, "," );
			m_color.registers[3] = ParseExpression( src );
			continue;
		}

		if( !token.Icmp( "red" ) )
		{
			m_color.registers[0] = ParseExpression( src );
			continue;
		}

		if( !token.Icmp( "green" ) )
		{
			m_color.registers[1] = ParseExpression( src );
			continue;
		}

		if( !token.Icmp( "blue" ) )
		{
			m_color.registers[2] = ParseExpression( src );
			continue;
		}

		if( !token.Icmp( "alpha" ) )
		{
			m_color.registers[3] = ParseExpression( src );
			continue;
		}

		if( !token.Icmp( "rgb" ) )
		{
			m_color.registers[0] = m_color.registers[1] = m_color.registers[2] = ParseExpression( src );
			continue;
		}

		if( !token.Icmp( "rgba" ) )
		{
			m_color.registers[0] = m_color.registers[1] = m_color.registers[2] = m_color.registers[3] = ParseExpression( src );
			continue;
		}

// ---> sikk - Added - No Motionblur Material Stage Flag
		if ( !token.Icmp( "noBlur" ) ) 
		{
			m_noMotionBlur = true;
			continue;
		}
// <--- sikk - Added - No Motionblur Material Stage Flag
		
		if( !token.Icmp( "if" ) )
		{
			m_conditionRegister = ParseExpression( src );
			continue;
		}

		auto pipelineManager = crPipelineManager::Get();
		if( !token.Icmp( "program" ) )
		{
			if( src.ReadTokenOnLine( &token ) )
			{
				m_vertexProgram = pipelineManager->FindShader( token, crPipelineManager::ST_VERTEX ); 
				m_fragmentProgram = pipelineManager->FindShader( token, crPipelineManager::ST_FRAGMENT);
			}
			continue;
		}

		if( !token.Icmp( "fragmentProgram" ) )
		{
			if( src.ReadTokenOnLine( &token ) )
				m_fragmentProgram = pipelineManager->FindShader( token, crPipelineManager::ST_FRAGMENT );
			
			continue;
		}

		if( !token.Icmp( "vertexProgram" ) )
		{
			if( src.ReadTokenOnLine( &token ) )
				m_vertexProgram = pipelineManager->FindShader( token, crPipelineManager::ST_VERTEX );

			continue;
		}
		
		if( !token.Icmp( "vertexParm2" ) )
		{
			if( !ParseVertexParm2( src, mtr ) )
				return false;
			continue;
		}
		
		if( !token.Icmp( "vertexParm" ) )
		{
			if( !ParseVertexParm( src, mtr ) )
				return false;

			continue;
		}
		
		if( !token.Icmp( "fragmentMap" ) )
		{
			ParseFragmentMap( src, mtr );
			continue;
		}
		
		common->Warning( "unknown token '%s' in material '%s'", token.c_str(), mtr.GetName() );
		mtr.SetMaterialFlag( MF_DEFAULTED );
		return;
	}
	
	mtr.pd->numStages++;

	// select a compressed depth based on what the stage is
	if( td == TD_DEFAULT )
	{
		switch( m_lighting )
		{
			case SL_BUMP:
				td = TD_BUMP;
				break;
			case SL_DIFFUSE:
				td = TD_DIFFUSE;
				break;
			case SL_SPECULAR:
				td = TD_SPECULAR;
				break;
			case SL_GLOSS:
				td = TD_GLOSS;
				break;
			default:
				break;
		}
	}
	
	// create a new coverage stage on the fly - copy all data from the current stage
	if( ( td == TD_DIFFUSE ) && m_hasAlphaTest )
	{
		// create new coverage stage
		crShaderStage* newCoverageStage = &mtr.pd->parseStages[mtr.pd->numStages];
		mtr.pd->numStages++;
		// copy it
		*newCoverageStage = *this;
		// toggle alphatest off for the current stage so it doesn't get called during the depth fill pass
		m_hasAlphaTest = false;

		// toggle alpha test on for the coverage stage
		newCoverageStage->m_hasAlphaTest = true;
		newCoverageStage->m_lighting = SL_COVERAGE;
		textureStage_t* coverageTS = &newCoverageStage->m_texture;
		
		// now load the image with all the parms we parsed for the coverage stage
		if( imageName[0] )
		{
			// foresthale 2014-05-17: don't binarize when in the editors - we just run uncompressed from the source assets
			coverageTS->image = globalImages->ImageFromFile( imageName, CheckEditorUsage( TD_COVERAGE ), cubeMap );
			if( !coverageTS->image )
				coverageTS->image = globalImages->DefaultImage();
			
		}
		else if( !coverageTS->cinematic && !coverageTS->dynamic && !NewShaderStage() )
		{
			common->Warning( "material '%s' had stage with no image", mtr.GetName() );
			coverageTS->image = globalImages->DefaultImage();
		}
	}

	// foresthale 2014-05-17: don't binarize when in the editors - we just run uncompressed from the source assets
	td = CheckEditorUsage( td );
		
	// now load the image with all the parms we parsed
	if( imageName[0] )
	{
		m_texture.image = globalImages->ImageFromFile( imageName, td, cubeMap );
		if( !m_texture.image )
			m_texture.image = globalImages->DefaultImage();
	}
	else if( !m_texture.cinematic && !m_texture.dynamic && !NewShaderStage() )
	{
		common->Warning( "material '%s' had stage with no image", mtr.GetName() );
		m_texture.image = globalImages->DefaultImage();
	}
}

/*
================
crShaderStage::ParseBlend
================
*/
void crShaderStage::ParseBlend( idLexer& in_src, idMaterial &mtr )
{
	idToken token;
	int srcBlend, dstBlend;
	
	if( !in_src.ReadToken( &token ) )
		return;
	
	// blending combinations
	if( !token.Icmp( "blend" ) )
	{
		m_drawStateBits = GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
		return;
	}

	if( !token.Icmp( "add" ) )
	{
		m_drawStateBits = GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE;
		return;
	}

	if( !token.Icmp( "filter" ) || !token.Icmp( "modulate" ) )
	{
		m_drawStateBits = GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
		return;
	}

	if( !token.Icmp( "none" ) )
	{
		// none is used when defining an alpha mask that doesn't draw
		m_drawStateBits = GLS_SRCBLEND_ZERO | GLS_DSTBLEND_ONE;
		return;
	}

	if( !token.Icmp( "bumpmap" ) )
	{
		m_lighting = SL_BUMP;
		return;
	}

	if( !token.Icmp( "diffusemap" ) )
	{
		m_lighting = SL_DIFFUSE;
		return;
	}

	if( !token.Icmp( "specularmap" ) )
	{
		m_lighting = SL_SPECULAR;
		return;
	}

	if( !token.Icmp( "glossmap" ) )
	{
		m_lighting = SL_GLOSS;
		return;
	}
	
	srcBlend = mtr.NameToSrcBlendMode( token );
	
	MatchToken( in_src, "," );
	if( !in_src.ReadToken( &token ) )
		return;
	
	dstBlend = mtr.NameToDstBlendMode( token );
	
	m_drawStateBits = srcBlend | dstBlend;
}


/*
================
crShaderStage::ParseVertexParm

If there is a single value, it will be repeated across all elements
If there are two values, 3 = 0.0, 4 = 1.0
if there are three values, 4 = 1.0
================
*/
bool crShaderStage::ParseVertexParm( idLexer& src, idMaterial &mtr )
{
	idToken				token;
	
	src.ReadTokenOnLine( &token );
	int	parm = token.GetIntValue();
	if( !token.IsNumeric() || parm < 0 || parm >= MAX_VERTEX_PARMS )
	{
		common->Warning( "bad vertexParm number\n" );
		return false;
	}

	if( parm >= m_numVertexParms )
		m_numVertexParms = parm + 1;
	
	m_vertexParms[parm][0] = ParseExpression( src );
	
	src.ReadTokenOnLine( &token );
	if( !token[0] || token.Icmp( "," ) )
	{
		m_vertexParms[parm][1] = m_vertexParms[parm][2] = m_vertexParms[parm][3] = m_vertexParms[parm][0];
		return true;
	}
	
	m_vertexParms[parm][1] = ParseExpression( src );
	
	src.ReadTokenOnLine( &token );
	if( !token[0] || token.Icmp( "," ) )
	{
		m_vertexParms[parm][2] = mtr.GetExpressionConstant( 0 );
		m_vertexParms[parm][3] = mtr.GetExpressionConstant( 1 );
		return true;
	}
	
	m_vertexParms[parm][2] = ParseExpression( src );
	
	src.ReadTokenOnLine( &token );
	if( !token[0] || token.Icmp( "," ) )
	{
		m_vertexParms[parm][3] = mtr.GetExpressionConstant( 1 );
		return true;
	}
	
	m_vertexParms[parm][3] = ParseExpression( src );
	return true;
}

/*
================
crShaderStage::ParseVertexParm2
================
*/
bool crShaderStage::ParseVertexParm2( idLexer& src, idMaterial &mtr )
{
	idToken	token;
	src.ReadTokenOnLine( &token );
	int	parm = token.GetIntValue();
	if( !token.IsNumeric() || parm < 0 || parm >= MAX_VERTEX_PARMS )
	{
		common->Warning( "bad vertexParm number\n" );
		return false;
	}
	
	if( parm >= m_numVertexParms )
		m_numVertexParms = parm + 1;
	
	m_vertexParms[parm][0] = ParseExpression( src );
	MatchToken( src, "," );
	m_vertexParms[parm][1] = ParseExpression( src );
	MatchToken( src, "," );
	m_vertexParms[parm][2] = ParseExpression( src );
	MatchToken( src, "," );
	m_vertexParms[parm][3] = ParseExpression( src );

	return true;
}


/*
================
crShaderStage::ParseFragmentMap
================
*/
bool crShaderStage::ParseFragmentMap( idLexer& src, idMaterial &in_mtr )
{
	const char*			str;
	textureUsage_t		td;
	cubeFiles_t			cubeMap;
	idToken				token;
	
// BEATO Begin:
	vkSampler::filter_t sf = vkSampler::FILTER_NEAREST;
	vkSampler::wrapping_t sr = vkSampler::WRAP_REPEAT;
// BEATO End
	td = TD_DEFAULT;
	cubeMap = CF_2D;
	
	src.ReadTokenOnLine( &token );
	int	unit = token.GetIntValue();
	if( !token.IsNumeric() || unit < 0 || unit >= MAX_FRAGMENT_IMAGES )
	{
		common->Warning( "bad fragmentMap number\n" );
		return false;
	}
	
	// unit 1 is the normal map.. make sure it gets flagged as the proper depth
	// rebb : This is a way too generic assumption ! Replacing with "normalMap" image option for now
	/*
	if( unit == 1 )
	{
		td = TD_BUMP;
	}
	*/
	
	if( unit >= m_numFragmentProgramImages )
		m_numFragmentProgramImages = unit + 1;
	
	while( 1 )
	{
		src.ReadTokenOnLine( &token );
		
		if( !token.Icmp( "normalMap" ))
		{
			td = TD_BUMP;
			continue;
		}

		if( !token.Icmp( "cubeMap" ) )
		{
			cubeMap = CF_NATIVE;
			continue;
		}

		if( !token.Icmp( "cameraCubeMap" ) )
		{
			cubeMap = CF_CAMERA;
			continue;
		}

		if (!token.Icmp("cameraCubeSky")) // motorsep 12-30-2022; to use with cubemaps created from equirectangular panoramas in Bixorama (or perhaps any other similar software)
		{
			cubeMap = CF_CAMERA_ALT;
			continue;
		}

		if( !token.Icmp( "nearest" ) )
		{
			sf = vkSampler::FILTER_NEAREST;
			continue;
		}

		if( !token.Icmp( "linear" ) )
		{
			sf = vkSampler::FILTER_LINEAR;
			continue;
		}

		if( !token.Icmp( "clamp" ) )
		{
			sr = vkSampler::WRAP_BORDER;
			continue;
		}

		if( !token.Icmp( "noclamp" ) )
		{
			sr = vkSampler::WRAP_REPEAT;
			continue;
		}

		if( !token.Icmp( "zeroclamp" ) )
		{
			sr = vkSampler::WRAP_EDGE;
			continue;
		}

		if( !token.Icmp( "alphazeroclamp" ) )
		{
			sr = vkSampler::WRAP_BORDER;
			continue;
		}

		if( !token.Icmp( "forceHighQuality" ) )
		{
			td = TD_HIGHQUALITY;	// sikk - Added - High Quality Texture Depth (full RGBA)
			continue;
		}

		if( !token.Icmp( "highquality" ) )
		{
			td = TD_HIGHQUALITY;	// sikk - Added - High Quality Texture Depth (full RGBA)
			continue;
		}

		if( !token.Icmp( "uncompressed" ) )
		{
			td = TD_HIGHQUALITY;	// sikk - Added - High Quality Texture Depth (full RGBA)
			continue;
		}

		if( !token.Icmp( "nopicmip" ) )
			continue;
		
		// assume anything else is the image name
		src.UnreadToken( &token );
		break;
	}

	str = R_ParsePastImageProgram( src );

	// foresthale 2014-05-17: don't binarize when in the editors - we just run uncompressed from the source assets
	td = CheckEditorUsage( td );
	
	auto globalImages = idRenderSystem::GetGlobalImages();
	m_fragmentProgramImages[unit] = globalImages->ImageFromFile( str, td, cubeMap );
	if( !m_fragmentProgramImages[unit] )
		m_fragmentProgramImages[unit] = globalImages->DefaultImage();
	
	return true;
}

/*

Any errors during parsing just set MF_DEFAULTED and return, rather than throwing
a hard error. This will cause the material to fall back to default material,
but otherwise let things continue.

Each material may have a set of calculations that must be evaluated before
drawing with it.

Every expression that a material uses can be evaluated at one time, which
will allow for perfect common subexpression removal when I get around to
writing it.

Without this, scrolling an entire surface could result in evaluating the
same texture matrix calculations a half dozen times.

  Open question: should I allow arbitrary per-vertex color, texCoord, and vertex
  calculations to be specified in the material code?

  Every stage will definately have a valid image pointer.

  We might want the ability to change the sort value based on conditionals,
  but it could be a hassle to implement,

*/

// keep all of these on the stack, when they are static it makes material parsing non-reentrant
typedef struct mtrParsingData_s
{
	bool			registersAreConstant;
	bool			forceOverlays;
	bool			registerIsTemporary[MAX_EXPRESSION_REGISTERS];
	float			shaderRegisters[MAX_EXPRESSION_REGISTERS];
	expOp_t			shaderOps[MAX_EXPRESSION_OPS];
	uint32_t		numStages;
	crShaderStage	parseStages[MAX_SHADER_STAGES];
} mtrParsingData_t;

extern idCVar r_useHightQualitySky;

idCVar r_forceSoundOpAmplitude( "r_forceSoundOpAmplitude", "0", CVAR_FLOAT, "Don't call into the sound system for amplitudes" );

/*
=============
idMaterial::CommonInit
=============
*/
void idMaterial::CommonInit( void )
{
	desc = "<none>";
//	renderBump = "";
	contentFlags = CONTENTS_SOLID;
	surfaceFlags = SURFTYPE_NONE;
	materialFlags = 0;
	sort = SS_BAD;
	stereoEye = 0;
	coverage = MC_BAD;
	cullType = CT_FRONT_SIDED;
	deform = DFRM_NONE;
	numOps = 0;
	ops = nullptr;
	numRegisters = 0;
	expressionRegisters = nullptr;
	constantRegisters = nullptr;
	numAmbientStages = 0;
	editorImage = nullptr;
	lightFalloffSampler = nullptr;
	lightFalloffImage = nullptr;
	shouldCreateBackSides = false;
	entityGui = 0;
	fogLight = false;
	blendLight = false;
	ambientLight = false;
	noFog = false;
	hasSubview = false;
	allowOverlays = true;
	unsmoothedTangents = false;
	gui = nullptr;
	std::memset( deformRegisters, 0, sizeof( deformRegisters ) );
	editorAlpha = 1.0;
	spectrum = 0;
	polygonOffset = 0;
	suppressInSubview = false;
	refCount = 0;
	portalSky = false;
	fastPathSampler = nullptr;
	fastPathBumpImage = nullptr;
	fastPathDiffuseImage = nullptr;
	fastPathSpecularImage = nullptr;
	fastPathGlossImage = nullptr;
	deformDecl = nullptr;

	decalInfo.stayTime = 10000;
	decalInfo.fadeTime = 4000;
	decalInfo.start[0] = 1;
	decalInfo.start[1] = 1;
	decalInfo.start[2] = 1;
	decalInfo.start[3] = 1;
	decalInfo.end[0] = 0;
	decalInfo.end[1] = 0;
	decalInfo.end[2] = 0;
	decalInfo.end[3] = 0;
}


/*
=============
idMaterial::idMaterial
=============
*/
idMaterial::idMaterial( void )
{
	CommonInit();
	
	// we put this here instead of in CommonInit, because
	// we don't want it cleared when a material is purged
	surfaceArea = 0;
}

/*
=============
idMaterial::~idMaterial
=============
*/
idMaterial::~idMaterial( void )
{
}

/*
===============
idMaterial::FreeData
===============
*/
void idMaterial::FreeData( void )
{
	int i;

#if 0 /// BEATO Begin:
	if( stages )
	{
		// delete any idCinematic textures
		for( i = 0; i < numStages; i++ )
		{
			if( stages[i].texture.cinematic != nullptr )
			{
				delete stages[i].texture.cinematic;
				stages[i].texture.cinematic = nullptr;
			}

			if( stages[i].newStage != nullptr )
			{
				Mem_Free( stages[i].newStage );
				stages[i].newStage = nullptr;
			}
		}
		R_StaticFree( stages );
		stages = nullptr;
	}
#else
	for ( uint32_t i = 0; i < stages.Num(); i++)
	{
		stages[i].Clear();
	}

	stages.Clear();
#endif /// BEATO End

	if( expressionRegisters != nullptr )
	{
		R_StaticFree( expressionRegisters );
		expressionRegisters = nullptr;
	}
	if( constantRegisters != nullptr )
	{
		R_StaticFree( constantRegisters );
		constantRegisters = nullptr;
	}
	if( ops != nullptr )
	{
		R_StaticFree( ops );
		ops = nullptr;
	}
}

/*
==============
idMaterial::GetEditorImage
==============
*/
idImage* idMaterial::GetEditorImage( void ) const
{
	idImageManagerLocal* globalImages = dynamic_cast<idImageManagerLocal*>( idRenderSystem::GetGlobalImages() );

	if( editorImage )
		return editorImage;

	// if we don't have an editorImageName, use the first stage image
	if( !editorImageName.Length() )
	{
		// _D3XP :: First check for a diffuse image, then use the first
#if 0 // BEATO Begin:
		if( numStages && stages )
		{
			int i;
			for( i = 0; i < numStages; i++ )
			{
				if( stages[i].lighting == SL_DIFFUSE )
				{
					editorImage = stages[i].texture.image;
					break;
				}
			}
			if( !editorImage )
			{
				editorImage = stages[0].texture.image;
			}
		}
#else
		if( stages.Num() > 0 )
		{
			for( uint32_t i = 0; i < stages.Num(); i++ )
			{
				if( stages[i].Lighting() == SL_DIFFUSE )
				{
					editorImage = stages[i].Texture().image;
					break;
				}
			}

			if( !editorImage )
			{
				editorImage = stages[0].Texture().image;
			}
		}
#endif /// BEATO End
		else
		{
			editorImage = globalImages->defaultImage;
		}
	}
	else
	{
		// look for an explicit one
		editorImage = globalImages->ImageFromFile( editorImageName, TD_EDITOR_DEFAULT );
	}
	
	if( !editorImage )
	{
		editorImage = globalImages->defaultImage;
	}
	
	return editorImage;
}


// info parms
typedef struct
{
	const char*	name;
	int		clearSolid, surfaceFlags, contents;
} infoParm_t;

static infoParm_t	infoParms[] =
{
	// game relevant attributes
	{"solid",		0,	0,	CONTENTS_SOLID },		// may need to override a clearSolid
	{"water",		1,	0,	CONTENTS_WATER },		// used for water
	{"playerclip",	0,	0,	CONTENTS_PLAYERCLIP },	// solid to players
	{"monsterclip",	0,	0,	CONTENTS_MONSTERCLIP },	// solid to monsters
	{"moveableclip", 0,	0,	CONTENTS_MOVEABLECLIP }, // solid to moveable entities
	{"ikclip",		0,	0,	CONTENTS_IKCLIP },		// solid to IK
	{"blood",		0,	0,	CONTENTS_BLOOD },		// used to detect blood decals
	{"trigger",		0,	0,	CONTENTS_TRIGGER },		// used for triggers
	{"aassolid",	0,	0,	CONTENTS_AAS_SOLID },	// solid for AAS
	{"aasobstacle",	0,	0,	CONTENTS_AAS_OBSTACLE },// used to compile an obstacle into AAS that can be enabled/disabled
	{"flashlight_trigger",	0,	0,	CONTENTS_FLASHLIGHT_TRIGGER }, // used for triggers that are activated by the flashlight
	{"opaque",		0, 0,	CONTENTS_OPAQUE},		// opaque for AI visibility
	{"nonsolid",	1,	0,	0 },					// clears the solid flag
	{"nullNormal",	0,	SURF_nullptrNORMAL, 0 },		// renderbump will draw as 0x80 0x80 0x80
	
	// utility relevant attributes
	{"areaportal",	1,	0,	CONTENTS_AREAPORTAL },	// divides areas
	{"qer_nocarve",	1,	0,	CONTENTS_NOCSG},		// don't cut brushes in editor
	
	{"discrete",	1,	SURF_DISCRETE,	0 },		// surfaces should not be automatically merged together or
	// clipped to the world,
	// because they represent discrete objects like gui shaders
	// mirrors, or autosprites
	{"noFragment",	0,	SURF_NOFRAGMENT,	0 },
	
	{"slick",		0,	SURF_SLICK,		0 },
	{"collision",	0,	SURF_COLLISION,	0 },
	{"noimpact",	0,	SURF_NOIMPACT,	0 },		// don't make impact explosions or marks
	{"nodamage",	0,	SURF_NODAMAGE,	0 },		// no falling damage when hitting
	{"ladder",		0,	SURF_LADDER,	0 },		// climbable
	{"nosteps",		0,	SURF_NOSTEPS,	0 },		// no footsteps
	
	// material types for particle, sound, footstep feedback
	{"metal",		0,  SURFTYPE_METAL,		0 },	// metal
	{"stone",		0,  SURFTYPE_STONE,		0 },	// stone
	{"flesh",		0,  SURFTYPE_FLESH,		0 },	// flesh
	{"wood",		0,  SURFTYPE_WOOD,		0 },	// wood
	{"cardboard",	0,	SURFTYPE_CARDBOARD,	0 },	// cardboard
	{"liquid",		0,	SURFTYPE_LIQUID,	0 },	// liquid
	{"glass",		0,	SURFTYPE_GLASS,		0 },	// glass
	{"plastic",		0,	SURFTYPE_PLASTIC,	0 },	// plastic
	{"ricochet",	0,	SURFTYPE_RICOCHET,	0 },	// behaves like metal but causes a ricochet sound
	
	// unassigned surface types
	{"surftype10",	0,	SURFTYPE_10,	0 },
	{"surftype11",	0,	SURFTYPE_11,	0 },
	{"surftype12",	0,	SURFTYPE_12,	0 },
	{"surftype13",	0,	SURFTYPE_13,	0 },
	{"mirrorOblique",	0,	SURFTYPE_MIRROR,	0 }, // motorsep 04-07-2015; added SURFTYPE_MIRROR surface flag for oblique projections rendering cases
	{"surftype15",	0,	SURFTYPE_15,	0 },
};

static const int numInfoParms = sizeof( infoParms ) / sizeof( infoParms[0] );


/*
===============
idMaterial::CheckSurfaceParm

See if the current token matches one of the surface parm bit flags
===============
*/
bool idMaterial::CheckSurfaceParm( idToken* token )
{

	for( int i = 0 ; i < numInfoParms ; i++ )
	{
		if( !token->Icmp( infoParms[i].name ) )
		{
			if( infoParms[i].surfaceFlags & SURF_TYPE_MASK )
			{
				// ensure we only have one surface type set
				surfaceFlags &= ~SURF_TYPE_MASK;
			}
			surfaceFlags |= infoParms[i].surfaceFlags;
			contentFlags |= infoParms[i].contents;
			if( infoParms[i].clearSolid )
			{
				contentFlags &= ~CONTENTS_SOLID;
			}
			return true;
		}
	}
	return false;
}

/*
===============
idMaterial::MatchToken

Sets defaultShader and returns false if the next token doesn't match
===============
*/
bool idMaterial::MatchToken( idLexer& src, const char* match )
{
	if( !src.ExpectTokenString( match ) )
	{
		SetMaterialFlag( MF_DEFAULTED );
		return false;
	}
	return true;
}

/*
=================
idMaterial::ParseSort
=================
*/
void idMaterial::ParseSort( idLexer& src )
{
	idToken token;
	
	if( !src.ReadTokenOnLine( &token ) )
	{
		src.Warning( "missing sort parameter" );
		SetMaterialFlag( MF_DEFAULTED );
		return;
	}
	
	if( !token.Icmp( "subview" ) )
	{
		sort = SS_SUBVIEW;
	}
	else if( !token.Icmp( "opaque" ) )
	{
		sort = SS_OPAQUE;
	}
	else if( !token.Icmp( "decal" ) )
	{
		sort = SS_DECAL;
	}
	else if( !token.Icmp( "far" ) )
	{
		sort = SS_FAR;
	}
	else if( !token.Icmp( "medium" ) )
	{
		sort = SS_MEDIUM;
	}
	else if( !token.Icmp( "close" ) )
	{
		sort = SS_CLOSE;
	}
	else if( !token.Icmp( "almostNearest" ) )
	{
		sort = SS_ALMOST_NEAREST;
	}
	else if( !token.Icmp( "nearest" ) )
	{
		sort = SS_NEAREST;
	}
	else if( !token.Icmp( "postProcess" ) )
	{
		sort = SS_POST_PROCESS;
	}
	else if( !token.Icmp( "portalSky" ) )
	{
		sort = SS_PORTAL_SKY;
	}
	else
	{
		sort = std::atof( token );
	}
}

/*
=================
idMaterial::ParseStereoEye
=================
*/
void idMaterial::ParseStereoEye( idLexer& src )
{
	idToken token;
	
	if( !src.ReadTokenOnLine( &token ) )
	{
		src.Warning( "missing eye parameter" );
		SetMaterialFlag( MF_DEFAULTED );
		return;
	}
	
	if( !token.Icmp( "left" ) )
	{
		stereoEye = -1;
	}
	else if( !token.Icmp( "right" ) )
	{
		stereoEye = 1;
	}
	else
	{
		stereoEye = 0;
	}
}

/*
=================
idMaterial::ParseDecalInfo
=================
*/
void idMaterial::ParseDecalInfo( idLexer& src )
{
	idToken token;
	
	decalInfo.stayTime = src.ParseFloat() * 1000;
	decalInfo.fadeTime = src.ParseFloat() * 1000;
	float	start[4], end[4];
	src.Parse1DMatrix( 4, start );
	src.Parse1DMatrix( 4, end );
	for( int i = 0 ; i < 4 ; i++ )
	{
		decalInfo.start[i] = start[i];
		decalInfo.end[i] = end[i];
	}
}

/*
=============
idMaterial::GetExpressionConstant
=============
*/
int idMaterial::GetExpressionConstant( float f )
{
	int i = 0;
	
	for( i = EXP_REG_NUM_PREDEFINED ; i < numRegisters ; i++ )
	{
		if( !pd->registerIsTemporary[i] && pd->shaderRegisters[i] == f )
			return i;
	}

	if( numRegisters == MAX_EXPRESSION_REGISTERS )
	{
		common->Warning( "GetExpressionConstant: material '%s' hit MAX_EXPRESSION_REGISTERS", GetName() );
		SetMaterialFlag( MF_DEFAULTED );
		return 0;
	}

	pd->registerIsTemporary[i] = false;
	pd->shaderRegisters[i] = f;
	numRegisters++;
	
	return i;
}

/*
=============
idMaterial::GetExpressionTemporary
=============
*/
int idMaterial::GetExpressionTemporary( void )
{
	if( numRegisters >= MAX_EXPRESSION_REGISTERS )
	{
		common->Warning( "GetExpressionTemporary: material '%s' hit MAX_EXPRESSION_REGISTERS", GetName() );
		SetMaterialFlag( MF_DEFAULTED );
		return 0;
	}
	pd->registerIsTemporary[numRegisters] = true;
	numRegisters++;
	return numRegisters - 1;
}

/*
=============
idMaterial::GetExpressionOp
=============
*/
expOp_t* idMaterial::GetExpressionOp( void )
{
	if( numOps == MAX_EXPRESSION_OPS )
	{
		common->Warning( "GetExpressionOp: material '%s' hit MAX_EXPRESSION_OPS", GetName() );
		SetMaterialFlag( MF_DEFAULTED );
		return &pd->shaderOps[0];
	}
	
	return &pd->shaderOps[numOps++];
}

/*
=================
idMaterial::EmitOp
=================
*/
int idMaterial::EmitOp( int a, int b, expOpType_t opType )
{
	expOp_t*	op;
	
	// optimize away identity operations
	if( opType == OP_TYPE_ADD )
	{
		if( !pd->registerIsTemporary[a] && pd->shaderRegisters[a] == 0 )
			return b;
		
		if( !pd->registerIsTemporary[b] && pd->shaderRegisters[b] == 0 )
			return a;
		
		if( !pd->registerIsTemporary[a] && !pd->registerIsTemporary[b] )
			return GetExpressionConstant( pd->shaderRegisters[a] + pd->shaderRegisters[b] );
	}

	if( opType == OP_TYPE_MULTIPLY )
	{
		if( !pd->registerIsTemporary[a] && pd->shaderRegisters[a] == 1 )
			return b;

		if( !pd->registerIsTemporary[a] && pd->shaderRegisters[a] == 0 )
			return a;
		
		if( !pd->registerIsTemporary[b] && pd->shaderRegisters[b] == 1 )
			return a;

		if( !pd->registerIsTemporary[b] && pd->shaderRegisters[b] == 0 )
			return b;

		if( !pd->registerIsTemporary[a] && !pd->registerIsTemporary[b] )
			return GetExpressionConstant( pd->shaderRegisters[a] * pd->shaderRegisters[b] );
	}
	
	op = GetExpressionOp();
	op->opType = opType;
	op->a = a;
	op->b = b;
	op->c = GetExpressionTemporary();
	
	return op->c;
}

/*
=================
idMaterial::ParseEmitOp
=================
*/
int idMaterial::ParseEmitOp( idLexer& src, int a, expOpType_t opType, int priority )
{
	int		b;
	
	b = ParseExpressionPriority( src, priority );
	return EmitOp( a, b, opType );
}

/*
=================
idMaterial::ParseTerm

Returns a register index
=================
*/
int idMaterial::ParseTerm( idLexer& src )
{
	idToken token;
	int		a, b;
	
	src.ReadToken( &token );
	
	if( token == "(" )
	{
		a = ParseExpression( src );
		MatchToken( src, ")" );
		return a;
	}
	
	if( !token.Icmp( "time" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_TIME;
	}
	if( !token.Icmp( "parm0" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM0;
	}
	if( !token.Icmp( "parm1" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM1;
	}
	if( !token.Icmp( "parm2" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM2;
	}
	if( !token.Icmp( "parm3" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM3;
	}
	if( !token.Icmp( "parm4" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM4;
	}
	if( !token.Icmp( "parm5" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM5;
	}
	if( !token.Icmp( "parm6" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM6;
	}
	if( !token.Icmp( "parm7" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM7;
	}
	if( !token.Icmp( "parm8" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM8;
	}
	if( !token.Icmp( "parm9" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM9;
	}
	if( !token.Icmp( "parm10" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM10;
	}
	if( !token.Icmp( "parm11" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_PARM11;
	}
	if( !token.Icmp( "global0" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL0;
	}
	if( !token.Icmp( "global1" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL1;
	}
	if( !token.Icmp( "global2" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL2;
	}
	if( !token.Icmp( "global3" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL3;
	}
	if( !token.Icmp( "global4" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL4;
	}
	if( !token.Icmp( "global5" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL5;
	}
	if( !token.Icmp( "global6" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL6;
	}
	if( !token.Icmp( "global7" ) )
	{
		pd->registersAreConstant = false;
		return EXP_REG_GLOBAL7;
	}
	if( !token.Icmp( "fragmentPrograms" ) )
	{
		return 1.0f;
	}	
	if( !token.Icmp( "sound" ) )
	{
		pd->registersAreConstant = false;
		return EmitOp( 0, 0, OP_TYPE_SOUND );
	}
	
	// parse negative numbers
	if( token == "-" )
	{
		src.ReadToken( &token );
		if( token.type == TT_NUMBER || token == "." )
		{
			return GetExpressionConstant( -( float ) token.GetFloatValue() );
		}
		src.Warning( "Bad negative number '%s'", token.c_str() );
		SetMaterialFlag( MF_DEFAULTED );
		return 0;
	}
	
	if( token.type == TT_NUMBER || token == "." || token == "-" )
	{
		return GetExpressionConstant( ( float ) token.GetFloatValue() );
	}
	
	// see if it is a table name
	const idDeclTable* table = static_cast<const idDeclTable*>( declManager->FindType( DECL_TABLE, token.c_str(), false ) );
	if( table )
	{
		// parse a table expression
		MatchToken( src, "[" );

		b = ParseExpression( src );

		MatchToken( src, "]" );

		return EmitOp( table->Index(), b, OP_TYPE_TABLE );
	}

	const idDeclTable2d* table2d = static_cast<const idDeclTable2d*>( declManager->FindType( DECL_TABLE2D, token.c_str(), false ) );
	if ( table2d )
	{
		// parse a table expression
		MatchToken( src, "[" );

		b = ParseExpression( src );

		MatchToken( src, "]" );

		return EmitOp( table2d->Index(), b, OP_TYPE_TABLE2D );
	}	

	src.Warning( "Bad term '%s'", token.c_str() );
	SetMaterialFlag( MF_DEFAULTED );
	return 0;
}

/*
=================
idMaterial::ParseExpressionPriority

Returns a register index
=================
*/
#define	TOP_PRIORITY 4
int idMaterial::ParseExpressionPriority( idLexer& src, int priority )
{
	idToken token;
	int		a;
	
	if( priority == 0 )
	{
		return ParseTerm( src );
	}
	
	a = ParseExpressionPriority( src, priority - 1 );
	
	if( TestMaterialFlag( MF_DEFAULTED ) )  	// we have a parse error
	{
		return 0;
	}
	
	if( !src.ReadToken( &token ) )
	{
		// we won't get EOF in a real file, but we can
		// when parsing from generated strings
		return a;
	}
	
	if( priority == 1 && token == "*" )
	{
		return ParseEmitOp( src, a, OP_TYPE_MULTIPLY, priority );
	}
	if( priority == 1 && token == "/" )
	{
		return ParseEmitOp( src, a, OP_TYPE_DIVIDE, priority );
	}
	if( priority == 1 && token == "%" )  	// implied truncate both to integer
	{
		return ParseEmitOp( src, a, OP_TYPE_MOD, priority );
	}
	if( priority == 2 && token == "+" )
	{
		return ParseEmitOp( src, a, OP_TYPE_ADD, priority );
	}
	if( priority == 2 && token == "-" )
	{
		return ParseEmitOp( src, a, OP_TYPE_SUBTRACT, priority );
	}
	if( priority == 3 && token == ">" )
	{
		return ParseEmitOp( src, a, OP_TYPE_GT, priority );
	}
	if( priority == 3 && token == ">=" )
	{
		return ParseEmitOp( src, a, OP_TYPE_GE, priority );
	}
	if( priority == 3 && token == "<" )
	{
		return ParseEmitOp( src, a, OP_TYPE_LT, priority );
	}
	if( priority == 3 && token == "<=" )
	{
		return ParseEmitOp( src, a, OP_TYPE_LE, priority );
	}
	if( priority == 3 && token == "==" )
	{
		return ParseEmitOp( src, a, OP_TYPE_EQ, priority );
	}
	if( priority == 3 && token == "!=" )
	{
		return ParseEmitOp( src, a, OP_TYPE_NE, priority );
	}
	if( priority == 4 && token == "&&" )
	{
		return ParseEmitOp( src, a, OP_TYPE_AND, priority );
	}
	if( priority == 4 && token == "||" )
	{
		return ParseEmitOp( src, a, OP_TYPE_OR, priority );
	}
	
	// assume that anything else terminates the expression
	// not too robust error checking...
	
	src.UnreadToken( &token );
	
	return a;
}

/*
=================
idMaterial::ParseExpression

Returns a register index
=================
*/
int idMaterial::ParseExpression( idLexer& src )
{
	return ParseExpressionPriority( src, TOP_PRIORITY );
}

/*
===============
idMaterial::NameToSrcBlendMode
===============
*/
int idMaterial::NameToSrcBlendMode( const idStr& name )
{
	if( !name.Icmp( "GL_ONE" ) )
	{
		return GLS_SRCBLEND_ONE;
	}
	else if( !name.Icmp( "GL_ZERO" ) )
	{
		return GLS_SRCBLEND_ZERO;
	}
	else if( !name.Icmp( "GL_DST_COLOR" ) )
	{
		return GLS_SRCBLEND_DST_COLOR;
	}
	else if( !name.Icmp( "GL_ONE_MINUS_DST_COLOR" ) )
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_COLOR;
	}
	else if( !name.Icmp( "GL_SRC_ALPHA" ) )
	{
		return GLS_SRCBLEND_SRC_ALPHA;
	}
	else if( !name.Icmp( "GL_ONE_MINUS_SRC_ALPHA" ) )
	{
		return GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if( !name.Icmp( "GL_DST_ALPHA" ) )
	{
		return GLS_SRCBLEND_DST_ALPHA;
	}
	else if( !name.Icmp( "GL_ONE_MINUS_DST_ALPHA" ) )
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if( !name.Icmp( "GL_SRC_ALPHA_SATURATE" ) )
	{
		assert( 0 ); // FIX ME
		return GLS_SRCBLEND_SRC_ALPHA;
	}
	
	common->Warning( "unknown blend mode '%s' in material '%s'", name.c_str(), GetName() );
	SetMaterialFlag( MF_DEFAULTED );
	
	return GLS_SRCBLEND_ONE;
}

/*
===============
idMaterial::NameToDstBlendMode
===============
*/
int idMaterial::NameToDstBlendMode( const idStr& name )
{
	if( !name.Icmp( "GL_ONE" ) )
	{
		return GLS_DSTBLEND_ONE;
	}
	else if( !name.Icmp( "GL_ZERO" ) )
	{
		return GLS_DSTBLEND_ZERO;
	}
	else if( !name.Icmp( "GL_SRC_ALPHA" ) )
	{
		return GLS_DSTBLEND_SRC_ALPHA;
	}
	else if( !name.Icmp( "GL_ONE_MINUS_SRC_ALPHA" ) )
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if( !name.Icmp( "GL_DST_ALPHA" ) )
	{
		return GLS_DSTBLEND_DST_ALPHA;
	}
	else if( !name.Icmp( "GL_ONE_MINUS_DST_ALPHA" ) )
	{
		return GLS_DSTBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if( !name.Icmp( "GL_SRC_COLOR" ) )
	{
		return GLS_DSTBLEND_SRC_COLOR;
	}
	else if( !name.Icmp( "GL_ONE_MINUS_SRC_COLOR" ) )
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_COLOR;
	}
	
	common->Warning( "unknown blend mode '%s' in material '%s'", name.c_str(), GetName() );
	SetMaterialFlag( MF_DEFAULTED );
	
	return GLS_DSTBLEND_ONE;
}

/*
===============
idMaterial::MultiplyTextureMatrix
===============
*/
void idMaterial::MultiplyTextureMatrix( textureStage_t* ts, const int registers[2][3] )
{
	int		old[2][3];
	
	if( !ts->hasMatrix )
	{
		ts->hasMatrix = true;
		std::memcpy( ts->matrix, registers, sizeof( ts->matrix ) );
		return;
	}
	
	std::memcpy( old, ts->matrix, sizeof( old ) );
	
	// multiply the two maticies
	ts->matrix[0][0] = EmitOp(
						   EmitOp( old[0][0], registers[0][0], OP_TYPE_MULTIPLY ),
						   EmitOp( old[0][1], registers[1][0], OP_TYPE_MULTIPLY ), OP_TYPE_ADD );
	ts->matrix[0][1] = EmitOp(
						   EmitOp( old[0][0], registers[0][1], OP_TYPE_MULTIPLY ),
						   EmitOp( old[0][1], registers[1][1], OP_TYPE_MULTIPLY ), OP_TYPE_ADD );
	ts->matrix[0][2] = EmitOp(
						   EmitOp(
							   EmitOp( old[0][0], registers[0][2], OP_TYPE_MULTIPLY ),
							   EmitOp( old[0][1], registers[1][2], OP_TYPE_MULTIPLY ), OP_TYPE_ADD ),
						   old[0][2], OP_TYPE_ADD );
						   
	ts->matrix[1][0] = EmitOp(
						   EmitOp( old[1][0], registers[0][0], OP_TYPE_MULTIPLY ),
						   EmitOp( old[1][1], registers[1][0], OP_TYPE_MULTIPLY ), OP_TYPE_ADD );
	ts->matrix[1][1] = EmitOp(
						   EmitOp( old[1][0], registers[0][1], OP_TYPE_MULTIPLY ),
						   EmitOp( old[1][1], registers[1][1], OP_TYPE_MULTIPLY ), OP_TYPE_ADD );
	ts->matrix[1][2] = EmitOp(
						   EmitOp(
							   EmitOp( old[1][0], registers[0][2], OP_TYPE_MULTIPLY ),
							   EmitOp( old[1][1], registers[1][2], OP_TYPE_MULTIPLY ), OP_TYPE_ADD ),
						   old[1][2], OP_TYPE_ADD );
						   
}

void idMaterial::ParseStage( idLexer &src )
{
	uint32_t numStages = 0;
	idToken token;
	crShaderStage *ss = nullptr;

	if ( numStages >= MAX_SHADER_STAGES ) 
	{
		SetMaterialFlag( MF_DEFAULTED );
		common->Warning( "material '%s' exceeded %i stages", GetName(), MAX_SHADER_STAGES );
	}

	ss = &pd->parseStages[numStages];
	ss->ParseStage( src, *this );
}

/*
===============
idMaterial::ParseDeform
===============
*/
void idMaterial::ParseDeform( idLexer& src )
{
	idToken token;
	
	if( !src.ExpectAnyToken( &token ) )
		return;
	
	if( !token.Icmp( "sprite" ) )
	{
		deform = DFRM_SPRITE;
		cullType = CT_TWO_SIDED;
		SetMaterialFlag( MF_NOSHADOWS );
		return;
	}
	if( !token.Icmp( "tube" ) )
	{
		deform = DFRM_TUBE;
		cullType = CT_TWO_SIDED;
		SetMaterialFlag( MF_NOSHADOWS );
		return;
	}
	if( !token.Icmp( "flare" ) )
	{
		deform = DFRM_FLARE;
		cullType = CT_TWO_SIDED;
		deformRegisters[0] = ParseExpression( src );
		SetMaterialFlag( MF_NOSHADOWS );
		return;
	}
	if( !token.Icmp( "expand" ) )
	{
		deform = DFRM_EXPAND;
		deformRegisters[0] = ParseExpression( src );
		return;
	}
	if( !token.Icmp( "move" ) )
	{
		deform = DFRM_MOVE;
		deformRegisters[0] = ParseExpression( src );
		return;
	}
	if( !token.Icmp( "turbulent" ) )
	{
		deform = DFRM_TURB;
		
		if( !src.ExpectAnyToken( &token ) )
		{
			src.Warning( "deform particle missing particle name" );
			SetMaterialFlag( MF_DEFAULTED );
			return;
		}
		deformDecl = declManager->FindType( DECL_TABLE, token.c_str(), true );
		
		deformRegisters[0] = ParseExpression( src );
		deformRegisters[1] = ParseExpression( src );
		deformRegisters[2] = ParseExpression( src );
		return;
	}
	if( !token.Icmp( "eyeBall" ) )
	{
		deform = DFRM_EYEBALL;
		return;
	}
	if( !token.Icmp( "particle" ) )
	{
		deform = DFRM_PARTICLE;
		if( !src.ExpectAnyToken( &token ) )
		{
			src.Warning( "deform particle missing particle name" );
			SetMaterialFlag( MF_DEFAULTED );
			return;
		}
		deformDecl = declManager->FindType( DECL_PARTICLE, token.c_str(), true );
		return;
	}
	if( !token.Icmp( "particle2" ) )
	{
		deform = DFRM_PARTICLE2;
		if( !src.ExpectAnyToken( &token ) )
		{
			src.Warning( "deform particle missing particle name" );
			SetMaterialFlag( MF_DEFAULTED );
			return;
		}
		deformDecl = declManager->FindType( DECL_PARTICLE, token.c_str(), true );
		return;
	}
	src.Warning( "Bad deform type '%s'", token.c_str() );
	SetMaterialFlag( MF_DEFAULTED );
}


/*
==============
idMaterial::AddImplicitStages

If a material has diffuse or specular stages without any
bump stage, add an implicit _flat bumpmap stage.

If a material has a bump stage but no diffuse or specular
stage, add a _white diffuse stage.

It is valid to have either a diffuse or specular without the other.

It is valid to have a reflection map and a bump map for bumpy reflection
==============
*/
void idMaterial::AddImplicitStages( void )
{
	char	buffer[1024];
	idLexer		newSrc;
	bool hasDiffuse = false;
	bool hasSpecular = false;
	bool hasGloss = false;
	bool hasBump = false;
	bool hasReflection = false;
	
	for( int i = 0 ; i < stages.Num(); i++ )
	{
		if( pd->parseStages[i].Lighting() == SL_BUMP )
			hasBump = true;
		
		if( pd->parseStages[i].Lighting() == SL_DIFFUSE )
			hasDiffuse = true;

		if( pd->parseStages[i].Lighting() == SL_SPECULAR )
			hasSpecular = true;

		if( pd->parseStages[i].Lighting() == SL_GLOSS )
			hasGloss = true;

		if( pd->parseStages[i].Texture().texgen == TG_REFLECT_CUBE )
			hasReflection = true;
	}
	
	// if it doesn't have an interaction at all, don't add anything
	if( !hasBump && !hasDiffuse && !hasSpecular )
		return;
	
	if( stages.Num() == MAX_SHADER_STAGES )
		return;
	
	if( !hasBump )
	{
		idStr::snPrintf( buffer, sizeof( buffer ), "blend bumpmap\nmap _flat\n}\n" );
		newSrc.LoadMemory( buffer, strlen( buffer ), "bumpmap" );
		newSrc.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		ParseStage( newSrc );
		newSrc.FreeSource();
	}
	
	if( !hasDiffuse && !hasSpecular && !hasReflection )
	{
		idStr::snPrintf( buffer, sizeof( buffer ), "blend diffusemap\nmap _white\n}\n" );
		newSrc.LoadMemory( buffer, strlen( buffer ), "diffusemap" );
		newSrc.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		ParseStage( newSrc );
		newSrc.FreeSource();
	}	
}

/*
===============
idMaterial::SortInteractionStages

The renderer expects bump, then diffuse, then specular
There can be multiple bump maps, followed by additional
diffuse and specular stages, which allows cross-faded bump mapping.

Ambient stages can be interspersed anywhere, but they are
ignored during interactions, and all the interaction
stages are ignored during ambient drawing.
===============
*/
void idMaterial::SortInteractionStages( void )
{
	int i = 0, j = 0;
	
	for( i = 0 ; i < stages.Num(); i = j )
	{
		// find the next bump map
		for( j = i + 1 ; j < stages.Num() ; j++ )
		{
			if( pd->parseStages[j].Lighting() == SL_BUMP )
			{
				// if the very first stage wasn't a bumpmap,
				// this bumpmap is part of the first group
				if( pd->parseStages[i].Lighting() != SL_BUMP )
					continue;
				
				break;
			}
		}
		
		// bubble sort everything bump / diffuse / specular
		for( int l = 1 ; l < j - i ; l++ )
		{
			for( int k = i ; k < j - l ; k++ )
			{
				if( pd->parseStages[k].Lighting() > pd->parseStages[k + 1].Lighting() )
				{
					crShaderStage	temp;
					temp = pd->parseStages[k];
					pd->parseStages[k] = pd->parseStages[k + 1];
					pd->parseStages[k + 1] = temp;
				}
			}
		}
	}
}

/*
=================
idMaterial::ParseMaterial

The current text pointer is at the explicit text definition of the
Parse it into the global material variable. Later functions will optimize it.

If there is any error during parsing, defaultShader will be set.
=================
*/
void idMaterial::ParseMaterial( idLexer& src )
{
	int			i = 0;
	int			s = 0;
	const char*	str = nullptr;
	char		buffer[1024];
	idToken		token;
	idLexer		newSrc;
	vkSampler::filter_t sampFilter = vkSampler::FILTER_NEAREST;
	vkSampler::wrapping_t sampWraping = vkSampler::WRAP_BORDER;
	
	numOps = 0;
	numRegisters = EXP_REG_NUM_PREDEFINED;	// leave space for the parms to be copied in
	for( i = 0 ; i < numRegisters ; i++ )
	{
		pd->registerIsTemporary[i] = true;		// they aren't constants that can be folded
	}
	
	// numStages = 0;
	pd->registersAreConstant = true;			// until shown otherwise
	///textureRepeat_t	trpDefault = TR_REPEAT;		// allow a global setting for repeat
	
	while( 1 )
	{
		if( TestMaterialFlag( MF_DEFAULTED ) )  	// we have a parse error
			return;
		
		if( !src.ExpectAnyToken( &token ) )
		{
			SetMaterialFlag( MF_DEFAULTED );
			return;
		}
		
		// end of material definition
		if( token == "}" )
			break;

		else if( !token.Icmp( "qer_editorimage" ) )
		{
			src.ReadTokenOnLine( &token );
			editorImageName = token.c_str();
			src.SkipRestOfLine();
			continue;
		}
		// description
		else if( !token.Icmp( "description" ) )
		{
			src.ReadTokenOnLine( &token );
			desc = token.c_str();
			continue;
		}
		// check for the surface / content bit flags
		else if( CheckSurfaceParm( &token ) )
			continue;
				
		// polygonOffset
		else if( !token.Icmp( "polygonOffset" ) )
		{
			SetMaterialFlag( MF_POLYGONOFFSET );
			if( !src.ReadTokenOnLine( &token ) )
			{
				polygonOffset = 1;
				continue;
			}
			// explict larger (or negative) offset
			polygonOffset = token.GetFloatValue();

			// TODO: set direct in the pipeline ( may be a waste of pipeline allocation )
			continue;
		}
		// noshadow
		else if( !token.Icmp( "noShadows" ) )
		{
			SetMaterialFlag( MF_NOSHADOWS );
			continue;
		}
		else if( !token.Icmp( "suppressInSubview" ) )
		{
			suppressInSubview = true;
			continue;
		}
		else if( !token.Icmp( "portalSky" ) )
		{
			portalSky = true;
			continue;
		}
		// noSelfShadow
		else if( !token.Icmp( "noSelfShadow" ) )
		{
			SetMaterialFlag( MF_NOSELFSHADOW );
			continue;
		}
		// noPortalFog
		else if( !token.Icmp( "noPortalFog" ) )
		{
			SetMaterialFlag( MF_NOPORTALFOG );
			continue;
		}
		// forceShadows allows nodraw surfaces to cast shadows
		else if( !token.Icmp( "forceShadows" ) )
		{
			SetMaterialFlag( MF_FORCESHADOWS );
			continue;
		}
		// overlay / decal suppression
		else if( !token.Icmp( "noOverlays" ) )
		{
			allowOverlays = false;
			continue;
		}
		// moster blood overlay forcing for alpha tested or translucent surfaces
		else if( !token.Icmp( "forceOverlays" ) )
		{
			pd->forceOverlays = true;
			continue;
		}
		// translucent
		else if( !token.Icmp( "translucent" ) )
		{
			coverage = MC_TRANSLUCENT;
			continue;
		}
		// global zero clamp
		else if( !token.Icmp( "zeroclamp" ) )
		{
// BEATO Begin:
			//trpDefault = TR_CLAMP_TO_ZERO;
			sampWraping = vkSampler::WRAP_BORDER; // TODO: create a black border in texture
// BEATO End
			continue;
		}
		// global clamp
		else if( !token.Icmp( "clamp" ) )
		{
// BEATO Begin
			sampWraping = vkSampler::WRAP_BORDER; // trpDefault = TR_CLAMP;
// BEATO End
			continue;
		}
		// global clamp
		else if( !token.Icmp( "alphazeroclamp" ) )
		{
// BEATO Begin:
			sampWraping = vkSampler::WRAP_BORDER; // trpDefault = TR_CLAMP_TO_ZERO;
// BEATO End
			continue;
		}
		// forceOpaque is used for skies-behind-windows
		else if( !token.Icmp( "forceOpaque" ) )
		{
			coverage = MC_OPAQUE;
			continue;
		}
		// twoSided
		else if( !token.Icmp( "twoSided" ) )
		{
			cullType = CT_TWO_SIDED;

			// twoSided implies no-shadows, because the shadow
			// volume would be coplanar with the surface, giving depth fighting
			// we could make this no-self-shadows, but it may be more important
			// to receive shadows from no-self-shadow monsters
			if( !r_useShadowMapping.GetBool() ) // motorsep 11-08-2014; when shadow mapping is on, we allow two-sided surfaces to cast shadows 
				SetMaterialFlag( MF_NOSHADOWS );
// BEATO Begin: Pipeline configuration
			// m_pipelineInfo.faceCull = crPipeline::FC_TWO_FACES;
			// m_pipelineInfo.polygonModeFace = crPipeline::FC_TWO_FACES;
			// m_pipelineInfo.stencilFace = crPipeline::FC_TWO_FACES;
// BEATO End
		}
		// backSided
		else if( !token.Icmp( "backSided" ) )
		{
			cullType = CT_BACK_SIDED;

// BEATO Begin: Pipeline configuration
		// m_pipelineInfo.faceCull = crPipeline::FC_FRONT;
		// m_pipelineInfo.polygonModeFace = crPipeline::FC_FRONT;
		// m_pipelineInfo.stencilFace = crPipeline::FC_FRONT;
// BEATO End
			
			// the shadow code doesn't handle this, so just disable shadows.
			// We could fix this in the future if there was a need.
			SetMaterialFlag( MF_NOSHADOWS );
		}
		// foglight
		else if( !token.Icmp( "fogLight" ) )
		{
			fogLight = true;
			continue;
		}
		// blendlight
		else if( !token.Icmp( "blendLight" ) )
		{
			blendLight = true;
			continue;
		}
		// ambientLight
		else if( !token.Icmp( "ambientLight" ) )
		{
			ambientLight = true;
			continue;
		}
		// mirror
		else if( !token.Icmp( "mirror" ) )
		{
			sort = SS_SUBVIEW;
			coverage = MC_OPAQUE;
			continue;
		}
		// noFog
		else if( !token.Icmp( "noFog" ) )
		{
			noFog = true;
			continue;
		}
		// unsmoothedTangents
		else if( !token.Icmp( "unsmoothedTangents" ) )
		{
			unsmoothedTangents = true;
			continue;
		}
		// lightFallofImage <imageprogram>
		// specifies the image to use for the third axis of projected
		// light volumes
		else if( !token.Icmp( "lightFalloffImage" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr	copy;
			
			copy = str;	// so other things don't step on it
			
			lightFalloffSampler = new vkSampler();
			lightFalloffSampler->Create( vkSampler::FILTER_LINEAR, vkSampler::WRAP_BORDER, vkSampler::WRAP_BORDER, vkSampler::WRAP_BORDER );
			
			auto globalImages = idRenderSystem::GetGlobalImages();
			lightFalloffImage = globalImages->ImageFromFile( copy, TD_LIGHT );	// sikk - changed to TD_LIGHT (no compression), was TD_DEFAULT
			continue;
		}
		// guisurf <guifile> | guisurf entity
		// an entity guisurf must have an idUserInterface
		// specified in the renderEntity
		else if( !token.Icmp( "guisurf" ) )
		{
			src.ReadTokenOnLine( &token );
			if( !token.Icmp( "entity" ) )
			{
				entityGui = 1;
			}
			else if( !token.Icmp( "entity2" ) )
			{
				entityGui = 2;
			}
			else if( !token.Icmp( "entity3" ) )
			{
				entityGui = 3;
			}
			else
			{
				gui = uiManager->FindGui( token.c_str(), true );
			}
			continue;
		}
		// sort
		else if( !token.Icmp( "sort" ) )
		{
			ParseSort( src );
			continue;
		}
		else if( !token.Icmp( "stereoeye" ) )
		{
			ParseStereoEye( src );
			continue;
		}
		// spectrum <integer>
		else if( !token.Icmp( "spectrum" ) )
		{
			src.ReadTokenOnLine( &token );
			spectrum = atoi( token.c_str() );
			continue;
		}
		// deform < sprite | tube | flare >
		else if( !token.Icmp( "deform" ) )
		{
			ParseDeform( src );
			continue;
		}
		// decalInfo <staySeconds> <fadeSeconds> ( <start rgb> ) ( <end rgb> )
		else if( !token.Icmp( "decalInfo" ) )
		{
			ParseDecalInfo( src );
			continue;
		}
// BEATO Begin: Nowdays modeling tools have better bumpmap/normal generation options
#if 0
		// renderbump <args...>
		else if( !token.Icmp( "renderbump" ) )
		{
			src.ParseRestOfLine( renderBump );
			continue;
		}
#endif
// BEATO End
		// diffusemap for stage shortcut
		else if( !token.Icmp( "diffusemap" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::snPrintf( buffer, sizeof( buffer ), "blend diffusemap\nmap %s\n}\n", str );
			newSrc.LoadMemory( buffer, strlen( buffer ), "diffusemap" );
			newSrc.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
			ParseStage( newSrc );
			newSrc.FreeSource();
			continue;
		}
		// specularmap for stage shortcut
		else if( !token.Icmp( "specularmap" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::snPrintf( buffer, sizeof( buffer ), "blend specularmap\nmap %s\n}\n", str );
			newSrc.LoadMemory( buffer, strlen( buffer ), "specularmap" );
			newSrc.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
			ParseStage( newSrc );
			newSrc.FreeSource();
			continue;
		}
		// glossmap for stage shortcut
		else if( !token.Icmp( "glossmap" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::snPrintf( buffer, sizeof( buffer ), "blend glossmap\nmap %s\n}\n", str );
			newSrc.LoadMemory( buffer, strlen( buffer ), "glossmap" );
			newSrc.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
			ParseStage( newSrc );
			newSrc.FreeSource();
			continue;
		}
		// normalmap for stage shortcut
		else if( !token.Icmp( "bumpmap" ) )
		{
			str = R_ParsePastImageProgram( src );
			idStr::snPrintf( buffer, sizeof( buffer ), "blend bumpmap\nmap %s\n}\n", str );
			newSrc.LoadMemory( buffer, strlen( buffer ), "bumpmap" );
			newSrc.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
			ParseStage( newSrc );
			newSrc.FreeSource();
			continue;
		}
		// DECAL_MACRO for backwards compatibility with the preprocessor macros
		else if( !token.Icmp( "DECAL_MACRO" ) )
		{
			// polygonOffset
			SetMaterialFlag( MF_POLYGONOFFSET );
			polygonOffset = 1;
			
			// discrete
			surfaceFlags |= SURF_DISCRETE;
			contentFlags &= ~CONTENTS_SOLID;
			
			// sort decal
			sort = SS_DECAL;
			
			// noShadows
			SetMaterialFlag( MF_NOSHADOWS );
			continue;
		}

		// motorsep 11-23-2014; material LOD keys that define what LOD iteration the surface falls into
		else if( !token.Icmp( "lod1" ) )
		{
			SetMaterialFlag( MF_LOD1 );
			continue;
		}
		else if( !token.Icmp( "lod2" ) )
		{
			SetMaterialFlag( MF_LOD2 );
			continue;
		}
		else if( !token.Icmp( "lod3" ) )
		{
			SetMaterialFlag( MF_LOD3 );
			continue;
		}
		else if( !token.Icmp( "lod4" ) )
		{
			SetMaterialFlag( MF_LOD4 );
			continue;
		}
		else if( !token.Icmp( "persistentLOD" ) )
		{
			SetMaterialFlag( MF_LOD_PERSISTENT );
			continue;
		}
		else if( token == "{" )
		{
			// create the new stage
			ParseStage( src );
			continue;
		}
		else
		{
			common->Warning( "unknown general material parameter '%s' in '%s'", token.c_str(), GetName() );
			SetMaterialFlag( MF_DEFAULTED );
			return;
		}
	}
	
	// add _flat or _white stages if needed
	AddImplicitStages();
	
	// order the diffuse / bump / specular stages properly
	SortInteractionStages();
	
	// if we need to do anything with normals (lighting or environment mapping)
	// and two sided lighting was asked for, flag
	// shouldCreateBackSides() and change culling back to single sided,
	// so we get proper tangent vectors on both sides
	
	// we can't just call ReceivesLighting(), because the stages are still
	// in temporary form
	if( cullType == CT_TWO_SIDED )
	{
		//m_pipelineInfo.faceCull = crPipeline::FC_TWO_FACES;
		for( i = 0 ; i < pd->numStages ; i++ )
		{
			if( pd->parseStages[i].Lighting() != SL_AMBIENT || pd->parseStages[i].Texture().texgen != TG_EXPLICIT )
			{
				if( cullType == CT_TWO_SIDED )
				{
					cullType = CT_FRONT_SIDED;
					shouldCreateBackSides = true;
				}
				break;
			}
		}

	}
	
	// currently a surface can only have one unique texgen for all the stages on old hardware
	texgen_t firstGen = TG_EXPLICIT;
	for( i = 0; i < pd->numStages; i++ )
	{
		if( pd->parseStages[i].Texture().texgen != TG_EXPLICIT )
		{
			if( firstGen == TG_EXPLICIT )
			{
				firstGen = pd->parseStages[i].Texture().texgen;
			}
			else if( firstGen != pd->parseStages[i].Texture().texgen )
			{
				common->Warning( "material '%s' has multiple stages with a texgen", GetName() );
				break;
			}
		}
	}
}

/*
=========================
idMaterial::SetGui
=========================
*/
void idMaterial::SetGui( const char* _gui ) const
{
	gui = uiManager->FindGui( _gui, true, false, true );
}

/*
=========================
idMaterial::Parse

Parses the current material definition and finds all necessary images.
=========================
*/
bool idMaterial::Parse( const char* text, const int textLength, bool allowBinaryVersion )
{
	idLexer	src;
	idToken	token;
	mtrParsingData_t parsingData;
	
	src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( DECL_LEXER_FLAGS );
	src.SkipUntilString( "{" );
	
	// reset to the unparsed state
	CommonInit();
	
	std::memset( &parsingData, 0, sizeof( parsingData ) );
	
	pd = &parsingData;	// this is only valid during parse
	
	// parse it
	ParseMaterial( src );
	
	// if we are doing an fs_copyfiles, also reference the editorImage
	if( cvarSystem->GetCVarInteger( "fs_copyFiles" ) )
		GetEditorImage();
	
	//
	// count non-lit stages
	numAmbientStages = 0;
	int i;
	for( i = 0 ; i < stages.Num(); i++ )
	{
		if( pd->parseStages[i].Lighting() == SL_AMBIENT )
			numAmbientStages++;
	}
	
	// see if there is a subview stage
	if( sort == SS_SUBVIEW )
	{
		hasSubview = true;
	}
	else
	{
		hasSubview = false;
		for( i = 0 ; i < stages.Num(); i++ )
		{
			if( pd->parseStages[i].Texture().dynamic )
				hasSubview = true;
		}
	}
	
	// automatically determine coverage if not explicitly set
	if( coverage == MC_BAD )
	{
		// automatically set MC_TRANSLUCENT if we don't have any interaction stages and
		// the first stage is blended and not an alpha test mask or a subview
		if( !pd->numStages )
			// non-visible
			coverage = MC_TRANSLUCENT;
		
		else if( pd->numStages != numAmbientStages )
			// we have an interaction draw
			coverage = MC_OPAQUE;
		else if(
			( pd->parseStages[0].DrawStateBits() & GLS_DSTBLEND_BITS ) != GLS_DSTBLEND_ZERO ||
			( pd->parseStages[0].DrawStateBits() & GLS_SRCBLEND_BITS ) == GLS_SRCBLEND_DST_COLOR ||
			( pd->parseStages[0].DrawStateBits() & GLS_SRCBLEND_BITS ) == GLS_SRCBLEND_ONE_MINUS_DST_COLOR ||
			( pd->parseStages[0].DrawStateBits() & GLS_SRCBLEND_BITS ) == GLS_SRCBLEND_DST_ALPHA ||
			( pd->parseStages[0].DrawStateBits() & GLS_SRCBLEND_BITS ) == GLS_SRCBLEND_ONE_MINUS_DST_ALPHA
		)
			// blended with the destination
			coverage = MC_TRANSLUCENT;
		else
			coverage = MC_OPAQUE;
	}
	
	// translucent automatically implies noshadows
	if( coverage == MC_TRANSLUCENT )
		SetMaterialFlag( MF_NOSHADOWS );
	else
		contentFlags |= CONTENTS_OPAQUE; // mark the contents as opaque
	
	// if we are translucent, draw with an alpha in the editor
	if( coverage == MC_TRANSLUCENT )
		editorAlpha = 0.5;
	else
		editorAlpha = 1.0;
	
	// the sorts can make reasonable defaults
	if( sort == SS_BAD )
	{
		if( TestMaterialFlag( MF_POLYGONOFFSET ) )
			sort = SS_DECAL;
		else if( coverage == MC_TRANSLUCENT )
			sort = SS_MEDIUM;
		else
			sort = SS_OPAQUE;
	}
	
	// anything that references _currentRender will automatically get sort = SS_POST_PROCESS
	// and coverage = MC_TRANSLUCENT
	idImageManagerLocal* globalImages = static_cast<idImageManagerLocal*>( idRenderSystem::GetGlobalImages() );

	for( i = 0 ; i < pd->numStages; i++ )
	{
		crShaderStage*	pStage = &pd->parseStages[i];
		if( pStage->Texture().image == globalImages->originalCurrentRenderImage )
		{
			if( sort != SS_PORTAL_SKY )
			{
				sort = SS_POST_PROCESS;
				coverage = MC_TRANSLUCENT;
			}
			break;
		}

		for( int j = 0 ; j < pStage->NumFragmentProgramImages() ; j++ )
		{
			if( pStage->FragmentProgramImages()[j] == globalImages->originalCurrentRenderImage )
			{
				if( sort != SS_PORTAL_SKY )
				{
					sort = SS_POST_PROCESS;
					coverage = MC_TRANSLUCENT;
				}

				i = pd->numStages;
				break;
			}
		}
	}
	
	// set the drawStateBits depth flags
	for( i = 0 ; i < pd->numStages ; i++ )
	{
		crShaderStage*	pStage = &pd->parseStages[i];
		if( sort == SS_POST_PROCESS )
			// post-process effects fill the depth buffer as they draw, so only the
			// topmost post-process effect is rendered
			pStage->SetDrawStateBits( GLS_DEPTHFUNC_LESS ); // GLS_DEPTHFUNC_LESS;
		else if( coverage == MC_TRANSLUCENT || pStage->IgnoreAlphaTest() )
			// translucent surfaces can extend past the exactly marked depth buffer
			pStage->SetDrawStateBits( GLS_DEPTHFUNC_LESS | GLS_DEPTHMASK ); // GLS_DEPTHFUNC_LESS | GLS_DEPTHMASK;
		else
			// opaque and perforated surfaces must exactly match the depth buffer,
			// which gets alpha test correct
			pStage->SetDrawStateBits( GLS_DEPTHFUNC_EQUAL | GLS_DEPTHMASK ); // GLS_DEPTHFUNC_EQUAL | GLS_DEPTHMASK;
	}
	
	// determine if this surface will accept overlays / decals
	
	if( pd->forceOverlays )
	{
		// explicitly flaged in material definition
		allowOverlays = true;
	}
	else
	{
		if( !IsDrawn() )
			allowOverlays = false;
		
		if( Coverage() != MC_OPAQUE )
			allowOverlays = false;
		
		if( GetSurfaceFlags() & SURF_NOIMPACT )
			allowOverlays = false;
	}
	
	// add a tiny offset to the sort orders, so that different materials
	// that have the same sort value will at least sort consistantly, instead
	// of flickering back and forth
	/* this messed up in-game guis
		if ( sort != SS_SUBVIEW ) {
			int	hash, l;
	
			l = name.Length();
			hash = 0;
			for ( int i = 0 ; i < l ; i++ ) {
				hash ^= name[i];
			}
			sort += hash * 0.01;
		}
	*/
	
	if( pd->numStages )
	{
		//stages = ( shaderStage_t* )R_StaticAlloc( numStages * sizeof( stages[0] ), TAG_MATERIAL );
		stages.Resize( pd->numStages );
		std::memcpy( stages.Ptr(), pd->parseStages, pd->numStages * sizeof( crShaderStage ) );
	}
	
	if( numOps )
	{
		ops = ( expOp_t* )R_StaticAlloc( numOps * sizeof( ops[0] ), TAG_MATERIAL );
		std::memcpy( ops, pd->shaderOps, numOps * sizeof( ops[0] ) );
	}
	
	if( numRegisters )
	{
		expressionRegisters = ( float* )R_StaticAlloc( numRegisters * sizeof( expressionRegisters[0] ), TAG_MATERIAL );
		std::memcpy( expressionRegisters, pd->shaderRegisters, numRegisters * sizeof( expressionRegisters[0] ) );
	}
	
	// see if the registers are completely constant, and don't need to be evaluated
	// per-surface
	CheckForConstantRegisters();
	
	// See if the material is trivial for the fast path
	SetFastPathImages();
	
	pd = nullptr;	// the pointer will be invalid after exiting this function
	
	// finish things up
	if( TestMaterialFlag( MF_DEFAULTED ) )
	{
		MakeDefault();
		return false;
	}
	return true;
}

/*
===================
idMaterial::Print
===================
*/
const char* opNames[] =
{
	"OP_TYPE_ADD",
	"OP_TYPE_SUBTRACT",
	"OP_TYPE_MULTIPLY",
	"OP_TYPE_DIVIDE",
	"OP_TYPE_MOD",
	"OP_TYPE_TABLE",
	"OP_TYPE_TABLE2D",
	"OP_TYPE_GT",
	"OP_TYPE_GE",
	"OP_TYPE_LT",
	"OP_TYPE_LE",
	"OP_TYPE_EQ",
	"OP_TYPE_NE",
	"OP_TYPE_AND",
	"OP_TYPE_OR"
};

void idMaterial::Print( void ) const
{
	int			i;
	
	for( i = EXP_REG_NUM_PREDEFINED ; i < GetNumRegisters() ; i++ )
	{
		common->Printf( "register %i: %f\n", i, expressionRegisters[i] );
	}
	common->Printf( "\n" );
	for( i = 0 ; i < numOps ; i++ )
	{
		const expOp_t* op = &ops[i];
		if ( op->opType == OP_TYPE_TABLE || op->opType == OP_TYPE_TABLE2D )
			common->Printf( "%i = %s[ %i ]\n", op->c, declManager->DeclByIndex( DECL_TABLE, op->a )->GetName(), op->b );
		else
			common->Printf( "%i = %i %s %i\n", op->c, op->a, opNames[ op->opType ], op->b );
		
	}
}

/*
===============
idMaterial::Save
===============
*/
bool idMaterial::Save( const char* fileName )
{
	return ReplaceSourceFileText();
}

/*
===============
idMaterial::AddReference
===============
*/
void idMaterial::AddReference( void )
{
	refCount++;
	
	for( int i = 0; i < stages.Num(); i++ )
	{
		crShaderStage* s = &stages[i];
		
		if( s->Texture().image )
			s->Texture().image->AddReference();
		
	}
}

/*
===============
idMaterial::EvaluateRegisters

Parameters are taken from the localSpace and the renderView,
then all expressions are evaluated, leaving the material registers
set to their apropriate values.
===============
*/
void idMaterial::EvaluateRegisters(
	float* 			registers,
	const float		localShaderParms[MAX_ENTITY_SHADER_PARMS],
	const float		globalShaderParms[MAX_GLOBAL_SHADER_PARMS],
	const float		floatTime,
	idSoundEmitter* soundEmitter ) const
{

	int		i, b;
	expOp_t*	op;
	
	// copy the material constants
	for( i = EXP_REG_NUM_PREDEFINED ; i < numRegisters ; i++ )
	{
		registers[i] = expressionRegisters[i];
	}
	
	// copy the local and global parameters
	registers[EXP_REG_TIME] = floatTime;
	registers[EXP_REG_PARM0] = localShaderParms[0];
	registers[EXP_REG_PARM1] = localShaderParms[1];
	registers[EXP_REG_PARM2] = localShaderParms[2];
	registers[EXP_REG_PARM3] = localShaderParms[3];
	registers[EXP_REG_PARM4] = localShaderParms[4];
	registers[EXP_REG_PARM5] = localShaderParms[5];
	registers[EXP_REG_PARM6] = localShaderParms[6];
	registers[EXP_REG_PARM7] = localShaderParms[7];
	registers[EXP_REG_PARM8] = localShaderParms[8];
	registers[EXP_REG_PARM9] = localShaderParms[9];
	registers[EXP_REG_PARM10] = localShaderParms[10];
	registers[EXP_REG_PARM11] = localShaderParms[11];
	registers[EXP_REG_GLOBAL0] = globalShaderParms[0];
	registers[EXP_REG_GLOBAL1] = globalShaderParms[1];
	registers[EXP_REG_GLOBAL2] = globalShaderParms[2];
	registers[EXP_REG_GLOBAL3] = globalShaderParms[3];
	registers[EXP_REG_GLOBAL4] = globalShaderParms[4];
	registers[EXP_REG_GLOBAL5] = globalShaderParms[5];
	registers[EXP_REG_GLOBAL6] = globalShaderParms[6];
	registers[EXP_REG_GLOBAL7] = globalShaderParms[7];
	
	op = ops;
	for( i = 0 ; i < numOps ; i++, op++ )
	{
		switch( op->opType )
		{
			case OP_TYPE_ADD:
				registers[op->c] = registers[op->a] + registers[op->b];
				break;
			case OP_TYPE_SUBTRACT:
				registers[op->c] = registers[op->a] - registers[op->b];
				break;
			case OP_TYPE_MULTIPLY:
				registers[op->c] = registers[op->a] * registers[op->b];
				break;
			case OP_TYPE_DIVIDE:
				registers[op->c] = registers[op->a] / registers[op->b];
				break;
			case OP_TYPE_MOD:
				b = ( int )registers[op->b];
				b = b != 0 ? b : 1;
				registers[op->c] = ( int )registers[op->a] % b;
				break;
			case OP_TYPE_TABLE:
			{
				const idDeclTable* table = static_cast<const idDeclTable*>( declManager->DeclByIndex( DECL_TABLE, op->a ) );
				registers[op->c] = table->TableLookup( registers[op->b] );
				break;
			}
			case OP_TYPE_TABLE2D:
			{
				const idDeclTable2d* table = static_cast<const idDeclTable2d*>( declManager->DeclByIndex( DECL_TABLE2D, op->a ) );
				registers[ op->c ] = table->TableLookup( registers[ op->b ] );
				break;
			}
			case OP_TYPE_SOUND:
				if( r_forceSoundOpAmplitude.GetFloat() > 0 )
					registers[op->c] = r_forceSoundOpAmplitude.GetFloat();
				else if( soundEmitter )
					registers[op->c] = soundEmitter->CurrentAmplitude();
				else
					registers[op->c] = 0;
				
				break;
			case OP_TYPE_GT:
				registers[op->c] = registers[ op->a ] > registers[op->b];
				break;
			case OP_TYPE_GE:
				registers[op->c] = registers[ op->a ] >= registers[op->b];
				break;
			case OP_TYPE_LT:
				registers[op->c] = registers[ op->a ] < registers[op->b];
				break;
			case OP_TYPE_LE:
				registers[op->c] = registers[ op->a ] <= registers[op->b];
				break;
			case OP_TYPE_EQ:
				registers[op->c] = registers[ op->a ] == registers[op->b];
				break;
			case OP_TYPE_NE:
				registers[op->c] = registers[ op->a ] != registers[op->b];
				break;
			case OP_TYPE_AND:
				registers[op->c] = registers[ op->a ] && registers[op->b];
				break;
			case OP_TYPE_OR:
				registers[op->c] = registers[ op->a ] || registers[op->b];
				break;
			default:
				common->FatalError( "R_EvaluateExpression: bad opcode" );
		}
	}
	
}

/*
=============
idMaterial::Texgen
=============
*/
texgen_t idMaterial::Texgen( void ) const
{
	if( !stages.IsEmpty() )
	{
		for( int i = 0; i < stages.Num(); i++ )
		{
			if( stages[ i ].Texture().texgen != TG_EXPLICIT )
				return stages[ i ].Texture().texgen;
		}
	}
	
	return TG_EXPLICIT;
}

/*
=============
idMaterial::GetImageWidth
=============
*/
int idMaterial::GetImageWidth( void ) const
{
	assert( GetStage( 0 ) && GetStage( 0 )->Texture().image );
	return GetStage( 0 )->Texture().image->GetUploadWidth();
}

/*
=============
idMaterial::GetImageHeight
=============
*/
int idMaterial::GetImageHeight( void ) const
{
	assert( GetStage( 0 ) && GetStage( 0 )->Texture().image );
	return GetStage( 0 )->Texture().image->GetUploadHeight();
}

/*
=============
idMaterial::CinematicLength
=============
*/
int	idMaterial::CinematicLength( void ) const
{
	if( stages.IsEmpty() || !stages[0].Texture().cinematic )
		return 0;
	
	return stages[0].Texture().cinematic->AnimationLength();
}

/*
=============
idMaterial::UpdateCinematic
=============
*/
void idMaterial::UpdateCinematic( int time ) const
{
}

/*
=============
idMaterial::CloseCinematic
=============
*/
void idMaterial::CloseCinematic( void ) const
{
	for( int i = 0; i < stages.Num(); i++ )
	{
		if( stages[i].Texture().cinematic )
		{
			stages[i].Texture().cinematic->Close();
			delete stages[i].Texture().cinematic;
			stages[i].Texture().cinematic = nullptr;
		}
	}
}

/*
=============
idMaterial::ResetCinematicTime
=============
*/
void idMaterial::ResetCinematicTime( int time ) const
{
	for( int i = 0; i < stages.Size(); i++ )
	{
		if( stages[i].Texture().cinematic )
			stages[i].Texture().cinematic->ResetTime( time );
		
	}
}

/*
=============
idMaterial::GetCinematicStartTime
=============
*/
int idMaterial::GetCinematicStartTime( void ) const
{
	for( int i = 0; i < stages.Size(); i++ )
	{
		if( stages[i].Texture().cinematic )
			return stages[i].Texture().cinematic->GetStartTime();
		
	}
	return -1;
}

/*
==================
idMaterial::CheckForConstantRegisters

As of 5/2/03, about half of the unique materials loaded on typical
maps are constant, but 2/3 of the surface references are.
==================
*/
void idMaterial::CheckForConstantRegisters()
{
	assert( constantRegisters == nullptr );
	
	if( !pd->registersAreConstant )
		return;
	
	if( !r_useConstantMaterials.GetBool() )
		return;
	
	// evaluate the registers once, and save them
	constantRegisters = ( float* )R_ClearedStaticAlloc( GetNumRegisters() * sizeof( float ) );
	
	float shaderParms[MAX_ENTITY_SHADER_PARMS];
	std::memset( shaderParms, 0, sizeof( shaderParms ) );
	viewDef_t	viewDef;
	std::memset( &viewDef, 0, sizeof( viewDef ) );
	
	EvaluateRegisters( constantRegisters, shaderParms, viewDef.renderView.shaderParms, 0.0f, 0 );
}

/*
===================
idMaterial::ImageName
===================
*/
const char* idMaterial::ImageName( void ) const
{
	if( stages.Num() == 0 )
		return "_scratch";
	
	idImage*	image = stages[0].Texture().image;
	if( image )
		return image->GetName();
	
	return "_scratch";
}

/*
=================
idMaterial::Size
=================
*/
constexpr size_t MATERIAL_SIZE = sizeof( idMaterial );
size_t idMaterial::Size( void ) const
{
	return MATERIAL_SIZE;
}

/*
===================
idMaterial::SetDefaultText
===================
*/
bool idMaterial::SetDefaultText( void )
{
	// if there exists an image with the same name
	if( 1 )    //fileSystem->ReadFile( GetName(), nullptr ) != -1 ) {
	{
		char generated[2048];
		idStr::snPrintf( generated, sizeof( generated ),
						 "material %s // IMPLICITLY GENERATED\n"
						 "{\n"
						 "{\n"
						 "blend blend\n"
						 "colored\n"
						 "map \"%s\"\n"
						 "clamp\n"
						 "}\n"
						 "}\n", GetName(), GetName() );
		SetText( generated );
		return true;
	}
	else
	{
		return false;
	}
}

/*
===================
idMaterial::DefaultDefinition
===================
*/
const char* idMaterial::DefaultDefinition( void ) const
{
	return
		"{\n"
		"\t"	"{\n"
		"\t\t"		"blend\tblend\n"
		"\t\t"		"map\t\t_default\n"
		"\t"	"}\n"
		"}";
}


/*
===================
idMaterial::GetBumpStage
===================
*/
const shaderStage_t* idMaterial::GetBumpStage( void ) const
{
	for( int i = 0 ; i < stages.Num() ; i++ )
	{
		if( stages[i].Lighting() == SL_BUMP )
			return &stages[i];
		
	}
	return nullptr;
}

/*
===================
idMaterial::ReloadImages
===================
*/
void idMaterial::ReloadImages( bool force ) const
{
	for( int i = 0 ; i < stages.Size() ; i++ )
	{
/// BEATO Begin:
		if( stages[i].NumFragmentProgramImages() > 0 )
		{
			for( int j = 0 ; j < stages[i].NumFragmentProgramImages() ; j++ )
			{
				if( stages[i].FragmentProgramImages()[j] )
					stages[i].FragmentProgramImages()[j]->Reload( force );
			}
		}
/// BEATO End
		else if( stages[i].Texture().image )
		{
			stages[i].Texture().image->Reload( force );
		}
	}
}

/*
=============
idMaterial::SetFastPathImages

See if the material is trivial for the fast path
=============
*/
void idMaterial::SetFastPathImages( void )
{
	fastPathBumpImage = nullptr;
	fastPathDiffuseImage = nullptr;
	fastPathSpecularImage = nullptr;
	fastPathGlossImage = nullptr;
	idImageManagerLocal* globalImages = static_cast<idImageManagerLocal*>( idRenderSystem::GetGlobalImages() );
	
	if( constantRegisters == nullptr )
		return;
	
	// go through the individual surface stages
	//
	// We also have the very rare case of some materials that have conditional interactions
	// for the "hell writing" that can be shined on them.
	for( int surfaceStageNum = 0; surfaceStageNum < GetNumStages(); surfaceStageNum++ )
	{
		const crShaderStage* surfaceStage = GetStage( surfaceStageNum );
		
		if( surfaceStage->Texture().hasMatrix )
			goto fail;
		
		// check for vertex coloring
		if( surfaceStage->StageVertexColor() != SVC_IGNORE )
			goto fail;
		
		// check for non-identity colors
		for( int i = 0; i < 4; i++ )
		{
			if( idMath::Fabs( constantRegisters[surfaceStage->color.registers[i]] - 1.0f ) > 0.1f )
				goto fail;
		}
		
		switch( surfaceStage->Lighting() )
		{
			case SL_COVERAGE:
			case SL_AMBIENT:
				break;
			case SL_BUMP:
			{
				if( fastPathBumpImage )
					goto fail;
				
				fastPathBumpImage = surfaceStage->Texture().image;
				break;
			}
			case SL_DIFFUSE:
			{
				if( fastPathDiffuseImage )
					goto fail;
				
				fastPathDiffuseImage = surfaceStage->Texture().image;
				break;
			}
			case SL_SPECULAR:
			{
				if( fastPathSpecularImage )
					goto fail;

				fastPathSpecularImage = surfaceStage->Texture().image;
			}
			case SL_GLOSS:
			{
				if( fastPathGlossImage )
					goto fail;
	
				fastPathGlossImage = surfaceStage->Texture().image;
			}
		}
	}
	// need a bump image, but specular can default
	// we also need a diffuse image, because we can't get a pure black with our YCoCg conversion
	// from 565 DXT.  The general-path code also sets the diffuse color to 0 in the default case,
	// but the fast path can't.
	if( fastPathBumpImage == nullptr || fastPathDiffuseImage == nullptr )
		goto fail;
	
	if( fastPathSpecularImage == nullptr )
		fastPathSpecularImage = globalImages->blackImage;

	if( fastPathGlossImage == nullptr )
		fastPathGlossImage = globalImages->glossImage;
	
	return;
	
fail:
	fastPathBumpImage = nullptr;
	fastPathDiffuseImage = nullptr;
	fastPathSpecularImage = nullptr;
	fastPathGlossImage = nullptr;
}
