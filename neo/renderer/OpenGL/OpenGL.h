
#ifndef __OPENGL_H__
#define __OPENGL_H__

#include <SDL3/SDL_video.h>
#include <GL/glcorearb.h>

//
// Matrix Mode 
#define GL_MATRIX_MODE				0x0BA0
#define GL_MODELVIEW				0x1700
#define GL_PROJECTION				0x1701
#define GL_TEXTURE                  0x1702
#define GL_POLYGON				    0x0009
#define GL_ALL_ATTRIB_BITS			0xFFFFFFFF
//

typedef void (APIENTRYP PFNGLDEPTHBOUNDSEXTPROC) (GLclampd zmin, GLclampd zmax);
#define GL_DEPTH_BOUNDS_TEST_EXT          0x8890
#define GL_DEPTH_BOUNDS_EXT               0x8891

extern PFNGLGETERRORPROC                            glGetError;
extern PFNGLGETINTEGERVPROC                         glGetIntegerv;
extern PFNGLGETFLOATVPROC                           glGetFloatv;
extern PFNGLGETSTRINGPROC                           glGetString;
extern PFNGLGETSTRINGIPROC							glGetStringi;

extern PFNGLFLUSHPROC                               glFlush;
extern PFNGLFINISHPROC                              glFinish;

extern PFNGLENABLEPROC                              glEnable;
extern PFNGLDISABLEPROC                             glDisable;

extern PFNGLCLEARPROC                               glClear;

extern PFNGLCLEARCOLORPROC                          glClearColor;
extern PFNGLCLEARSTENCILPROC                        glClearStencil;
extern PFNGLCLEARDEPTHPROC                          glClearDepth;

extern PFNGLCOLORMASKPROC                           glColorMask;
extern PFNGLDEPTHMASKPROC                           glDepthMask;

extern PFNGLBLENDFUNCPROC                           glBlendFunc;
extern PFNGLDEPTHFUNCPROC                           glDepthFunc;
extern PFNGLSTENCILFUNCPROC                         glStencilFunc;

extern PFNGLSCISSORPROC                             glScissor;
extern PFNGLVIEWPORTPROC                            glViewport;

extern PFNGLPOLYGONMODEPROC                         glPolygonMode;
extern PFNGLPOLYGONOFFSETPROC                       glPolygonOffset;
extern PFNGLCULLFACEPROC                            glCullFace;

extern PFNGLDRAWBUFFERPROC                          glDrawBuffer;
extern PFNGLREADBUFFERPROC                          glReadBuffer;

extern PFNGLSTENCILOPPROC                           glStencilOp;

extern PFNGLLINEWIDTHPROC                           glLineWidth;
extern PFNGLPOINTSIZEPROC                           glPointSize;

extern PFNGLREADPIXELSPROC                          glReadPixels;

// GL_ARB_multitexture
extern PFNGLACTIVETEXTUREPROC						glActiveTexture;
extern PFNGLBINDTEXTUREPROC                         glBindTexture;

// GL_EXT_direct_state_access
extern PFNGLBINDMULTITEXTUREEXTPROC                 glBindMultiTextureEXT;

// GL_ARB_vertex_buffer_object
extern PFNGLBINDBUFFERPROC                          glBindBuffer;
extern PFNGLBINDBUFFERRANGEPROC						glBindBufferRange;
extern PFNGLDELETEBUFFERSPROC						glDeleteBuffers;
extern PFNGLGENBUFFERSPROC							glGenBuffers;
extern PFNGLISBUFFERPROC							glIsBuffer;
extern PFNGLBUFFERDATAPROC							glBufferData;
extern PFNGLBUFFERSUBDATAPROC						glBufferSubData;
extern PFNGLGETBUFFERSUBDATAPROC					glGetBufferSubData;
extern PFNGLMAPBUFFERPROC							glMapBuffer;
extern PFNGLUNMAPBUFFERPROC							glUnmapBuffer;
extern PFNGLGETBUFFERPARAMETERIVPROC				glGetBufferParameteriv;
extern PFNGLGETBUFFERPOINTERVPROC					glGetBufferPointerv;

extern PFNGLPIXELSTOREIPROC                         glPixelStorei;

