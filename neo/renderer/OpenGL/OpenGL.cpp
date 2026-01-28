
#include "precompiled.h"
#include "OpenGL.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_init.h>
#include "renderer_common.h"

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
PFNGLCOLORMASKPROC                          	glColorMask = nullptr;	
PFNGLBLENDFUNCPROC                          	glBlendFunc = nullptr;
PFNGLBLENDFUNCSEPARATEPROC						glBlendFuncSeparate = nullptr;
PFNGLBLENDEQUATIONPROC							glBlendEquation = nullptr;

PFNGLCLEARDEPTHPROC                         	glClearDepth = nullptr;
PFNGLDEPTHMASKPROC                          	glDepthMask = nullptr;
PFNGLDEPTHFUNCPROC                          	glDepthFunc = nullptr;

PFNGLCLEARSTENCILPROC                       	glClearStencil = nullptr;
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

// buffer
PFNGLCREATEBUFFERSPROC                       	glCreateBuffers = nullptr;
PFNGLNAMEDBUFFERSTORAGEPROC                  	glNamedBufferStorage = nullptr;
PFNGLMAPNAMEDBUFFERRANGEPROC                 	glMapNamedBufferRange = nullptr;
PFNGLUNMAPNAMEDBUFFERPROC                    	glUnmapNamedBuffer = nullptr;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC         	glFlushMappedNamedBufferRange = nullptr;
PFNGLNAMEDBUFFERSUBDATAPROC                  	glNamedBufferSubData = nullptr;
PFNGLGETNAMEDBUFFERSUBDATAPROC               	glGetNamedBufferSubData = nullptr;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC              	glCopyNamedBufferSubData = nullptr;


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

// BEATO
PFNGLCREATETEXTURESPROC					        glCreateTextures = nullptr;
PFNGLISTEXTUREPROC                              glIsTexture = nullptr;
PFNGLTEXTURESTORAGE1DPROC				        glTextureStorage1D = nullptr;
PFNGLTEXTURESTORAGE2DPROC				        glTextureStorage2D = nullptr;
PFNGLTEXTURESTORAGE3DPROC				        glTextureStorage3D = nullptr;
PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC            glTextureStorage2DMultisample = nullptr;
PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC            glTextureStorage3DMultisample = nullptr;
PFNGLTEXTURESUBIMAGE1DPROC				        glTextureSubImage1D = nullptr;
PFNGLTEXTURESUBIMAGE2DPROC				        glTextureSubImage2D = nullptr;
PFNGLTEXTURESUBIMAGE3DPROC				        glTextureSubImage3D = nullptr;
PFNGLCOPYTEXTURESUBIMAGE1DPROC                  glCopyTextureSubImage1D = nullptr;
PFNGLCOPYTEXTURESUBIMAGE2DPROC                  glCopyTextureSubImage2D = nullptr;
PFNGLCOPYTEXTURESUBIMAGE3DPROC                  glCopyTextureSubImage3D = nullptr;
PFNGLTEXTUREPARAMETERIVPROC				        glTextureParameteriv = nullptr;
PFNGLTEXTUREPARAMETERFVPROC				        glTextureParameterfv = nullptr;
PFNGLGETTEXTUREPARAMETERIVPROC                  glGetTextureParameteriv = nullptr;
PFNGLGETTEXTUREPARAMETERFVPROC                  glGetTextureParameterfv = nullptr;
PFNGLGETTEXTURELEVELPARAMETERFVPROC             glGetTextureLevelParameterfv = nullptr;
PFNGLGETTEXTURELEVELPARAMETERIVPROC             glGetTextureLevelParameteriv = nullptr;
PFNGLGETTEXTUREIMAGEPROC                        glGetTextureImage = nullptr;
PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC              glGetCompressedTextureImage = nullptr;

// GL_ARB_clear_texture
PFNGLCLEARTEXIMAGEPROC                          glClearTexImage = nullptr;
PFNGLCLEARTEXSUBIMAGEPROC                       glClearTexSubImage = nullptr;

