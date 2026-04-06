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
#include "framework/Common_local.h"
#include "Backend.hpp"

idCVar r_drawFlickerBox( "r_drawFlickerBox", "0", CVAR_RENDERER | CVAR_BOOL, "visual test for dropping frames" );
idCVar stereoRender_warp( "stereoRender_warp", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "use the optical warping renderprog instead of stereoDeGhost" );
idCVar stereoRender_warpStrength( "stereoRender_warpStrength", "1.45", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "amount of pre-distortion" );

idCVar stereoRender_warpCenterX( "stereoRender_warpCenterX", "0.5", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "center for left eye, right eye will be 1.0 - this" );
idCVar stereoRender_warpCenterY( "stereoRender_warpCenterY", "0.5", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "center for both eyes" );
idCVar stereoRender_warpParmZ( "stereoRender_warpParmZ", "0", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "development parm" );
idCVar stereoRender_warpParmW( "stereoRender_warpParmW", "0", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "development parm" );
idCVar stereoRender_warpTargetFraction( "stereoRender_warpTargetFraction", "1.0", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "fraction of half-width the through-lens view covers" );

idCVar r_motionBlur( "r_motionBlur", "0", CVAR_RENDERER | CVAR_INTEGER | CVAR_ARCHIVE, "1 - 5, log2 of the number of motion blur samples" );
idCVar r_glowEnable( "r_glowEnable", "0", CVAR_RENDERER | CVAR_BOOL | CVAR_ARCHIVE, "enables Bloom effect for materials that use a glow pass" );
idCVar r_useHightQualitySky( "r_useHightQualitySky", "0", CVAR_BOOL | CVAR_ARCHIVE, "Use high quality skyboxes" );

static const idVec4 zero = idVec4( 0.0f, 0.0f, 0.0f, 0.0f );
static const idVec4 one = idVec4( 1.0f, 1.0f, 1.0f, 1.0f );
static const idVec4 negOne = idVec4( -1.0f, -1.0f, -1.0f, -1.0f );

/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

crBackend::crBackend( void )
{
}

crBackend::~crBackend( void )
{
}

crBackend *crBackend::Get(void)
{
	static crBackend backEnd = crBackend();
    return &backEnd;
}

bool crBackend::StartUp( const uint8_t in_samples, const uint32_t in_width, const uint32_t in_heigth )
{	
	m_numBuffers = std::min( static_cast<uint32_t>( r_bufferCount.GetInteger() ), MAX_SMP_FRAMES );

	m_defaultFB = new crFramebuffer();
	m_defaultFB->Create( { in_width, in_heigth, 0, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_D24_UNORM_S8_UINT }, m_numBuffers );

	return true;
}

bool crBackend::SetScreenParms( const uint8_t in_samples, const uint32_t in_width, const uint32_t in_heigth )
{
	// TODO: RECONFIGURE SWAP CHAIN AND FRAME BUFFERS
	return false;
}

void crBackend::ShutDown(void)
{
	if ( m_defaultFB != nullptr )
	{
		m_defaultFB->Destroy();
		delete m_defaultFB;
		m_defaultFB = nullptr;
	}
}


/*
=============
crBackend::DrawFlickerBox
=============
*/
void crBackend::DrawFlickerBox( void )
{
#if 0
	if( !r_drawFlickerBox.GetBool() )
		return;
	
	if( tr.frameCount & 1 )
		glClearColor( 1, 0, 0, 1 );

	else
		glClearColor( 0, 1, 0, 1 );

	glScissor( 0, 0, 256, 256 );
	glClear( GL_COLOR_BUFFER_BIT );
#endif
}

