
#include "precompiled.h"
#include "OpenGL.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_init.h>
#include "renderer/tr_local.h"

PFNGLGETERRORPROC                           	glGetError = nullptr;
PFNGLGETINTEGERVPROC                         	glGetIntegerv = nullptr;
PFNGLGETFLOATVPROC                          	glGetFloatv = nullptr;
PFNGLGETSTRINGPROC								glGetString = nullptr;
PFNGLGETSTRINGIPROC								glGetStringi = nullptr;

PFNGLFLUSHPROC                              	glFlush = nullptr;
PFNGLFINISHPROC                              	glFinish = nullptr;

PFNGLENABLEPROC                             	glEnable = nullptr;
PFNGLDISABLEPROC                            	glDisable = nullptr;

PFNGLCLEARPROC                              	glClear = nullptr;

PFNGLCLEARCOLORPROC                         	glClearColor = nullptr;
PFNGLCLEARSTENCILPROC                       	glClearStencil = nullptr;
PFNGLCLEARDEPTHPROC                         	glClearDepth = nullptr;

PFNGLCOLORMASKPROC                          	glColorMask = nullptr;
PFNGLDEPTHMASKPROC                          	glDepthMask = nullptr;

PFNGLBLENDFUNCPROC                          	glBlendFunc = nullptr;
PFNGLDEPTHFUNCPROC                          	glDepthFunc = nullptr;
PFNGLSTENCILFUNCPROC                        	glStencilFunc = nullptr;

PFNGLSCISSORPROC                            	glScissor = nullptr;
PFNGLVIEWPORTPROC                           	glViewport = nullptr;

PFNGLPOLYGONMODEPROC                        	glPolygonMode = nullptr;
PFNGLPOLYGONOFFSETPROC                      	glPolygonOffset = nullptr;
PFNGLCULLFACEPROC                           	glCullFace = nullptr;

PFNGLDRAWBUFFERPROC                         	glDrawBuffer = nullptr;
PFNGLREADBUFFERPROC                         	glReadBuffer = nullptr;

PFNGLSTENCILOPPROC                          	glStencilOp = nullptr;

PFNGLLINEWIDTHPROC                          	glLineWidth = nullptr;
PFNGLPOINTSIZEPROC                           	glPointSize = nullptr;

PFNGLREADPIXELSPROC                         	glReadPixels = nullptr;

// GL_ARB_multitexture
PFNGLACTIVETEXTUREPROC							glActiveTexture = nullptr;
PFNGLBINDTEXTUREPROC                        	glBindTexture = nullptr;

PFNGLBINDTEXTUREUNITPROC						glBindTextureUnit = nullptr;

// GL_ARB_vertex_buffer_object
PFNGLBINDBUFFERPROC                         	glBindBuffer = nullptr;
PFNGLBINDBUFFERRANGEPROC						glBindBufferRange = nullptr;
PFNGLDELETEBUFFERSPROC							glDeleteBuffers = nullptr;
PFNGLGENBUFFERSPROC								glGenBuffers = nullptr;
PFNGLISBUFFERPROC								glIsBuffer = nullptr;
PFNGLBUFFERDATAPROC								glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC							glBufferSubData = nullptr;
PFNGLGETBUFFERSUBDATAPROC						glGetBufferSubData = nullptr;
PFNGLMAPBUFFERPROC								glMapBuffer = nullptr;
PFNGLUNMAPBUFFERPROC							glUnmapBuffer = nullptr;
PFNGLGETBUFFERPARAMETERIVPROC					glGetBufferParameteriv = nullptr;
PFNGLGETBUFFERPOINTERVPROC						glGetBufferPointerv = nullptr;

PFNGLPIXELSTOREIPROC                         	glPixelStorei = nullptr;

PFNGLGENTEXTURESPROC                         	glGenTextures = nullptr;
PFNGLDELETETEXTURESPROC							glDeleteTextures = nullptr;
PFNGLTEXIMAGE2DPROC                          	glTexImage2D = nullptr;
PFNGLTEXSUBIMAGE2DPROC							glTexSubImage2D = nullptr;
PFNGLTEXIMAGE3DPROC								glTexImage3D = nullptr;
PFNGLCOPYTEXIMAGE2DPROC                      	glCopyTexImage2D = nullptr;
PFNGLTEXPARAMETERFPROC                       	glTexParameterf = nullptr;
PFNGLTEXPARAMETERIPROC                       	glTexParameteri = nullptr;
PFNGLTEXPARAMETERFVPROC                      	glTexParameterfv = nullptr;
PFNGLTEXPARAMETERIVPROC                      	glTexParameteriv = nullptr;