// GL_ARB_get_texture_sub_image
PFNGLGETTEXTURESUBIMAGEPROC                     glGetTextureSubImage = nullptr;
PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC           glGetCompressedTextureSubImage = nullptr;

// GL_ARB_copy_image
PFNGLCOPYIMAGESUBDATAPROC                       glCopyImageSubData = nullptr;
//

// GL_ARB_texture_compression
PFNGLCOMPRESSEDTEXIMAGE2DPROC               	glCompressedTexImage2D = nullptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC            	glCompressedTexSubImage2D = nullptr;
PFNGLGETCOMPRESSEDTEXIMAGEPROC              	glGetCompressedTexImage = nullptr;

// GL_ARB_sampler_objects
PFNGLCREATESAMPLERSPROC                         glCreateSamplers = nullptr;
PFNGLDELETESAMPLERSPROC                         glDeleteSamplers = nullptr;
PFNGLBINDSAMPLERPROC                            glBindSampler = nullptr;
PFNGLBINDSAMPLERSPROC                           glBindSamplers = nullptr;
PFNGLISSAMPLERPROC                              glIsSampler = nullptr;
PFNGLSAMPLERPARAMETERIPROC                      glSamplerParameteri = nullptr;
PFNGLSAMPLERPARAMETERIVPROC                     glSamplerParameteriv = nullptr;
PFNGLSAMPLERPARAMETERFPROC                      glSamplerParameterf = nullptr;
PFNGLSAMPLERPARAMETERFVPROC                     glSamplerParameterfv = nullptr;
PFNGLGETSAMPLERPARAMETERIVPROC                  glGetSamplerParameteriv = nullptr;
PFNGLGETSAMPLERPARAMETERFVPROC                  glGetSamplerParameterfv = nullptr;

// GL_ARB_bindless_texture
PFNGLGETTEXTUREHANDLEARBPROC				    glGetTextureHandleARB = nullptr;
PFNGLGETTEXTURESAMPLERHANDLEARBPROC		    	glGetTextureSamplerHandleARB = nullptr;
PFNGLMAKETEXTUREHANDLERESIDENTARBPROC	    	glMakeTextureHandleResidentARB = nullptr;
PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC     	glMakeTextureHandleNonResidentARB = nullptr;
PFNGLISTEXTUREHANDLERESIDENTARBPROC		    	glIsTextureHandleResidentARB = nullptr;

// GL_ARB_map_buffer_range
PFNGLMAPBUFFERRANGEPROC							glMapBufferRange = nullptr;

PFNGLDRAWARRAYSPROC                          	glDrawArrays = nullptr;

// GL_ARB_draw_elements_base_vertex
PFNGLDRAWELEMENTSBASEVERTEXPROC  				glDrawElementsBaseVertex = nullptr;

PFNGLDRAWARRAYSINSTANCEDPROC					glDrawArraysInstanced = nullptr;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC		glDrawElementsInstancedBaseVertex = nullptr;

PFNGLDISPATCHCOMPUTEPROC                     	glDispatchCompute = nullptr;

// GL_ARB_vertex_array_object
PFNGLGENVERTEXARRAYSPROC						glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC						glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC                 	glDeleteVertexArrays = nullptr;

//
PFNGLCREATEVERTEXARRAYSPROC                  glCreateVertexArrays = nullptr;
PFNGLDISABLEVERTEXARRAYATTRIBPROC            glDisableVertexArrayAttrib = nullptr;
PFNGLENABLEVERTEXARRAYATTRIBPROC             glEnableVertexArrayAttrib = nullptr;
PFNGLVERTEXARRAYELEMENTBUFFERPROC            glVertexArrayElementBuffer = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERPROC             glVertexArrayVertexBuffer = nullptr;
PFNGLVERTEXARRAYATTRIBBINDINGPROC            glVertexArrayAttribBinding = nullptr;
PFNGLVERTEXARRAYATTRIBFORMATPROC             glVertexArrayAttribFormat = nullptr;

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
PFNGLSHADERBINARYPROC                        	glShaderBinary = nullptr;
PFNGLSPECIALIZESHADERPROC                    	glSpecializeShader = nullptr;
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