extern PFNGLGENTEXTURESPROC                         glGenTextures;
extern PFNGLDELETETEXTURESPROC                      glDeleteTextures;
extern PFNGLTEXIMAGE2DPROC                          glTexImage2D;
extern PFNGLTEXSUBIMAGE2DPROC                       glTexSubImage2D;
extern PFNGLTEXIMAGE3DPROC							glTexImage3D;
extern PFNGLCOPYTEXIMAGE2DPROC                      glCopyTexImage2D;
extern PFNGLTEXPARAMETERFPROC                       glTexParameterf;
extern PFNGLTEXPARAMETERIPROC                       glTexParameteri;
extern PFNGLTEXPARAMETERFVPROC                      glTexParameterfv;
extern PFNGLTEXPARAMETERIVPROC                      glTexParameteriv;

// GL_ARB_texture_compression
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC                glCompressedTexImage2D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC             glCompressedTexSubImage2D;
extern PFNGLGETCOMPRESSEDTEXIMAGEPROC               glGetCompressedTexImage;

// GL_ARB_map_buffer_range
extern PFNGLMAPBUFFERRANGEPROC						glMapBufferRange;

// GL_ARB_draw_elements_base_vertex
extern PFNGLDRAWELEMENTSBASEVERTEXPROC  			glDrawElementsBaseVertex;

// GL_ARB_vertex_array_object
extern PFNGLGENVERTEXARRAYSPROC						glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC						glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC                  glDeleteVertexArrays;

// GL_ARB_vertex_program / GL_ARB_fragment_program
//PFNGLPROGRAMSTRINGPROC					glProgramString;
extern PFNGLVERTEXATTRIBPOINTERPROC					glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC				glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC			glDisableVertexAttribArray;

// GLSL / OpenGL 2.0
extern PFNGLCREATESHADERPROC						glCreateShader;
extern PFNGLDELETESHADERPROC						glDeleteShader;
extern PFNGLSHADERSOURCEPROC						glShaderSource;
extern PFNGLCOMPILESHADERPROC						glCompileShader;
extern PFNGLGETSHADERIVPROC							glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC					glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC						glCreateProgram;
extern PFNGLDELETEPROGRAMPROC						glDeleteProgram;
extern PFNGLATTACHSHADERPROC						glAttachShader;
extern PFNGLDETACHSHADERPROC						glDetachShader;
extern PFNGLLINKPROGRAMPROC							glLinkProgram;
extern PFNGLUSEPROGRAMPROC							glUseProgram;
extern PFNGLGETPROGRAMIVPROC						glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC					glGetProgramInfoLog;
extern PFNGLPROGRAMPARAMETERIPROC					glProgramParameteri;
extern PFNGLBINDATTRIBLOCATIONPROC					glBindAttribLocation;
extern PFNGLGETUNIFORMLOCATIONPROC					glGetUniformLocation;
extern PFNGLUNIFORM1IPROC							glUniform1i;
extern PFNGLUNIFORM4FVPROC							glUniform4fv;

// foresthale 2014-02-18: added qglDrawbuffers
extern PFNGLDRAWBUFFERSPROC                         glDrawBuffers;

// foresthale 2014-02-16: added GL_ARB_framebuffer_object
extern PFNGLISRENDERBUFFERPROC                      glIsRenderbuffer;
extern PFNGLBINDRENDERBUFFERPROC                    glBindRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC                 glDeleteRenderbuffers;
extern PFNGLGENRENDERBUFFERSPROC                    glGenRenderbuffers;
extern PFNGLRENDERBUFFERSTORAGEPROC                 glRenderbufferStorage;
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC          glGetRenderbufferParameteriv;
extern PFNGLISFRAMEBUFFERPROC                       glIsFramebuffer;
extern PFNGLBINDFRAMEBUFFERPROC                     glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC                  glDeleteFramebuffers;
extern PFNGLGENFRAMEBUFFERSPROC                     glGenFramebuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC              glCheckFramebufferStatus;
extern PFNGLFRAMEBUFFERTEXTURE1DPROC                glFramebufferTexture1D;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC                glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTURE3DPROC                glFramebufferTexture3D;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC             glFramebufferRenderbuffer;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
extern PFNGLGENERATEMIPMAPPROC                      glGenerateMipmap;
extern PFNGLBLITFRAMEBUFFERPROC                     glBlitFramebuffer;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC      glRenderbufferStorageMultisample;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC             glFramebufferTextureLayer;

// GL_ARB_uniform_buffer_object
extern PFNGLGETUNIFORMBLOCKINDEXPROC				glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC					glUniformBlockBinding;

// GL_ATI_separate_stencil / OpenGL 2.0
extern PFNGLSTENCILOPSEPARATEPROC					glStencilOpSeparate;
extern PFNGLSTENCILFUNCSEPARATEPROC					glStencilFuncSeparate;