/*
=============
crBackend::SetBuffer
=============
*/
void crBackend::SetBuffer( const void* data )
{
	// see which draw buffer we want to render the frame to
	const setBufferCommand_t* cmd = ( const setBufferCommand_t* )data;
	uint32_t bufferID = m_frame % SMP_FRAMES;
	
	RENDERLOG_PRINTF( "---------- RB_SetBuffer ---------- to buffer # %d\n", bufferID );

	/// It waits for the resources of the previous paired frame to be released,
	/// acquires and prepares the presentation image state, clears the command buffer, 
	/// and initializes command recording for the frame.
	//tr.Swapchain()->AcquireImage( bufferID );
	
#if 0 // TODO: We gona bind the present image at end of rendering chain
	auto presentImage = m_swapchain->Image();

	/// Before use the SwapChainImage, we need to perform a state transition
	presentImage->layout = VK_IMAGE_LAYOUT_UNDEFINED;
	presentImage->stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; // Come from SO
	presentImage->access = VK_ACCESS_2_NONE;
	VkImageStateTransition( presentImage, m_graphicCommandBuffer->CommandBuffer(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT );

	/// jut clear images
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO; 
    colorAttachment.pNext = nullptr; 
    colorAttachment.imageView = *m_swapchain->Image();
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE; 
    colorAttachment.resolveImageView = nullptr;
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = m_clearValues;

	VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.flags = 0;
    renderingInfo.renderArea = { 0, 0, tr.GetWidth(), tr.GetHeight() };
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = nullptr;
    renderingInfo.pStencilAttachment = nullptr;
    vkCmdBeginRendering( m_graphicCommandBuffer->CommandBuffer(), &renderingInfo );
	/// End frame rendering
    vkCmdEndRendering( m_graphicCommandBuffer->CommandBuffer() );
#endif

	m_defaultFB->Bind();

	/// Reset Scissor
	Scissor( 0, 0, tr.GetWidth(), tr.GetHeight() );
	
	// clear screen for debugging
	// automatically enable this with several other debug tools
	// that might leave unrendered portions of the screen
	if( r_clear.GetFloat() || idStr::Length( r_clear.GetString() ) != 1 || r_singleArea.GetBool() || r_showOverDraw.GetBool() )
	{
		float c[3];
		if( sscanf( r_clear.GetString(), "%f %f %f", &c[0], &c[1], &c[2] ) == 3 )
			Clear( true, false, false, 0, c[0], c[1], c[2], 1.0f );
		else if( r_clear.GetInteger() == 2 )
			Clear( true, false, false, 0, 0.0f, 0.0f,  0.0f, 1.0f );
		else if( r_showOverDraw.GetBool() )
			Clear( true, false, false, 0, 1.0f, 1.0f, 1.0f, 1.0f );
		else
			Clear( true, false, false, 0, 0.4f, 0.0f, 0.25f, 1.0f );
	}

}

/*
====================
R_MakeStereoRenderImage
====================
*/
static void R_MakeStereoRenderImage( idImage* image )
{
	idImageOpts	opts;
	opts.width = tr.GetWidth();
	opts.height = tr.GetHeight();
	opts.numLevels = 1;
	opts.format = crInternalFormat::RGBA8U;
	image->AllocImage( opts );
}

/*
====================
crBackend::StereoRenderExecuteBackEndCommands

Renders the draw list twice, with slight modifications for left eye / right eye
====================
*/
#if 0
void crBackend::StereoRenderExecuteBackEndCommands( const emptyCommand_t* const allCmds )
{
	uint64_t backEndStartTime = Sys_Microseconds();
	
	// If we are in a monoscopic context, this draws to the only buffer, and is
	// the same as GL_BACK.  In a quad-buffer stereo context, this is necessary
	// to prevent GL from forcing the rendering to go to both BACK_LEFT and
	// BACK_RIGHT at a performance penalty.
	// To allow stereo deghost processing, the views have to be copied to separate
	// textures anyway, so there isn't any benefit to rendering to BACK_RIGHT for
	// that eye.
	
	auto globalImages = dynamic_cast<idImageManagerLocal*>( idRenderSystem::GetGlobalImages() );

	// create the stereoRenderImage if we haven't already
	static idImage* stereoRenderImages[2];
	for( int i = 0; i < 2; i++ )
	{
		if( stereoRenderImages[i] == nullptr )
			stereoRenderImages[i] = globalImages->ImageFromFunction( va( "_stereoRender%i", i ), R_MakeStereoRenderImage );
		
		// resize the stereo render image if the main window has changed size
		if( stereoRenderImages[i]->GetUploadWidth() != tr.GetWidth() ||
				stereoRenderImages[i]->GetUploadHeight() != tr.GetHeight() )
		{
			stereoRenderImages[i]->Resize( tr.GetWidth(), tr.GetHeight() );
		}
	}
	
	// In stereoRender mode, the front end has generated two RC_DRAW_VIEW commands
	// with slightly different origins for each eye.
	
	// TODO: only do the copy after the final view has been rendered, not mirror subviews?
	
	// Render the 3D draw views from the screen origin so all the screen relative
	// texture mapping works properly, then copy the portion we are going to use
	// off to a texture.
	bool foundEye[2] = { false, false };
	
	for( int stereoEye = 1; stereoEye >= -1; stereoEye -= 2 )
	{
		// set up the target texture we will draw to
		const int targetEye = ( stereoEye == 1 ) ? 1 : 0;
		
		// Set the back end into a known default state to fix any stale render state issues
		SetDefaultState();
		
		for( const emptyCommand_t* cmds = allCmds; cmds != nullptr; cmds = ( const emptyCommand_t* )cmds->next )
		{
			switch( cmds->commandId )
			{
				case RC_NOP:
					break;
				case RC_DRAW_VIEW_GUI:
				case RC_DRAW_VIEW_3D:
				{
					const drawSurfsCommand_t* const dsc = ( const drawSurfsCommand_t* )cmds;
					const viewDef_t&			eyeViewDef = *dsc->viewDef;
					
					if( eyeViewDef.renderView.viewEyeBuffer && eyeViewDef.renderView.viewEyeBuffer != stereoEye )
						// this is the render view for the other eye
						continue;
					
					foundEye[ targetEye ] = true;
					DrawView( dsc, stereoEye );
					if( cmds->commandId == RC_DRAW_VIEW_GUI )
					{
					}
				}
				break;

				case RC_SET_BUFFER:
					SetBuffer( cmds );
					break;
				case RC_COPY_RENDER:
					CopyRender( cmds );
					break;
				case RC_POST_PROCESS:
				{
					postProcessCommand_t* cmd = ( postProcessCommand_t* )cmds;
					if( cmd->viewDef->renderView.viewEyeBuffer != stereoEye )
						break;
					
					PostProcess( cmds );
				}
				break;
				default:
					common->Error( "RB_ExecuteBackEndCommands: bad commandId" );
					break;
			}
		}
		
		// copy to the target
		stereoRenderImages[ targetEye ]->CopyFramebuffer( 0, 0, tr.GetWidth(), tr.GetHeight() );
	}
	
	// perform the final compositing / warping / deghosting to the actual framebuffer(s)
	assert( foundEye[0] && foundEye[1] );
	
	SetDefaultState();
	
	SetMVP( renderMatrix_identity );
	
	// If we are in quad-buffer pixel format but testing another 3D mode,
	// make sure we draw to both eyes.  This is likely to be sub-optimal
	// performance on most cards and drivers, but it is better than getting
	// a confusing, half-ghosted view.
	if( tr.GetStereo3DMode() != STEREO3D_QUAD_BUFFER )
	{
		glDrawBuffer( GL_BACK );
	}
	
	// GL_State( GLS_DEPTHFUNC_ALWAYS );
	Cull( CT_TWO_SIDED );
	
	// We just want to do a quad pass - so make sure we disable any texgen and
	// set the texture matrix to the identity so we don't get anomalies from
	// any stale uniform data being present from a previous draw call
	const float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	const float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_S, texS );
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_T, texT );
	
	// disable any texgen
	const float texGenEnabled[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled );
	
	renderProgManager.BindShader_Texture();
	GL_Color( 1, 1, 1, 1 );
	
	switch( tr.GetStereo3DMode() )
	{
		case STEREO3D_QUAD_BUFFER:
			glDrawBuffer( GL_BACK_RIGHT );
			SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			DrawElementsWithCounters( &unitSquareSurface );
			
			glDrawBuffer( GL_BACK_LEFT );
			SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			DrawElementsWithCounters( &unitSquareSurface );
			
			break;
		case STEREO3D_HDMI_720:
			// HDMI 720P 3D
			SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			ViewportAndScissor( 0, 0, 1280, 720 );
			DrawElementsWithCounters( &unitSquareSurface );
			
			SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			ViewportAndScissor( 0, 750, 1280, 720 );
			DrawElementsWithCounters( &unitSquareSurface );
			
			// force the HDMI 720P 3D guard band to a constant color
			glScissor( 0, 720, 1280, 30 );
			glClear( GL_COLOR_BUFFER_BIT );
			break;
		default:
		case STEREO3D_SIDE_BY_SIDE:
			if( stereoRender_warp.GetBool() )
			{
				// this is the Rift warp
				// renderSystem->GetWidth() / GetHeight() have returned equal values (640 for initial Rift)
				// and we are going to warp them onto a symetric square region of each half of the screen
				
				renderProgManager.BindShader_StereoWarp();
				
				// clear the entire screen to black
				// we could be smart and only clear the areas we aren't going to draw on, but
				// clears are fast...
				glScissor( 0, 0, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight );
				glClearColor( 0, 0, 0, 0 );
				glClear( GL_COLOR_BUFFER_BIT );
				
				// the size of the box that will get the warped pixels
				// With the 7" displays, this will be less than half the screen width
				const int pixelDimensions = ( glConfig.nativeScreenWidth >> 1 ) * stereoRender_warpTargetFraction.GetFloat();
				
				// Always scissor to the half-screen boundary, but the viewports
				// might cross that boundary if the lenses can be adjusted closer
				// together.
				glViewport( ( glConfig.nativeScreenWidth >> 1 ) - pixelDimensions,
							( glConfig.nativeScreenHeight >> 1 ) - ( pixelDimensions >> 1 ),
							pixelDimensions, pixelDimensions );
				glScissor( 0, 0, glConfig.nativeScreenWidth >> 1, glConfig.nativeScreenHeight );
				
				idVec4	color( stereoRender_warpCenterX.GetFloat(), stereoRender_warpCenterY.GetFloat(), stereoRender_warpParmZ.GetFloat(), stereoRender_warpParmW.GetFloat() );
				// don't use GL_Color(), because we don't want to clamp
				renderProgManager.SetRenderParm( RENDERPARM_COLOR, color.ToFloatPtr() );
				
				SelectTexture( 0 );
				stereoRenderImages[0]->Bind();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
				DrawElementsWithCounters( &unitSquareSurface );
				
				idVec4	color2( stereoRender_warpCenterX.GetFloat(), stereoRender_warpCenterY.GetFloat(), stereoRender_warpParmZ.GetFloat(), stereoRender_warpParmW.GetFloat() );
				// don't use GL_Color(), because we don't want to clamp
				renderProgManager.SetRenderParm( RENDERPARM_COLOR, color2.ToFloatPtr() );
				
				glViewport( ( glConfig.nativeScreenWidth >> 1 ),
							( glConfig.nativeScreenHeight >> 1 ) - ( pixelDimensions >> 1 ),
							pixelDimensions, pixelDimensions );
				glScissor( glConfig.nativeScreenWidth >> 1, 0, glConfig.nativeScreenWidth >> 1, glConfig.nativeScreenHeight );
				
				SelectTexture( 0 );
				stereoRenderImages[1]->Bind();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
				DrawElementsWithCounters( &unitSquareSurface );
				break;
			}
			// a non-warped side-by-side-uncompressed (dual input cable) is rendered
			// just like STEREO3D_SIDE_BY_SIDE_COMPRESSED, so fall through.
		case STEREO3D_SIDE_BY_SIDE_COMPRESSED:
			SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			ViewportAndScissor( 0, 0, tr.GetWidth(), tr.GetHeight() );
			DrawElementsWithCounters( &unitSquareSurface );
			
			SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			ViewportAndScissor( tr.GetWidth(), 0, tr.GetWidth(), tr.GetHeight() );
			DrawElementsWithCounters( &unitSquareSurface );
			break;
			
		case STEREO3D_TOP_AND_BOTTOM_COMPRESSED:
			SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			ViewportAndScissor( 0, 0, tr.GetWidth(), tr.GetHeight() );
			DrawElementsWithCounters( &unitSquareSurface );
			
			SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			ViewportAndScissor( 0, tr.GetHeight(), tr.GetWidth(), tr.GetHeight() );
			DrawElementsWithCounters( &unitSquareSurface );
			break;
			
		case STEREO3D_INTERLACED:
			// every other scanline
			SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			
			SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			
			ViewportAndScissor( 0, 0, tr.GetWidth(), tr.GetHeight() * 2 );
			renderProgManager.BindShader_StereoInterlace();
			DrawElementsWithCounters( &unitSquareSurface );
			
			SelectTexture( 0 );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			
			SelectTexture( 1 );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			
			break;
	}
	
	// debug tool
	DrawFlickerBox();
	
	// make sure the drawing is actually started
	glFlush();
	
	// we may choose to sync to the swapbuffers before the next frame
	
	// stop rendering on this thread
	uint64_t backEndFinishTime = Sys_Microseconds();
	pc.totalMicroSec = backEndFinishTime - backEndStartTime;
}
#endif