// GL_ARB_separate_shader_objects
PFNGLBINDPROGRAMPIPELINEPROC                 	glBindProgramPipeline = nullptr;
PFNGLCREATEPROGRAMPIPELINESPROC              	glCreateProgramPipelines = nullptr;
PFNGLDELETEPROGRAMPIPELINESPROC              	glDeleteProgramPipelines = nullptr;
PFNGLVALIDATEPROGRAMPIPELINEPROC             	glValidateProgramPipeline = nullptr;
PFNGLGETPROGRAMPIPELINEIVPROC                	glGetProgramPipelineiv = nullptr;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC           	glGetProgramPipelineInfoLog = nullptr;
PFNGLUSEPROGRAMSTAGESPROC                    	glUseProgramStages = nullptr;
PFNGLACTIVESHADERPROGRAMPROC                 	glActiveShaderProgram = nullptr;
PFNGLPROGRAMUNIFORM1IPROC                    	glProgramUniform1i = nullptr;
PFNGLPROGRAMUNIFORM1IVPROC                   	glProgramUniform1iv = nullptr;
PFNGLPROGRAMUNIFORM1UIVPROC                  	glProgramUniform1uiv = nullptr;

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

// BEATO Begin: use direct state acess
PFNGLCREATERENDERBUFFERSPROC					glCreateRenderbuffers = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC				glNamedRenderbufferStorage = nullptr;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC	glNamedRenderbufferStorageMultisample = nullptr;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC			glNamedFramebufferRenderbuffer = nullptr;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC		glGetNamedRenderbufferParameteriv = nullptr;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC			glCheckNamedFramebufferStatus = nullptr;
PFNGLBLITNAMEDFRAMEBUFFERPROC                	glBlitNamedFramebuffer = nullptr;
// BEATO End

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

PFNGLMEMORYBARRIERPROC                       	glMemoryBarrier = nullptr;
PFNGLMEMORYBARRIERBYREGIONPROC               	glMemoryBarrierByRegion = nullptr;
// TODO: Deprecate GL
void ( APIENTRYP glRasterPos2f )( GLfloat x, GLfloat y );
void ( APIENTRYP glDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRYP glPushAttrib )(GLbitfield mask);
void ( APIENTRYP glPopAttrib )(void);
void ( APIENTRYP glArrayElement )(GLint i);

idCVar r_waylandcompat( "r_waylandcompat", "0", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "wayland compatible framebuffer" );
idCVar r_useOpenGL32( "r_useOpenGL32", "1", CVAR_INTEGER, "0 = OpenGL 2.0, 1 = OpenGL 3.2 compatibility profile, 2 = OpenGL 3.2 core profile", 0, 2 );

static void APIENTRY DebugOutputCall( GLenum in_source, GLenum in_type, GLuint in_id, GLenum in_severity, GLsizei in_length, const GLchar *in_message, const void *in_userParam );

glContextState::glContextState( void )
{
}

glContextState::~glContextState( void )
{
}

static bool QGL_Init( const char* dllname );
static void QGL_Shutdown( void );

/*
===================
glContextState::PreInit

 R_GetModeListForDisplay is called before GLimp_Init(), but SDL needs SDL_Init() first.
 So do that in GLimp_PreInit()
 Calling that function more than once doesn't make a difference
===================
*/
void glContextState::PreInit( void ) // DG: added this function for SDL compatibility
{
	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		if( SDL_Init( SDL_INIT_VIDEO ) )
			common->Error( "Error while initializing SDL: %s", SDL_GetError() );
	}
}