// GL_ARB_texture_compression
PFNGLCOMPRESSEDTEXIMAGE2DPROC               	glCompressedTexImage2D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC            	glCompressedTexSubImage2D = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEPROC              	glGetCompressedTexImage = nullptr;

// GL_ARB_map_buffer_range
PFNGLMAPBUFFERRANGEPROC							glMapBufferRange = nullptr;

// GL_ARB_draw_elements_base_vertex
PFNGLDRAWELEMENTSBASEVERTEXPROC  				glDrawElementsBaseVertex = nullptr;

// GL_ARB_vertex_array_object
PFNGLGENVERTEXARRAYSPROC						glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC						glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC                 	glDeleteVertexArrays = nullptr;

// GL_ARB_vertex_program / GL_ARB_fragment_program
//PFNGLPROGRAMSTRINGPROC						glProgramString;
PFNGLVERTEXATTRIBPOINTERPROC					glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC				glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC				glDisableVertexAttribArray = nullptr;

// GLSL / OpenGL 2.0
PFNGLCREATESHADERPROC							glCreateShader = nullptr;
PFNGLDELETESHADERPROC							glDeleteShader = nullptr;
PFNGLSHADERSOURCEPROC							glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC							glCompileShader = nullptr;
PFNGLGETSHADERIVPROC							glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC						glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC							glCreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC							glDeleteProgram = nullptr;
PFNGLATTACHSHADERPROC							glAttachShader = nullptr;
PFNGLDETACHSHADERPROC							glDetachShader = nullptr;
PFNGLLINKPROGRAMPROC							glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC								glUseProgram = nullptr;
PFNGLGETPROGRAMIVPROC							glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC						glGetProgramInfoLog = nullptr;
PFNGLPROGRAMPARAMETERIPROC						glProgramParameteri = nullptr;
PFNGLBINDATTRIBLOCATIONPROC						glBindAttribLocation = nullptr;
PFNGLGETUNIFORMLOCATIONPROC						glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC								glUniform1i = nullptr;
PFNGLUNIFORM4FVPROC								glUniform4fv = nullptr;

// foresthale 2014-02-18: added qglDrawbuffers
PFNGLDRAWBUFFERSPROC                        	glDrawBuffers = nullptr;

// foresthale 2014-02-16: added GL_ARB_framebuffer_object
PFNGLISRENDERBUFFERPROC                     	glIsRenderbuffer = nullptr;
PFNGLBINDRENDERBUFFERPROC                   	glBindRenderbuffer = nullptr;
PFNGLDELETERENDERBUFFERSPROC                	glDeleteRenderbuffers = nullptr;
PFNGLGENRENDERBUFFERSPROC                   	glGenRenderbuffers = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC                	glRenderbufferStorage = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVPROC         	glGetRenderbufferParameteriv = nullptr;
PFNGLISFRAMEBUFFERPROC                      	glIsFramebuffer = nullptr;
PFNGLBINDFRAMEBUFFERPROC                    	glBindFramebuffer = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC                 	glDeleteFramebuffers = nullptr;
PFNGLGENFRAMEBUFFERSPROC                    	glGenFramebuffers = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC             	glCheckFramebufferStatus = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DPROC               	glFramebufferTexture1D = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC               	glFramebufferTexture2D = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DPROC               	glFramebufferTexture3D = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC            	glFramebufferRenderbuffer = nullptr;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC	glGetFramebufferAttachmentParameteriv = nullptr;
PFNGLGENERATEMIPMAPPROC                     	glGenerateMipmap = nullptr;
PFNGLBLITFRAMEBUFFERPROC                    	glBlitFramebuffer = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC     	glRenderbufferStorageMultisample = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC            	glFramebufferTextureLayer = nullptr;

// GL_ARB_uniform_buffer_object
PFNGLGETUNIFORMBLOCKINDEXPROC					glGetUniformBlockIndex = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC					glUniformBlockBinding = nullptr;

// GL_ATI_separate_stencil / OpenGL 2.0
PFNGLSTENCILOPSEPARATEPROC						glStencilOpSeparate = nullptr;
PFNGLSTENCILFUNCSEPARATEPROC					glStencilFuncSeparate = nullptr;

// GL_EXT_depth_bounds_test
PFNGLDEPTHBOUNDSEXTPROC                 		glDepthBoundsEXT = nullptr;