/*
====================
crBackend::ExecuteBackEndCommands

This function will be called syncronously if running without
smp extensions, or asyncronously by another thread.
====================
*/
void crBackend::ExecuteBackEndCommands( const emptyCommand_t* cmds )
{
	// r_debugRenderToTexture
	int c_draw3d = 0;
	int c_draw2d = 0;
	int c_setBuffers = 0;
	int c_copyRenders = 0;
	
	resolutionScale.SetCurrentGPUFrameTime( commonLocal.GetRendererGPUMicroseconds() );
	
	renderLog.StartFrame();
	
	if( cmds->commandId == RC_NOP && !cmds->next )
		return;
	
/// Don't suport stereo rendering... fow now 
#if 0
	if( tr.GetStereo3DMode() != STEREO3D_OFF )
	{
		StereoRenderExecuteBackEndCommands( cmds );
		renderLog.EndFrame();
		return;
	}
#endif
/// gunganlan 

	uint64_t backEndStartTime = Sys_Microseconds();
	
	// needed for editor rendering
	SetDefaultState();
	
	// foresthale 2014-04-21: r_glow
	// we can only render the postprocess glow if the buffer is updated in the same frame
	glowRenderCopied = false;

	// If we have a stereo pixel format, this will draw to both
	// the back left and back right buffers, which will have a
	// performance penalty.
	for( ; cmds != nullptr; cmds = reinterpret_cast<const emptyCommand_t*>( cmds->next ) )
	{
		switch( cmds->commandId )
		{
			case RC_NOP:
				break;
			case RC_DRAW_VIEW_3D:
			case RC_DRAW_VIEW_GUI:
				DrawView( cmds, 0 );
				if( ( ( const drawSurfsCommand_t* )cmds )->viewDef->viewEntitys )
					c_draw3d++;
				else
					c_draw2d++;
				break;
			case RC_SET_BUFFER:
				SetBuffer( cmds );
				c_setBuffers++;
				break;
			case RC_COPY_RENDER:
				CopyRender( cmds );
				c_copyRenders++;
				break;
			case RC_POST_PROCESS:
				PostProcess( cmds );
				break;
			default:
				common->Error( "RB_ExecuteBackEndCommands: bad commandId" );
				break;
		}
	}
	
	DrawFlickerBox();
	
	// Fix for the steam overlay not showing up while in game without Shell/Debug/Console/Menu also rendering
	// glColorMask( 1, 1, 1, 1 );
	// glFlush();

	/// TODO: submit here? 
	
	// stop rendering on this thread
	uint64_t backEndFinishTime = Sys_Microseconds();
	pc.totalMicroSec = backEndFinishTime - backEndStartTime;
	
	if( r_debugRenderToTexture.GetInteger() == 1 )
	{
		common->Printf( "3d: %i, 2d: %i, SetBuf: %i, CpyRenders: %i, CpyFrameBuf: %i\n", c_draw3d, c_draw2d, c_setBuffers, c_copyRenders, pc.c_copyFrameBuffer );
		pc.c_copyFrameBuffer = 0;
	}
	renderLog.EndFrame();
}