/*
===================
glContextState::Init
===================
*/
bool glContextState::Init( const bool in_stereo, const uint8_t in_multiSamples )
{
	common->Printf( "Initializing OpenGL subsystem\n" );
	
	glContextState::PreInit(); // DG: make sure SDL is initialized
	
    // get window handler
    m_window = static_cast<SDL_Window*>( sys->GetVideoSystem()->WindowHandler() );
	assert( m_window != nullptr );

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
		
		m_context = SDL_GL_CreateContext( m_window );
		
		if( !m_context )
		{
			common->Warning( "Couldn't set GL mode %d/%d/%d: %s", channelcolorbits, tdepthbits, tstencilbits, SDL_GetError() );
			continue;
		}
		
		if( SDL_GL_SetSwapInterval( r_swapInterval.GetInteger() ) < 0 )
			common->Warning( "SDL_GL_SWAP_CONTROL not supported" );
			
		// RB begin
		SDL_GetWindowSize( m_window, &glConfig.nativeScreenWidth, &glConfig.nativeScreenHeight );
		// RB end
		
		glConfig.isFullscreen = ( SDL_GetWindowFlags( m_window ) & SDL_WINDOW_FULLSCREEN ) == SDL_WINDOW_FULLSCREEN;
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
	
	if( !m_context )
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
glContextState::Shutdown
===================
*/
void glContextState::Shutdown( void )
{
	if ( !m_context )
		return;

	SDL_GL_DestroyContext( m_context );

	m_context = nullptr;
}

/*
===================
glContextState::SetScreenParms
===================
*/
bool glContextState::SetScreenParms( const bool in_stereo, const uint8_t in_multiSamples )
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
glContextState::SwapBuffers
===================
*/
void glContextState::SwapBuffers( void )
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
	
	SDL_GL_SwapWindow( m_window );
}

/*
=================
glContextState::SetGamma
=================
*/
void glContextState::SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] )
{
}

/*
=================
glContextState::BindFrameBuffer
=================
*/
void glContextState::BindFrameBuffer(const GLuint in_frameBuffer)
{
	if ( m_state.frameBuffer != in_frameBuffer )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, in_frameBuffer );
		m_state.frameBuffer == in_frameBuffer;
	}
}

/*
=================
glContextState::BindVertexArray
=================
*/
void glContextState::BindVertexArray(const GLuint in_vertexArray)
{
	if( m_state.vertexArray != in_vertexArray )
	{
		glBindVertexArray( in_vertexArray );
		m_state.vertexArray = in_vertexArray;
	} 
}

/*
=================
glContextState::BindProgramPipeline
=================
*/
void glContextState::BindProgramPipeline(const GLuint in_programPipeline)
{
	if ( m_state.program.pipeline != in_programPipeline )
	{
		glBindProgramPipeline( in_programPipeline );
		m_state.program.pipeline != in_programPipeline;
	}
}

/*
=================
glContextState::FaceCull
=================
*/
void glContextState::FaceCull( const GLboolean in_enable, const GLenum in_face )
{
	/// is disable, change nothing
	if( m_state.cullFaceState.enable == GL_FALSE && in_enable == GL_FALSE )
		return;

	if ( m_state.cullFaceState.enable == GL_FALSE )
	{
		glEnable( GL_CULL_FACE );

		if ( m_state.cullFaceState.face != in_face )
		{
			glCullFace( in_face );
			m_state.cullFaceState.face = in_face; 
		}
		
	}
	else
	{
		// disable face culling
		glDisable( GL_CULL_FACE );
	}

	m_state.cullFaceState.enable = in_enable;
}

void glContextState::DepthTest(const GLboolean in_enable, const GLenum in_func)
{
	if ( m_state.depthFuncState.depthTest == GL_FALSE && in_enable == GL_FALSE )
		return;

	if( m_state.depthFuncState.depthTest == GL_FALSE && in_enable == GL_TRUE )
	{
		glEnable( GL_DEPTH_TEST );
		m_state.depthFuncState.depthTest = GL_TRUE;
	}
	else
	{
		glDisable( GL_DEPTH_TEST );
		m_state.depthFuncState.depthTest = GL_FALSE;	
	}

	if (m_state.depthFuncState.depthFunc != in_func )
	{
		glDepthFunc( in_func );
		m_state.depthFuncState.depthFunc = in_func;
	}	
}