// GL_ARB_sync
PFNGLFENCESYNCPROC								glFenceSync= nullptr ;
PFNGLISSYNCPROC									glIsSync = nullptr;
PFNGLCLIENTWAITSYNCPROC							glClientWaitSync = nullptr;
PFNGLDELETESYNCPROC								glDeleteSync = nullptr;

// GL_ARB_occlusion_query
PFNGLGENQUERIESPROC								glGenQueries = nullptr;
PFNGLDELETEQUERIESPROC							glDeleteQueries = nullptr;
PFNGLISQUERYPROC								glIsQuery = nullptr;
PFNGLBEGINQUERYPROC								glBeginQuery = nullptr;
PFNGLENDQUERYPROC								glEndQuery = nullptr;
PFNGLGETQUERYIVPROC								glGetQueryiv = nullptr;
PFNGLGETQUERYOBJECTIVPROC						glGetQueryObjectiv = nullptr;
PFNGLGETQUERYOBJECTUIVPROC						glGetQueryObjectuiv = nullptr;

// GL_ARB_timer_query / GL_EXT_timer_query
PFNGLGETQUERYOBJECTUI64VPROC					glGetQueryObjectui64v = nullptr;

// GL_ARB_debug_output
PFNGLDEBUGMESSAGECONTROLPROC					glDebugMessageControl = nullptr;
PFNGLDEBUGMESSAGEINSERTPROC						glDebugMessageInsert = nullptr;
PFNGLDEBUGMESSAGECALLBACKPROC					glDebugMessageCallback = nullptr;
PFNGLGETDEBUGMESSAGELOGPROC						glGetDebugMessageLog = nullptr;

// TODO: Deprecate GL
void ( APIENTRYP glRasterPos2f )( GLfloat x, GLfloat y );
void ( APIENTRYP glOrtho )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val );
void ( APIENTRYP glBegin )( GLenum mode );
void ( APIENTRYP glDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRYP glEnd )( void );
void ( APIENTRYP glVertex3f )( GLfloat x, GLfloat y, GLfloat z );
void ( APIENTRYP glVertex3fv )( const GLfloat *v );
void ( APIENTRYP glColor4fv )( const GLfloat *v );
void ( APIENTRYP glColor4ubv )(const GLubyte *v);
void ( APIENTRYP glColor3f )( GLfloat red, GLfloat green, GLfloat blue );
void ( APIENTRYP glColor3fv )( const GLfloat *v );
void ( APIENTRYP glColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRYP glLoadIdentity )( void );
void ( APIENTRYP glPushMatrix )( void );
void ( APIENTRYP glPopMatrix )(void);
void ( APIENTRYP glMatrixMode )( GLenum mode );
void ( APIENTRYP glLoadMatrixf )( const GLfloat *m );
void ( APIENTRYP glPushAttrib )(GLbitfield mask);
void ( APIENTRYP glPopAttrib )(void);
void ( APIENTRYP glArrayElement )(GLint i);

idCVar r_waylandcompat( "r_waylandcompat", "0", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "wayland compatible framebuffer" );
idCVar r_useOpenGL32( "r_useOpenGL32", "1", CVAR_INTEGER, "0 = OpenGL 2.0, 1 = OpenGL 3.2 compatibility profile, 2 = OpenGL 3.2 core profile", 0, 2 );

static void APIENTRY DebugOutputCall( GLenum in_source, GLenum in_type, GLuint in_id, GLenum in_severity, GLsizei in_length, const GLchar *in_message, const void *in_userParam );

static struct OpenGL
{
    SDL_Window*     window = nullptr;
    SDL_GLContext   context = nullptr;
}context;

static bool QGL_Init( const char* dllname );
static void QGL_Shutdown( void );


/*
===================
GLimp_PreInit

 R_GetModeListForDisplay is called before GLimp_Init(), but SDL needs SDL_Init() first.
 So do that in GLimp_PreInit()
 Calling that function more than once doesn't make a difference
===================
*/
void GLimp_PreInit( void ) // DG: added this function for SDL compatibility
{
	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		if( SDL_Init( SDL_INIT_VIDEO ) )
			common->Error( "Error while initializing SDL: %s", SDL_GetError() );
	}
}