// GL_EXT_depth_bounds_test
extern PFNGLDEPTHBOUNDSEXTPROC                 		glDepthBoundsEXT;

// GL_ARB_sync
extern PFNGLFENCESYNCPROC							glFenceSync ;
extern PFNGLISSYNCPROC								glIsSync;
extern PFNGLCLIENTWAITSYNCPROC						glClientWaitSync;
extern PFNGLDELETESYNCPROC							glDeleteSync;

// GL_ARB_occlusion_query
extern PFNGLGENQUERIESPROC							glGenQueries;
extern PFNGLDELETEQUERIESPROC						glDeleteQueries;
extern PFNGLISQUERYPROC								glIsQuery;
extern PFNGLBEGINQUERYPROC							glBeginQuery;
extern PFNGLENDQUERYPROC							glEndQuery;
extern PFNGLGETQUERYIVPROC							glGetQueryiv;
extern PFNGLGETQUERYOBJECTIVPROC					glGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC					glGetQueryObjectuiv;

// GL_ARB_timer_query / GL_EXT_timer_query
extern PFNGLGETQUERYOBJECTUI64VPROC					glGetQueryObjectui64v;

// GL_ARB_debug_output
extern PFNGLDEBUGMESSAGECONTROLPROC					glDebugMessageControl;
extern PFNGLDEBUGMESSAGEINSERTPROC					glDebugMessageInsert;
extern PFNGLDEBUGMESSAGECALLBACKPROC				glDebugMessageCallback;
extern PFNGLGETDEBUGMESSAGELOGPROC					glGetDebugMessageLog;

//#include <SDL3/SDL_opengl.h>
//#include <GL/glext.h>

// TODO: Deprecate GL
extern void ( APIENTRYP glRasterPos2f )( GLfloat x, GLfloat y );
extern void ( APIENTRYP glOrtho )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val );
extern void ( APIENTRYP glBegin )( GLenum mode );
extern void ( APIENTRYP glDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void ( APIENTRYP glEnd )( void );
extern void ( APIENTRYP glVertex3f )( GLfloat x, GLfloat y, GLfloat z );
extern void ( APIENTRYP glVertex3fv )( const GLfloat *v );
extern void ( APIENTRYP glColor4fv )( const GLfloat *v );
extern void ( APIENTRYP glColor4ubv )(const GLubyte *v);
extern void ( APIENTRYP glColor3f )( GLfloat red, GLfloat green, GLfloat blue );
extern void ( APIENTRYP glColor3fv )( const GLfloat *v );
extern void ( APIENTRYP glColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void ( APIENTRYP glLoadIdentity )( void );
extern void ( APIENTRYP glPushMatrix )( void );
extern void ( APIENTRYP glPopMatrix )(void);
extern void ( APIENTRYP glMatrixMode )( GLenum mode );
extern void ( APIENTRYP glLoadMatrixf )( const GLfloat *m );
extern void ( APIENTRYP glPushAttrib )(GLbitfield mask);
extern void ( APIENTRYP glPopAttrib )(void);
extern void ( APIENTRYP glArrayElement )(GLint i);

// DG: R_GetModeListForDisplay is called before GLimp_Init(), but SDL needs SDL_Init() first.
// So add PreInit for platforms that need it, others can just stub it.
void GLimp_PreInit( void );

// If the desired mode can't be set satisfactorily, false will be returned.
// If succesful, sets glConfig.nativeScreenWidth, glConfig.nativeScreenHeight, and glConfig.pixelAspect
// The renderer will then reset the glimpParms to "safe mode" of 640x480
// fullscreen and try again.  If that also fails, the error will be fatal.
bool GLimp_Init( const bool in_stereo, const uint8_t in_multiSamples );

// will set up gl up with the new parms
bool GLimp_SetScreenParms( const bool in_stereo, const uint8_t in_multiSamples );

// Destroys the rendering context, closes the window, resets the resolution,
// and resets the gamma ramps.
void GLimp_Shutdown( void );

// Sets the hardware gamma ramps for gamma and brightness adjustment.
// These are now taken as 16 bit values, so we can take full advantage
// of dacs with >8 bits of precision
void GLimp_SetGamma( uint16_t red[256], uint16_t green[256], uint16_t blue[256] );

void GLimp_EnableLogging( bool enable );

void GLimp_SwapBuffers( void );

#endif //!__OPENGL_H__