void glContextState::Blending( const GLboolean in_enable, const GLenum in_SRCFactor, const GLenum in_SRCAlphaFactor, const GLenum in_DSTFactor, const GLenum in_DSTAlphaFactor, const GLenum in_blendOp )
{
	// ignore if blend is blend is disable
	if ( m_state.blendingState.enable == GL_FALSE && in_enable == GL_FALSE )
		return;

	if( m_state.blendingState.enable == GL_FALSE && in_enable == GL_TRUE )
	{
		glEnable( GL_BLEND );
		m_state.blendingState.enable = GL_TRUE;
	}
	else
	{
		glDisable( GL_BLEND );
		m_state.blendingState.enable = GL_FALSE;	
	}

	if (	m_state.blendingState.srcFactor != in_SRCFactor || 
			m_state.blendingState.srcFactorAlpha != in_SRCAlphaFactor || 
			m_state.blendingState.dstFactor != in_DSTFactor || 
			m_state.blendingState.dstFactorAlpha != in_DSTAlphaFactor )
	{
		glBlendFuncSeparate( in_SRCFactor, in_DSTFactor, in_SRCAlphaFactor, in_DSTAlphaFactor );
	}
	
	if ( m_state.blendingState.blendOp != in_blendOp )
		glBlendEquation( in_blendOp );
}

void glContextState::StencilTest(const GLboolean in_enable, const GLenum in_face, const GLenum in_pass, const GLenum in_fail, const GLenum in_Zfail)
{
	if ( m_state.stencilState.enable == GL_FALSE && in_enable == GL_FALSE )
		return;	

	if( m_state.stencilState.enable == GL_FALSE && in_enable == GL_TRUE )
	{
		glEnable( GL_STENCIL_TEST );
		m_state.stencilState.enable = GL_TRUE;
	}
	else
	{
		glDisable( GL_STENCIL_TEST );
		m_state.stencilState.enable = GL_FALSE;	
	}

	if ( in_face != m_state.stencilState.face || 
		 in_pass != m_state.stencilState.pass  ||
		 in_fail != m_state.stencilState.fail ||
		 in_Zfail != m_state.stencilState.zfail )
	{
		glStencilOpSeparate( in_face, in_fail, in_Zfail, in_pass );
	}
}

void glContextState::PolygonMode( const GLenum in_face, const GLenum in_mode )
{
	if ( m_state.polygonMode.face == in_face && m_state.polygonMode.mode == in_mode )
		return;

	glPolygonMode( in_face, in_mode );
	m_state.polygonMode.face = in_face;
	m_state.polygonMode.mode = in_mode;
}

template< typename _t >
static inline void LoadGLProc( _t proc, const char* pName )
{
    proc = reinterpret_cast<_t>( SDL_GL_GetProcAddress( pName ) );
}

#define GET_GL_PROC( P ) LoadGLProc( P, #P )