/*
===================
GLimp_Init
===================
*/
bool GLimp_Init( const bool in_stereo, const uint8_t in_multiSamples )
{
	common->Printf( "Initializing OpenGL subsystem\n" );
	
	GLimp_PreInit(); // DG: make sure SDL is initialized
	
    // get window handler
    context.window = static_cast<SDL_Window*>( sys->GetVideoSystem()->WindowHandler() );
	assert( context.window != nullptr );

	int colorbits = 24;
	int depthbits = 24;
	int stencilbits = 8;
	
	for( int i = 0; i < 16; i++ )
	{
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if( ( i % 4 ) == 0 && i )
		{
			// one pass, reduce
			switch( i / 4 )
			{
				case 2 :
					if( colorbits == 24 )
						colorbits = 16;
					break;
				case 1 :
					if( depthbits == 24 )
						depthbits = 16;
					else if( depthbits == 16 )
						depthbits = 8;
				case 3 :
					if( stencilbits == 24 )
						stencilbits = 16;
					else if( stencilbits == 16 )
						stencilbits = 8;
			}
		}
		
		int tcolorbits = colorbits;
		int tdepthbits = depthbits;
		int tstencilbits = stencilbits;
		
		if( ( i % 4 ) == 3 )
		{
			// reduce colorbits
			if( tcolorbits == 24 )
				tcolorbits = 16;
		}
		
		if( ( i % 4 ) == 2 )
		{
			// reduce depthbits
			if( tdepthbits == 24 )
				tdepthbits = 16;
			else if( tdepthbits == 16 )
				tdepthbits = 8;
		}
		
		if( ( i % 4 ) == 1 )
		{
			// reduce stencilbits
			if( tstencilbits == 24 )
				tstencilbits = 16;
			else if( tstencilbits == 16 )
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}
		
		int channelcolorbits = 4;
		if( tcolorbits == 24 )
			channelcolorbits = 8;
			
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, channelcolorbits );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, channelcolorbits );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, channelcolorbits );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, tdepthbits );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, tstencilbits );
		
		if( r_waylandcompat.GetBool() )
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
		else
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, channelcolorbits );
			
		SDL_GL_SetAttribute( SDL_GL_STEREO, in_stereo ? 1 : 0 );
		
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, in_multiSamples ? 1 : 0 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, in_multiSamples );
		
		// RB begin
		if( r_useOpenGL32.GetInteger() > 0 )
		{
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
			
			if( r_debugContext.GetBool() )
				SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
		}
		
		if( r_useOpenGL32.GetInteger() > 1 )
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		// RB end
		
		context.context = SDL_GL_CreateContext( context.window );
		
		if( !context.context )
		{
			common->Warning( "Couldn't set GL mode %d/%d/%d: %s", channelcolorbits, tdepthbits, tstencilbits, SDL_GetError() );
			continue;
		}
		
		if( SDL_GL_SetSwapInterval( r_swapInterval.GetInteger() ) < 0 )
			common->Warning( "SDL_GL_SWAP_CONTROL not supported" );
			
		// RB begin
		SDL_GetWindowSize( context.window, &glConfig.nativeScreenWidth, &glConfig.nativeScreenHeight );
		// RB end
		
		glConfig.isFullscreen = ( SDL_GetWindowFlags( context.window ) & SDL_WINDOW_FULLSCREEN ) == SDL_WINDOW_FULLSCREEN;
		common->Printf( "Using %d color bits, %d depth, %d stencil display\n",
						channelcolorbits, tdepthbits, tstencilbits );
						
		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		
		// RB begin
		glConfig.isStereoPixelFormat = in_stereo;
		glConfig.multisamples = in_multiSamples;
		glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted
		// should side-by-side stereo modes be consider aspect 0.5?
		
		// RB end

        // TODO: set fullscreen

		break;
	}
	
	if( !context.context )
	{
		common->Printf( "No usable GL mode found: %s", SDL_GetError() );
		return false;
	}
	
	QGL_Init( "nodriverlib" );
	
	// DG: disable cursor, we have two cursors in menu (because mouse isn't grabbed in menu)
    SDL_HideCursor();
	//SDL_ShowCursor();
	// DG end

#if _DEBUG
	// enable debug
    glEnable( GL_DEBUG_OUTPUT);

    // To lock on error (synchronized debug)
    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS ); 

    glDebugMessageCallback( DebugOutputCall, nullptr ); // set callback
    glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE ); // Ativa tudo
#endif //_DEBUG
	
	return true;
}

/*
===================
GLimp_Shutdown
===================
*/
void GLimp_Shutdown( void )
{
	if ( !context.context )
		return;

	SDL_GL_DestroyContext( context.context );

	context.context = nullptr;
}

