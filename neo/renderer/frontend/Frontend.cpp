
#include "precompiled.h"
#include "renderer/renderer_common.h"
#include "Frontend.hpp"

crFrontend::crFrontend( void ) : 
    frameData( nullptr )
{
	REGISTER_PARALLEL_JOB( R_AddSingleLight, "R_AddSingleLight" );
	REGISTER_PARALLEL_JOB( R_AddSingleModel, "R_AddSingleModel" );
}

crFrontend::~crFrontend( void )
{
}

crFrontend *crFrontend::Get(void)
{
	static crFrontend gFrontend = crFrontend();
    return &gFrontend;
}

/*
=================
crFrontend::ViewStatistics
=================
*/
void crFrontend::ViewStatistics( viewDef_t* parms )
{
	// report statistics about this view
	if( !r_showSurfaces.GetBool() )
		return;
	
	common->Printf( "view:%p surfs:%i\n", parms, parms->numDrawSurfs );
}

/*
=============
crFrontend::AddDrawViewCmd

This is the main 3D rendering command.  A single scene may
have multiple views if a mirror, portal, or dynamic texture is present.
=============
*/
void crFrontend::AddDrawViewCmd( viewDef_t* parms, const bool guiOnly )
{
	drawSurfsCommand_t*	cmd = nullptr;
	
	cmd = ( drawSurfsCommand_t* )GetCommandBuffer( sizeof( *cmd ) );
	cmd->commandId = ( guiOnly ) ? RC_DRAW_VIEW_GUI : RC_DRAW_VIEW_3D;
	
	cmd->viewDef = parms;
	
	pc.c_numViews++;
	
	ViewStatistics( parms );
}

/*
=============
crFrontend::AddPostProcess

This issues the command to do a post process after all the views have
been rendered.
=============
*/
void crFrontend::AddDrawPostProcess( viewDef_t* parms )
{
	postProcessCommand_t* cmd = ( postProcessCommand_t* )GetCommandBuffer( sizeof( *cmd ) );
	cmd->commandId = RC_POST_PROCESS;
	cmd->viewDef = parms;
}


/*
============
crFrontend::GetCommandBuffer

Returns memory for a command buffer (stretchPicCommand_t,
drawSurfsCommand_t, etc) and links it to the end of the
current command chain.
============
*/
void* crFrontend::GetCommandBuffer( const size_t bytes )
{
	emptyCommand_t*	cmd = nullptr;
	cmd = ( emptyCommand_t* )FrameAlloc( bytes, FRAME_ALLOC_DRAW_COMMAND );
	cmd->next = nullptr;
	frameData->cmdTail->next = &cmd->commandId;
	frameData->cmdTail = cmd;
	return ( void* )cmd;
}