bool QGL_Init( const char* dllname )
{
	GET_GL_PROC( glGetError );
	GET_GL_PROC( glGetIntegerv );
	GET_GL_PROC( glGetFloatv );
	GET_GL_PROC( glGetString );
	GET_GL_PROC( glGetStringi );

	GET_GL_PROC( glFlush );
	GET_GL_PROC( glFinish );

	GET_GL_PROC( glEnable );
	GET_GL_PROC( glDisable );

	GET_GL_PROC( glClear );
	
	GET_GL_PROC( glClearColor );
	GET_GL_PROC( glColorMask );
	GET_GL_PROC( glBlendFunc );
	GET_GL_PROC( glBlendFuncSeparate );
	GET_GL_PROC( glBlendEquation );
	
	GET_GL_PROC( glClearDepth );
	GET_GL_PROC( glDepthMask );
	GET_GL_PROC( glDepthFunc );
	
	GET_GL_PROC( glClearStencil );
	GET_GL_PROC( glStencilFunc );

	GET_GL_PROC( glScissor );
	GET_GL_PROC( glViewport );

	GET_GL_PROC( glPolygonMode );
	GET_GL_PROC( glPolygonOffset );
	GET_GL_PROC( glCullFace );

	GET_GL_PROC( glDrawBuffer );
	GET_GL_PROC( glReadBuffer );

	GET_GL_PROC( glStencilOp );

	GET_GL_PROC( glLineWidth );
	GET_GL_PROC( glPointSize );

	GET_GL_PROC( glReadPixels );

	// GL_ARB_multitexture
	GET_GL_PROC( glActiveTexture );
	GET_GL_PROC( glBindTexture );

	GET_GL_PROC( glBindTextureUnit );

	// GL_ARB_vertex_buffer_object
	GET_GL_PROC( glBindBuffer );
	GET_GL_PROC( glBindBufferRange );
	GET_GL_PROC( glDeleteBuffers );
	GET_GL_PROC( glGenBuffers );
	GET_GL_PROC( glIsBuffer );
	GET_GL_PROC( glBufferData );
	GET_GL_PROC( glBufferSubData );
	GET_GL_PROC( glGetBufferSubData );
	GET_GL_PROC( glMapBuffer );
	GET_GL_PROC( glUnmapBuffer );
	GET_GL_PROC( glGetBufferParameteriv );
	GET_GL_PROC( glGetBufferPointerv );

	GET_GL_PROC( glCreateBuffers );
	GET_GL_PROC( glNamedBufferStorage );
	GET_GL_PROC( glMapNamedBufferRange );
	GET_GL_PROC( glUnmapNamedBuffer );
	GET_GL_PROC( glFlushMappedNamedBufferRange );
	GET_GL_PROC( glNamedBufferSubData );
	GET_GL_PROC( glGetNamedBufferSubData );
	GET_GL_PROC( glCopyNamedBufferSubData );

	GET_GL_PROC( glPixelStorei );

	GET_GL_PROC( glGenTextures );
	GET_GL_PROC( glDeleteTextures );
	GET_GL_PROC( glTexImage2D );
	GET_GL_PROC( glTexSubImage2D );
	GET_GL_PROC( glTexImage3D );
	GET_GL_PROC( glCopyTexImage2D );
	GET_GL_PROC( glTexParameterf );
	GET_GL_PROC( glTexParameteri );
	GET_GL_PROC( glTexParameterfv );
	GET_GL_PROC( glTexParameteriv );

	GET_GL_PROC( glCreateTextures );
	GET_GL_PROC( glIsTexture );
	GET_GL_PROC( glTextureStorage1D );
	GET_GL_PROC( glTextureStorage2D );
	GET_GL_PROC( glTextureStorage3D );
	GET_GL_PROC( glTextureStorage2DMultisample );
	GET_GL_PROC( glTextureStorage3DMultisample );
	GET_GL_PROC( glTextureSubImage1D );
	GET_GL_PROC( glTextureSubImage2D );
	GET_GL_PROC( glTextureSubImage3D );
	GET_GL_PROC( glCopyTextureSubImage1D );
	GET_GL_PROC( glCopyTextureSubImage2D );
	GET_GL_PROC( glCopyTextureSubImage3D );
	GET_GL_PROC( glTextureParameteriv );
	GET_GL_PROC( glTextureParameterfv );
	GET_GL_PROC( glGetTextureParameteriv );
	GET_GL_PROC( glGetTextureParameterfv );
	GET_GL_PROC( glGetTextureLevelParameterfv );
	GET_GL_PROC( glGetTextureLevelParameteriv );
	GET_GL_PROC( glGetTextureImage );
	GET_GL_PROC( glGetCompressedTextureImage );

	GET_GL_PROC( glClearTexImage );
	GET_GL_PROC( glClearTexSubImage );

	GET_GL_PROC( glGetTextureSubImage ); 
	GET_GL_PROC( glGetCompressedTextureSubImage ); 

	GET_GL_PROC( glCopyImageSubData );

	// GL_ARB_texture_compression
	GET_GL_PROC( glCompressedTexImage2D );
	GET_GL_PROC( glCompressedTexSubImage2D );
	GET_GL_PROC( glGetCompressedTexImage );

	// GL_ARB_sampler_objects
	GET_GL_PROC( glCreateSamplers );
	GET_GL_PROC( glDeleteSamplers );
	GET_GL_PROC( glBindSampler );
	GET_GL_PROC( glBindSamplers );
	GET_GL_PROC( glIsSampler );
	GET_GL_PROC( glSamplerParameteri );
	GET_GL_PROC( glSamplerParameteriv );
	GET_GL_PROC( glSamplerParameterf );
	GET_GL_PROC( glSamplerParameterfv );
	GET_GL_PROC( glGetSamplerParameteriv );
	GET_GL_PROC( glGetSamplerParameterfv );

	// GL_ARB_bindless_texture
	GET_GL_PROC( glGetTextureHandleARB );
	GET_GL_PROC( glGetTextureSamplerHandleARB );
	GET_GL_PROC( glMakeTextureHandleResidentARB );
	GET_GL_PROC( glMakeTextureHandleNonResidentARB );
	GET_GL_PROC( glIsTextureHandleResidentARB );

	// GL_ARB_map_buffer_range
	GET_GL_PROC( glMapBufferRange );

	GET_GL_PROC( glDrawArrays );
	GET_GL_PROC( glDrawArraysInstanced );
	GET_GL_PROC( glDrawElementsInstancedBaseVertex );

	// GL_ARB_draw_elements_base_vertex
	GET_GL_PROC( glDrawElementsBaseVertex );

	GET_GL_PROC( glDispatchCompute );

	// GL_ARB_vertex_array_object
	GET_GL_PROC( glGenVertexArrays );
	GET_GL_PROC( glBindVertexArray );
	GET_GL_PROC( glDeleteVertexArrays );

	GET_GL_PROC( glCreateVertexArrays );
	GET_GL_PROC( glDisableVertexArrayAttrib );
	GET_GL_PROC( glEnableVertexArrayAttrib );
	GET_GL_PROC( glVertexArrayElementBuffer );
	GET_GL_PROC( glVertexArrayVertexBuffer );
	GET_GL_PROC( glVertexArrayAttribBinding );
	GET_GL_PROC( glVertexArrayAttribFormat );

	// GL_ARB_vertex_program / GL_ARB_fragment_program
	GET_GL_PROC( glVertexAttribPointer );
	GET_GL_PROC( glEnableVertexAttribArray );
	GET_GL_PROC( glDisableVertexAttribArray );

	// GLSL / OpenGL 2.0
	GET_GL_PROC( glCreateShader );
	GET_GL_PROC( glDeleteShader );
	GET_GL_PROC( glShaderSource );
	GET_GL_PROC( glCompileShader );
	GET_GL_PROC( glShaderBinary );
	GET_GL_PROC( glCompileShader );
	GET_GL_PROC( glGetShaderiv );
	GET_GL_PROC( glGetShaderInfoLog );
	GET_GL_PROC( glCreateProgram );
	GET_GL_PROC( glDeleteProgram );
	GET_GL_PROC( glAttachShader );
	GET_GL_PROC( glDetachShader );
	GET_GL_PROC( glLinkProgram );
	GET_GL_PROC( glUseProgram );
	GET_GL_PROC( glGetProgramiv );
	GET_GL_PROC( glGetProgramInfoLog );
	GET_GL_PROC( glProgramParameteri );
	GET_GL_PROC( glBindAttribLocation );
	GET_GL_PROC( glGetUniformLocation );
	GET_GL_PROC( glUniform1i );
	GET_GL_PROC( glUniform4fv );

	// GL_ARB_separate_shader_objects
	GET_GL_PROC( glBindProgramPipeline );
	GET_GL_PROC( glCreateProgramPipelines );
	GET_GL_PROC( glDeleteProgramPipelines );
	GET_GL_PROC( glValidateProgramPipeline );
	GET_GL_PROC( glGetProgramPipelineiv );
	GET_GL_PROC( glGetProgramPipelineInfoLog );
	GET_GL_PROC( glUseProgramStages );
	GET_GL_PROC( glActiveShaderProgram );
	GET_GL_PROC( glProgramUniform1i );
	GET_GL_PROC( glProgramUniform1iv );
	GET_GL_PROC( glProgramUniform1uiv );

	// foresthale 2014-02-18: added qglDrawbuffers
	GET_GL_PROC( glDrawBuffers );

	// foresthale 2014-02-16: added GL_ARB_framebuffer_object
	GET_GL_PROC( glIsRenderbuffer );
	GET_GL_PROC( glBindRenderbuffer );
	GET_GL_PROC( glDeleteRenderbuffers );
	GET_GL_PROC( glGenRenderbuffers );
	GET_GL_PROC( glRenderbufferStorage );
	GET_GL_PROC( glGetRenderbufferParameteriv );
	GET_GL_PROC( glIsFramebuffer );
	GET_GL_PROC( glBindFramebuffer );
	GET_GL_PROC( glDeleteFramebuffers );
	GET_GL_PROC( glGenFramebuffers );
	GET_GL_PROC( glCheckFramebufferStatus );
	GET_GL_PROC( glFramebufferTexture1D );
	GET_GL_PROC( glFramebufferTexture2D );
	GET_GL_PROC( glFramebufferTexture3D );
	GET_GL_PROC( glFramebufferRenderbuffer );
	GET_GL_PROC( glGetFramebufferAttachmentParameteriv );
	GET_GL_PROC( glGenerateMipmap );
	GET_GL_PROC( glBlitFramebuffer );
	GET_GL_PROC( glRenderbufferStorageMultisample );
	GET_GL_PROC( glFramebufferTextureLayer );

// BEATO Begin: use direct state acess
	GET_GL_PROC( glCreateRenderbuffers );
	GET_GL_PROC( glNamedRenderbufferStorage );
	GET_GL_PROC( glNamedRenderbufferStorageMultisample );
	GET_GL_PROC( glNamedFramebufferRenderbuffer );
	GET_GL_PROC( glGetNamedRenderbufferParameteriv );
	GET_GL_PROC( glCheckNamedFramebufferStatus );
	GET_GL_PROC( glBlitNamedFramebuffer );
// BEATO End


	// GL_ARB_uniform_buffer_object
	GET_GL_PROC( glGetUniformBlockIndex );
	GET_GL_PROC( glUniformBlockBinding );

	// GL_ATI_separate_stencil / OpenGL 2.0
	GET_GL_PROC( glStencilOpSeparate );
	GET_GL_PROC( glStencilFuncSeparate );

	// GL_EXT_depth_bounds_test
	GET_GL_PROC( glDepthBoundsEXT );

	// GL_ARB_sync
	GET_GL_PROC( glFenceSync );
	GET_GL_PROC( glIsSync );
	GET_GL_PROC( glClientWaitSync );
	GET_GL_PROC( glDeleteSync );

	// GL_ARB_occlusion_query
	GET_GL_PROC( glGenQueries );
	GET_GL_PROC( glDeleteQueries );
	GET_GL_PROC( glIsQuery );
	GET_GL_PROC( glBeginQuery );
	GET_GL_PROC( glEndQuery );
	GET_GL_PROC( glGetQueryiv );
	GET_GL_PROC( glGetQueryObjectiv );
	GET_GL_PROC( glGetQueryObjectuiv );

	// GL_ARB_timer_query / GL_EXT_timer_query
	GET_GL_PROC( glGetQueryObjectui64v );

	// GL_ARB_debug_output
	GET_GL_PROC( glDebugMessageControl );
	GET_GL_PROC( glDebugMessageInsert );
	GET_GL_PROC( glDebugMessageCallback );
	GET_GL_PROC( glGetDebugMessageLog );

	GET_GL_PROC( glMemoryBarrier );
	GET_GL_PROC( glMemoryBarrierByRegion );

	glRasterPos2f = (decltype(glRasterPos2f))SDL_GL_GetProcAddress( "glRasterPos2f" );

	glDrawPixels = ( decltype( glDrawPixels ) )SDL_GL_GetProcAddress( "glDrawPixels" );
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