/*
===================
GLimp_SetScreenParms
===================
*/
bool GLimp_SetScreenParms( const bool in_stereo, const uint8_t in_multiSamples )
{
	// Note: the following stuff would also work with SDL1.2
	SDL_GL_SetAttribute( SDL_GL_STEREO, in_stereo ? 1 : 0 );	
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, in_multiSamples ? 1 : 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, in_multiSamples );
    
	glConfig.isStereoPixelFormat = in_stereo;
	glConfig.multisamples = in_multiSamples;
	
	return true;
}

/*
===================
GLimp_SwapBuffers
===================
*/
void GLimp_SwapBuffers( void )
{
    if( r_swapInterval.IsModified() )
	{
		r_swapInterval.ClearModified();
		
		int interval = 0;
		if( r_swapInterval.GetInteger() == 1 )
			interval = ( glConfig.swapControlTearAvailable ) ? -1 : 1;
		else if( r_swapInterval.GetInteger() == 2 )
			interval = 1;
	
        SDL_GL_SetSwapInterval( interval );
	}
	
	SDL_GL_SwapWindow( context.window );
}

/*
=================
GLimp_SetGamma
=================
*/



void GLimp_SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] )
{
}

#define GET_GL_PROC( T, P ) P = reinterpret_cast<T>( SDL_GL_GetProcAddress( #P ) )