/*
================
crBackend::DrawElementsWithCounters
================
*/
void crBackend::DrawElementsWithCounters( const drawSurf_t* surf )
{
	// get vertex buffer
	idVertexBuffer* vertexBuffer = nullptr;
	const vertCacheHandle_t vbHandle = surf->ambientCache;
	if( vertexCache.CacheIsStatic( vbHandle ) )
	{
		vertexBuffer = &vertexCache.staticData.vertexBuffer;
	}
	else
	{
		const uint64_t frameNum = ( int )( vbHandle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
		if( frameNum != ( ( vertexCache.currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
		{
			idLib::Warning( "DrawElementsWithCounters, vertexBuffer == nullptr" );
			return;
		}
		vertexBuffer = &vertexCache.frameData[vertexCache.drawListNum].vertexBuffer;
	}

	const int vertOffset = ( int )( vbHandle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	
	// get index buffer
	const vertCacheHandle_t ibHandle = surf->indexCache;
	idIndexBuffer* indexBuffer;
	if( vertexCache.CacheIsStatic( ibHandle ) )
	{
		indexBuffer = &vertexCache.staticData.indexBuffer;
	}
	else
	{
		const uint64_t frameNum = ( int )( ibHandle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
		if( frameNum != ( ( vertexCache.currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
		{
			idLib::Warning( "RB_DrawElementsWithCounters, indexBuffer == nullptr" );
			return;
		}
		indexBuffer = &vertexCache.frameData[vertexCache.drawListNum].indexBuffer;
	}
	// RB: 64 bit fixes, changed int to GLintptr
	const uintptr_t indexOffset = ( uintptr_t )( ibHandle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	// RB end
	
	RENDERLOG_PRINTF( "Binding Buffers: %p:%i %p:%i\n", vertexBuffer, vertOffset, indexBuffer, indexOffset );
	
	/// update uniform buffers positions
	crUniformManager* uniformManager = crUniformManager::Get();
	uniformManager->SubmitOffsets( tr.GraphicCommandBuffer() );
	
	// RB: 64 bit fixes, changed GLuint to GLintptr
	if( trState.currentIndexBuffer != indexBuffer->GetAPIObject() || !r_useStateCaching.GetBool() )
	{
		VkDeviceSize offset = indexOffset;
		VkDeviceSize size = VK_WHOLE_SIZE;
		auto indexBufferHandle = indexBuffer->GetAPIObject();
		vkCmdBindIndexBuffer( *tr.GraphicCommandBuffer(), *indexBufferHandle, offset, VK_INDEX_TYPE_UINT16 );
		trState.currentIndexBuffer = indexBufferHandle;
	}
	
	if( ( trState.vertexLayout != LAYOUT_DRAW_VERT ) || ( trState.currentVertexBuffer != vertexBuffer->GetAPIObject() ) || !r_useStateCaching.GetBool() )
	{
		VkDeviceSize offset = vertOffset;
		VkDeviceSize size = VK_WHOLE_SIZE;
		VkDeviceSize stride = sizeof( idDrawVert );
		auto vertexBufferHandle = vertexBuffer->GetAPIObject();
		auto vHandle = vertexBufferHandle->Handle();
		vkCmdBindVertexBuffers2( *tr.GraphicCommandBuffer(), 0, 1, &vHandle, &offset, &size, &stride );
		trState.currentVertexBuffer = vertexBufferHandle;		
		trState.vertexLayout = LAYOUT_DRAW_VERT;
	}
	// RB end
	
#if 0
	uint32_t firstIndex = indexOffset / sizeof( triIndex_t );
	int32_t vertexOffset = vertOffset / sizeof( idDrawVert );
	vkCmdDrawIndexed( m_swapchain->CommandBuffer(), r_singleTriangle.GetBool() ? 3 : surf->numIndexes, 1, firstIndex, vertexOffset, 0  );
#else
	vkCmdDrawIndexed( *tr.GraphicCommandBuffer(), r_singleTriangle.GetBool() ? 3 : surf->numIndexes, 1, 0, 0, 0  );
#endif
	
	// RB: added stats
	pc.c_drawElements++;
	pc.c_drawIndexes += surf->numIndexes;
	// RB end
}

/*
================
crBackend::SetVertexColorParms
================
*/
void crBackend::SetVertexColorParms( stageVertexColor_t svc )
{
	crUniformManager* uniformManager = crUniformManager::Get(); 
	//auto vertexUniforms = uniformManager->GetMeshUniforms();
	auto material = uniformManager->GetMaterialUniforms();

	switch( svc )
	{
		case SVC_IGNORE:
			material->colorModulate = zero; ;// SetVertexParm( RENDERPARM_VERTEXCOLOR_MODULATE, zero );
			material->colorAdd = one; ;// SetVertexParm( RENDERPARM_VERTEXCOLOR_ADD, 		one );
			break;
		case SVC_MODULATE:
			material->colorModulate = one;// SetVertexParm( RENDERPARM_VERTEXCOLOR_MODULATE, one );
			material->colorAdd = zero;// SetVertexParm( RENDERPARM_VERTEXCOLOR_ADD, zero );
			break;
		case SVC_INVERSE_MODULATE:
			material->colorModulate = negOne;// SetVertexParm( RENDERPARM_VERTEXCOLOR_MODULATE, negOne );
			material->colorAdd = one;// SetVertexParm( RENDERPARM_VERTEXCOLOR_ADD, one );
			break;
	}
}

void crBackend::ZeroPerformanceCounters( void )
{
	std::memset( &pc, 0, sizeof( pc ) );
}

/*
========================
crBackend::SetDefaultState
This should initialize all Dynamic Pipeline state that any part of the entire program
may touch, including the editor.
========================
*/
void crBackend::SetDefaultState(void)
{
	RENDERLOG_PRINTF( "--- GL_SetDefaultState ---\n" );
	
	//m_swapchain->ClearDepth( 1.0f );
	
	// make sure our GL state vector is set correctly
	std::memset( &trState, 0, sizeof( trState ) );

	// These are changed by GL_Cull
	//vkCmdSetCullMode( tr.GraphicCommandBuffer(), VK_CULL_MODE_NONE );

	/// now changed direct by the pipeline
	// qglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	// qglBlendFunc( GL_ONE, GL_ZERO );
	// qglDepthMask( GL_TRUE );
	// qglDepthFunc( GL_LESS );
	// qglDisable( GL_STENCIL_TEST );
	// qglDisable( GL_POLYGON_OFFSET_FILL );
	// qglDisable( GL_POLYGON_OFFSET_LINE );
	// qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// These should never be changed
	// qglEnable( GL_DEPTH_TEST );
	// qglEnable( GL_BLEND );
	// qglEnable( GL_SCISSOR_TEST );

	if ( r_useScissor.GetBool() ) 
	{
		VkRect2D rect{};
    	rect.offset.x = 0;
    	rect.offset.y = 0;
    	rect.extent.width = tr.GetWidth();
    	rect.extent.height = tr.GetHeight();
    	vkCmdSetScissor( *tr.GraphicCommandBuffer(), 0, 1, &rect );
	}
}

/*
====================
crBackend::Cull

This handles the flipping needed when the view being
rendered is a mirored view.
====================
*/
void crBackend::Cull( const cullType_t cullType )
{
	VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    if ( trState.faceCulling == cullType ) 
        return;

    if ( cullType == CT_TWO_SIDED ) 
	{
        cullMode = VK_CULL_MODE_NONE;
    } 
	else 
	{
        if ( cullType == CT_BACK_SIDED ) 
            // If it's a mirror image, flip it over.
            cullMode = viewDef->isMirror ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT;
		else 
            // CT_FRONT_SIDED
            cullMode = viewDef->isMirror ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
    }

    // Vulkan 1.3 function or VK_EXT_extended_dynamic_state
	vkCmdSetCullMode( *tr.GraphicCommandBuffer(), cullMode );

    trState.faceCulling = cullType;
}

/*
====================
crBackend::PolygonOffset
====================
*/
void crBackend::PolygonOffset( const float scale, const float bias )
{
	trState.polyOfsScale = scale;
	trState.polyOfsBias = bias;
	vkCmdSetDepthBias( *tr.GraphicCommandBuffer(), scale, 0.0f, bias );
}

/*
========================
crBackend::DepthBoundsTest
========================
*/
void crBackend::DepthBoundsTest( const float zmin, const float zmax )
{   
	/// Check if the hardware supports it.
	//if( !glConfig.depthBoundsTestAvailable )
	//	return;

	// get current frame, command buffer from swapchain     
	auto cmd = tr.GraphicCommandBuffer();
	if ( zmin == 0.0f && zmax == 0.0f ) /// Disable the test
	{
		/// Requires Vulkan 1.3 or VK_EXT_extended_dynamic_state
		vkCmdSetDepthBoundsTestEnable( *cmd, VK_FALSE );
	}
	else // Enable and configure limits.
	{
		vkCmdSetDepthBoundsTestEnable( *cmd, VK_TRUE );
		vkCmdSetDepthBounds( *cmd, zmin, zmax );
	}

	vkCmdSetDepthBounds( *cmd, zmin, zmax );
}

void crBackend::Scissor( const int x /* left*/, const int y /* bottom */, const int w, const int h )
{
	VkRect2D rect{};
	rect.offset.x = x;
    rect.offset.y = y;
    rect.extent.width = w;
    rect.extent.height = h;
    vkCmdSetScissor( *tr.GraphicCommandBuffer(), 0, 1, &rect );
}

void crBackend::Viewport( const int x /* left */, const int y /* bottom */, const int w, const int h )
{
	VkViewport viewport{};
    viewport.x = x;
    viewport.y = y + h;
    viewport.width = w;
    viewport.height = -std::abs(h);;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport( *tr.GraphicCommandBuffer(), 0, 1, &viewport );
}

/*
void crBackend::SwapBuffers(void)
{
	///
	m_defaultFB->Unbind();
	
	/// Waits for the presentation image to become available and prepares for presentation,
	/// sends the recorded commands throughout the frame, and sends it to the window.
	m_defaultFB->BlitColorAttachament( { 0, 0, 0, 0, 0, 0, tr.GetWidth(), tr.GetHeight(), 1, tr.GetWidth(), tr.GetHeight() } );
}
*/

void crBackend::Clear(bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a)
{
	uint32_t attachaments = 0;
	VkClearAttachment	clearAttachment[2]{};

	VkClearRect			clearRect{};
	clearRect.rect.extent.width = tr.GetWidth();
	clearRect.rect.extent.height = tr.GetHeight();
	clearRect.rect.offset.x = 0;
	clearRect.rect.offset.y = 0;
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	
	if( color )
	{
		clearAttachment[attachaments].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		clearAttachment[attachaments].colorAttachment = attachaments;
		clearAttachment[attachaments].clearValue.color.float32[0] = r;
		clearAttachment[attachaments].clearValue.color.float32[1] = g;
		clearAttachment[attachaments].clearValue.color.float32[2] = b;
		clearAttachment[attachaments].clearValue.color.float32[3] = a; 
		attachaments++;
	}

	if( depth || stencil )
	{
		clearAttachment[attachaments].aspectMask = VK_IMAGE_ASPECT_NONE; 
		if( depth )
			clearAttachment[attachaments].aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if( stencil )
			clearAttachment[attachaments].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		clearAttachment[attachaments].colorAttachment = 0;
		clearAttachment[attachaments].clearValue.depthStencil.depth = 0.0f;
		clearAttachment[attachaments].clearValue.depthStencil.stencil = stencilValue;
		attachaments++;
	}

	auto cmd = tr.GraphicCommandBuffer();
	vkCmdClearAttachments( *cmd, attachaments, clearAttachment, 1, &clearRect );
}