bool QGL_Init( const char* dllname )
{
	GET_GL_PROC( PFNGLGETERRORPROC, glGetError );
	GET_GL_PROC( PFNGLGETINTEGERVPROC, glGetIntegerv );
	GET_GL_PROC( PFNGLGETFLOATVPROC, glGetFloatv );
	GET_GL_PROC( PFNGLGETSTRINGPROC, glGetString );
	GET_GL_PROC( PFNGLGETSTRINGIPROC, glGetStringi );

	GET_GL_PROC( PFNGLFLUSHPROC, glFlush );
	GET_GL_PROC( PFNGLFINISHPROC, glFinish );

	GET_GL_PROC( PFNGLENABLEPROC, glEnable );
	GET_GL_PROC( PFNGLDISABLEPROC, glDisable );

	GET_GL_PROC( PFNGLCLEARPROC, glClear );

	GET_GL_PROC( PFNGLCLEARCOLORPROC, glClearColor );
	GET_GL_PROC( PFNGLCLEARSTENCILPROC, glClearStencil );
	GET_GL_PROC( PFNGLCLEARDEPTHPROC, glClearDepth );

	GET_GL_PROC( PFNGLCOLORMASKPROC, glColorMask );
	GET_GL_PROC( PFNGLDEPTHMASKPROC, glDepthMask );

	GET_GL_PROC( PFNGLBLENDFUNCPROC, glBlendFunc );
	GET_GL_PROC( PFNGLDEPTHFUNCPROC, glDepthFunc );
	GET_GL_PROC( PFNGLSTENCILFUNCPROC, glStencilFunc );

	GET_GL_PROC( PFNGLSCISSORPROC, glScissor );
	GET_GL_PROC( PFNGLVIEWPORTPROC, glViewport );

	GET_GL_PROC( PFNGLPOLYGONMODEPROC, glPolygonMode );
	GET_GL_PROC( PFNGLPOLYGONOFFSETPROC, glPolygonOffset );
	GET_GL_PROC( PFNGLCULLFACEPROC, glCullFace );

	GET_GL_PROC( PFNGLDRAWBUFFERPROC, glDrawBuffer );
	GET_GL_PROC( PFNGLREADBUFFERPROC, glReadBuffer );

	GET_GL_PROC( PFNGLSTENCILOPPROC, glStencilOp );

	GET_GL_PROC( PFNGLLINEWIDTHPROC, glLineWidth );
	GET_GL_PROC( PFNGLPOINTSIZEPROC, glPointSize );

	GET_GL_PROC( PFNGLREADPIXELSPROC, glReadPixels );

	// GL_ARB_multitexture
	GET_GL_PROC( PFNGLACTIVETEXTUREPROC, glActiveTexture );
	GET_GL_PROC( PFNGLBINDTEXTUREPROC, glBindTexture );

	GET_GL_PROC( PFNGLBINDTEXTUREUNITPROC,  glBindTextureUnit );

	// GL_ARB_vertex_buffer_object
	GET_GL_PROC( PFNGLBINDBUFFERPROC, glBindBuffer );
	GET_GL_PROC( PFNGLBINDBUFFERRANGEPROC, glBindBufferRange );
	GET_GL_PROC( PFNGLDELETEBUFFERSPROC, glDeleteBuffers );
	GET_GL_PROC( PFNGLGENBUFFERSPROC, glGenBuffers );
	GET_GL_PROC( PFNGLISBUFFERPROC, glIsBuffer );
	GET_GL_PROC( PFNGLBUFFERDATAPROC, glBufferData );
	GET_GL_PROC( PFNGLBUFFERSUBDATAPROC, glBufferSubData );
	GET_GL_PROC( PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData );
	GET_GL_PROC( PFNGLMAPBUFFERPROC, glMapBuffer );
	GET_GL_PROC( PFNGLUNMAPBUFFERPROC, glUnmapBuffer );
	GET_GL_PROC( PFNGLGETBUFFERPARAMETERIVPROC, glGetBufferParameteriv );
	GET_GL_PROC( PFNGLGETBUFFERPOINTERVPROC, glGetBufferPointerv );

	GET_GL_PROC( PFNGLPIXELSTOREIPROC, glPixelStorei );

	GET_GL_PROC( PFNGLGENTEXTURESPROC, glGenTextures );
	GET_GL_PROC( PFNGLDELETETEXTURESPROC, glDeleteTextures );
	GET_GL_PROC( PFNGLTEXIMAGE2DPROC, glTexImage2D );
	GET_GL_PROC( PFNGLTEXSUBIMAGE2DPROC, glTexSubImage2D );
	GET_GL_PROC( PFNGLTEXIMAGE3DPROC, glTexImage3D );
	GET_GL_PROC( PFNGLCOPYTEXIMAGE2DPROC, glCopyTexImage2D );
	GET_GL_PROC( PFNGLTEXPARAMETERFPROC, glTexParameterf );
	GET_GL_PROC( PFNGLTEXPARAMETERIPROC, glTexParameteri );
	GET_GL_PROC( PFNGLTEXPARAMETERFVPROC, glTexParameterfv );
	GET_GL_PROC( PFNGLTEXPARAMETERIVPROC, glTexParameteriv );

	// GL_ARB_texture_compression
	GET_GL_PROC( PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D );
	GET_GL_PROC( PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, glCompressedTexSubImage2D );
	GET_GL_PROC( PFNGLGETCOMPRESSEDTEXIMAGEPROC, glGetCompressedTexImage );

	// GL_ARB_map_buffer_range
	GET_GL_PROC( PFNGLMAPBUFFERRANGEPROC, glMapBufferRange );

	// GL_ARB_draw_elements_base_vertex
	GET_GL_PROC( PFNGLDRAWELEMENTSBASEVERTEXPROC, glDrawElementsBaseVertex );

	// GL_ARB_vertex_array_object
	GET_GL_PROC( PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays );
	GET_GL_PROC( PFNGLBINDVERTEXARRAYPROC, glBindVertexArray );
	GET_GL_PROC( PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays );

	// GL_ARB_vertex_program / GL_ARB_fragment_program
	GET_GL_PROC( PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer );
	GET_GL_PROC( PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray );
	GET_GL_PROC( PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray );

	// GLSL / OpenGL 2.0
	GET_GL_PROC( PFNGLCREATESHADERPROC, glCreateShader );
	GET_GL_PROC( PFNGLDELETESHADERPROC, glDeleteShader );
	GET_GL_PROC( PFNGLSHADERSOURCEPROC, glShaderSource );
	GET_GL_PROC( PFNGLCOMPILESHADERPROC, glCompileShader );
	GET_GL_PROC( PFNGLGETSHADERIVPROC, glGetShaderiv );
	GET_GL_PROC( PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog );
	GET_GL_PROC( PFNGLCREATEPROGRAMPROC, glCreateProgram );
	GET_GL_PROC( PFNGLDELETEPROGRAMPROC, glDeleteProgram );
	GET_GL_PROC( PFNGLATTACHSHADERPROC, glAttachShader );
	GET_GL_PROC( PFNGLDETACHSHADERPROC, glDetachShader );
	GET_GL_PROC( PFNGLLINKPROGRAMPROC, glLinkProgram );
	GET_GL_PROC( PFNGLUSEPROGRAMPROC, glUseProgram );
	GET_GL_PROC( PFNGLGETPROGRAMIVPROC, glGetProgramiv );
	GET_GL_PROC( PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog );
	GET_GL_PROC( PFNGLPROGRAMPARAMETERIPROC, glProgramParameteri );
	GET_GL_PROC( PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation );
	GET_GL_PROC( PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation );
	GET_GL_PROC( PFNGLUNIFORM1IPROC, glUniform1i );
	GET_GL_PROC( PFNGLUNIFORM4FVPROC, glUniform4fv );

	// foresthale 2014-02-18: added qglDrawbuffers
	GET_GL_PROC( PFNGLDRAWBUFFERSPROC, glDrawBuffers );

	// foresthale 2014-02-16: added GL_ARB_framebuffer_object
	GET_GL_PROC( PFNGLISRENDERBUFFERPROC, glIsRenderbuffer );
	GET_GL_PROC( PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer );
	GET_GL_PROC( PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers );
	GET_GL_PROC( PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers );
	GET_GL_PROC( PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage );
	GET_GL_PROC( PFNGLGETRENDERBUFFERPARAMETERIVPROC, glGetRenderbufferParameteriv );
	GET_GL_PROC( PFNGLISFRAMEBUFFERPROC, glIsFramebuffer );
	GET_GL_PROC( PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer );
	GET_GL_PROC( PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers );
	GET_GL_PROC( PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers );
	GET_GL_PROC( PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus );
	GET_GL_PROC( PFNGLFRAMEBUFFERTEXTURE1DPROC, glFramebufferTexture1D );
	GET_GL_PROC( PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D );
	GET_GL_PROC( PFNGLFRAMEBUFFERTEXTURE3DPROC, glFramebufferTexture3D );
	GET_GL_PROC( PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer );
	GET_GL_PROC( PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC, glGetFramebufferAttachmentParameteriv );
	GET_GL_PROC( PFNGLGENERATEMIPMAPPROC, glGenerateMipmap );
	GET_GL_PROC( PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer );
	GET_GL_PROC( PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample );
	GET_GL_PROC( PFNGLFRAMEBUFFERTEXTURELAYERPROC, glFramebufferTextureLayer );

	// GL_ARB_uniform_buffer_object
	GET_GL_PROC( PFNGLGETUNIFORMBLOCKINDEXPROC,  glGetUniformBlockIndex );
	GET_GL_PROC( PFNGLUNIFORMBLOCKBINDINGPROC,  glUniformBlockBinding );

	// GL_ATI_separate_stencil / OpenGL 2.0
	GET_GL_PROC( PFNGLSTENCILOPSEPARATEPROC, glStencilOpSeparate );
	GET_GL_PROC( PFNGLSTENCILFUNCSEPARATEPROC, glStencilFuncSeparate );

	// GL_EXT_depth_bounds_test
	GET_GL_PROC( PFNGLDEPTHBOUNDSEXTPROC, glDepthBoundsEXT );

	// GL_ARB_sync
	GET_GL_PROC( PFNGLFENCESYNCPROC, glFenceSync );
	GET_GL_PROC( PFNGLISSYNCPROC, glIsSync );
	GET_GL_PROC( PFNGLCLIENTWAITSYNCPROC, glClientWaitSync );
	GET_GL_PROC( PFNGLDELETESYNCPROC, glDeleteSync );

	// GL_ARB_occlusion_query
	GET_GL_PROC( PFNGLGENQUERIESPROC, glGenQueries );
	GET_GL_PROC( PFNGLDELETEQUERIESPROC, glDeleteQueries );
	GET_GL_PROC( PFNGLISQUERYPROC, glIsQuery );
	GET_GL_PROC( PFNGLBEGINQUERYPROC, glBeginQuery );
	GET_GL_PROC( PFNGLENDQUERYPROC, glEndQuery );
	GET_GL_PROC( PFNGLGETQUERYIVPROC, glGetQueryiv );
	GET_GL_PROC( PFNGLGETQUERYOBJECTIVPROC, glGetQueryObjectiv );
	GET_GL_PROC( PFNGLGETQUERYOBJECTUIVPROC, glGetQueryObjectuiv );

	// GL_ARB_timer_query / GL_EXT_timer_query
	GET_GL_PROC( PFNGLGETQUERYOBJECTUI64VPROC, glGetQueryObjectui64v );

	// GL_ARB_debug_output
	GET_GL_PROC( PFNGLDEBUGMESSAGECONTROLPROC, glDebugMessageControl );
	GET_GL_PROC( PFNGLDEBUGMESSAGEINSERTPROC, glDebugMessageInsert );
	GET_GL_PROC( PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback );
	GET_GL_PROC( PFNGLGETDEBUGMESSAGELOGPROC, glGetDebugMessageLog );

	glRasterPos2f = (decltype(glRasterPos2f))SDL_GL_GetProcAddress( "glRasterPos2f" );

	glOrtho = ( decltype( glOrtho ) )SDL_GL_GetProcAddress( "glOrtho" );
	glBegin = ( decltype( glBegin ) )SDL_GL_GetProcAddress( "glBegin" );
	glDrawPixels = ( decltype( glDrawPixels ) )SDL_GL_GetProcAddress( "glDrawPixels" );
	glEnd = ( decltype( glEnd ) )SDL_GL_GetProcAddress( "glEnd" );
	glVertex3f = ( decltype( glVertex3f ) )SDL_GL_GetProcAddress( "glVertex3f" );
	glVertex3fv = ( decltype( glVertex3fv ) )SDL_GL_GetProcAddress( "glVertex3fv" );
	glColor4fv = ( decltype( glColor4fv ) )SDL_GL_GetProcAddress( "glColor4fv" );
	glColor4ubv = ( decltype( glColor4ubv ) )SDL_GL_GetProcAddress( "glColor4ubv" );
	glColor3f = ( decltype( glColor3f ) )SDL_GL_GetProcAddress( "glColor3f" );
	glColor3fv = ( decltype( glColor3fv ) )SDL_GL_GetProcAddress( "glColor3fv" );
	glColor4f = ( decltype( glColor4f ) )SDL_GL_GetProcAddress( "glColor4f" );
	glLoadIdentity = ( decltype( glLoadIdentity ) )SDL_GL_GetProcAddress( "glLoadIdentity" );
	glPushMatrix = ( decltype( glPushMatrix ) )SDL_GL_GetProcAddress( "glPushMatrix" );
	glPopMatrix = ( decltype( glPopMatrix ) )SDL_GL_GetProcAddress( "glPopMatrix" );
	glMatrixMode = ( decltype( glMatrixMode ) )SDL_GL_GetProcAddress( "glMatrixMode" );
	glLoadMatrixf = ( decltype( glLoadMatrixf ) )SDL_GL_GetProcAddress( "glLoadMatrixf" );
	glPushAttrib = ( decltype( glPushAttrib ) )SDL_GL_GetProcAddress( "glPushAttrib" );
	glPopAttrib = ( decltype( glPopAttrib ) )SDL_GL_GetProcAddress( "glPopAttrib" );
	glArrayElement = ( decltype( glArrayElement ) )SDL_GL_GetProcAddress( "glArrayElement" );

	return true;
}

void QGL_Shutdown( void )
{
}

void APIENTRY DebugOutputCall( GLenum in_source, GLenum in_type, GLuint in_id, GLenum in_severity, GLsizei in_length, const GLchar *in_message, const void *in_userParam )
{
    const char * source = nullptr;
    const char * type = nullptr;
    const char * severity = nullptr;
 
    switch ( in_source )
    {
    case GL_DEBUG_SOURCE_API:
        source = "Source: API";
        break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source = "Source: Window System";
        break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source = "Source Shader Compiler";
        break;

    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source = "Source Third Party";
        break;

    case GL_DEBUG_SOURCE_APPLICATION:
        source = "Source Application";
        break;

    case GL_DEBUG_SOURCE_OTHER:
        source = "Source Other";
        break;
    
    default:
        source = "Source Unknow";
        break;
    }

    switch ( in_type )
    {
    case GL_DEBUG_TYPE_ERROR:
        type = "Type ERROR";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type = "Type Deprecated Behaviour";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type = "Type Undefined Behaviour";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        type = "Type Portability";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        type = "Type Performance";
        break;

    case GL_DEBUG_TYPE_MARKER:
        type = "Type Marker";
        break;

    case GL_DEBUG_TYPE_PUSH_GROUP:
        type = "Type Push Group";
        break;

    case GL_DEBUG_TYPE_POP_GROUP:
        type = "Type Pop Group";
        break;

    case GL_DEBUG_TYPE_OTHER:
        type = "Type Other";
        break;

    default:
        break;
    }

    switch ( in_severity )
    {
    case GL_DEBUG_SEVERITY_HIGH:
        severity = "High Severity";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity = "Medium Severity";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity = "Low Severity";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity = "Low Severity";
        break;
    default:
        severity = "Unknow Severity level";
        break;
    }

    common->Printf( "OpenGL Info: %s %s, from %s :\n * %s", type, severity, source, in_message );
	if( in_severity != GL_DEBUG_SEVERITY_NOTIFICATION )
		SDL_TriggerBreakpoint();
}