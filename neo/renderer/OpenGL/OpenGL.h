
#ifndef __OPENGL_H__
#define __OPENGL_H__

#include <SDL3/SDL_video.h>
#include <GL/glcorearb.h>

// TODO:
// GLint maxSamples = 0;
// glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

//
// Matrix Mode 
#define GL_MATRIX_MODE				0x0BA0
#define GL_MODELVIEW				0x1700
#define GL_PROJECTION				0x1701
#define GL_TEXTURE                  0x1702
#define GL_POLYGON				    0x0009
#define GL_ALL_ATTRIB_BITS			0xFFFFFFFF
//

// BEATO Begin:
// GL_EXT_depth_bounds_test
#ifndef GL_EXT_depth_bounds_test
#define GL_EXT_depth_bounds_test 1
#define GL_DEPTH_BOUNDS_TEST_EXT 0x8890
#define GL_DEPTH_BOUNDS_EXT 0x8891
typedef void (APIENTRYP PFNGLDEPTHBOUNDSEXTPROC) (GLclampd zmin, GLclampd zmax);
#endif //!GL_EXT_depth_bounds_test

// GL_EXT_texture_sRGB
#ifndef GL_EXT_texture_sRGB
#define GL_EXT_texture_sRGB 1
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif //!GL_EXT_texture_sRGB
// BEATO End

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
extern PFNGLCOLORMASKPROC                           glColorMask;
extern PFNGLBLENDFUNCPROC                           glBlendFunc;
extern PFNGLBLENDFUNCSEPARATEPROC					glBlendFuncSeparate;
extern PFNGLBLENDEQUATIONPROC						glBlendEquation;

extern PFNGLCLEARDEPTHPROC                          glClearDepth;
extern PFNGLDEPTHMASKPROC                           glDepthMask;
extern PFNGLDEPTHFUNCPROC                           glDepthFunc;

extern PFNGLCLEARSTENCILPROC                        glClearStencil;
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

extern PFNGLBINDTEXTUREUNITPROC                     glBindTextureUnit;

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

// buffer
extern PFNGLCREATEBUFFERSPROC                       glCreateBuffers;
extern PFNGLNAMEDBUFFERSTORAGEPROC                  glNamedBufferStorage;
extern PFNGLMAPNAMEDBUFFERRANGEPROC                 glMapNamedBufferRange;
extern PFNGLUNMAPNAMEDBUFFERPROC                    glUnmapNamedBuffer;
extern PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC         glFlushMappedNamedBufferRange;
extern PFNGLNAMEDBUFFERSUBDATAPROC                  glNamedBufferSubData;
extern PFNGLGETNAMEDBUFFERSUBDATAPROC               glGetNamedBufferSubData;
extern PFNGLCOPYNAMEDBUFFERSUBDATAPROC              glCopyNamedBufferSubData;

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

extern PFNGLCREATETEXTURESPROC                      glCreateTextures;
extern PFNGLISTEXTUREPROC                           glIsTexture;
extern PFNGLTEXTURESTORAGE1DPROC                    glTextureStorage1D;
extern PFNGLTEXTURESTORAGE2DPROC                    glTextureStorage2D;
extern PFNGLTEXTURESTORAGE3DPROC                    glTextureStorage3D;
extern PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC         glTextureStorage2DMultisample;
extern PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC         glTextureStorage3DMultisample;
extern PFNGLTEXTURESUBIMAGE1DPROC                   glTextureSubImage1D;
extern PFNGLTEXTURESUBIMAGE2DPROC                   glTextureSubImage2D;
extern PFNGLTEXTURESUBIMAGE3DPROC                   glTextureSubImage3D;
extern PFNGLCOPYTEXTURESUBIMAGE1DPROC               glCopyTextureSubImage1D;
extern PFNGLCOPYTEXTURESUBIMAGE2DPROC               glCopyTextureSubImage2D;
extern PFNGLCOPYTEXTURESUBIMAGE3DPROC               glCopyTextureSubImage3D;
extern PFNGLTEXTUREPARAMETERIVPROC                  glTextureParameteriv;
extern PFNGLTEXTUREPARAMETERFVPROC                  glTextureParameterfv;
extern PFNGLGETTEXTUREPARAMETERIVPROC               glGetTextureParameteriv;
extern PFNGLGETTEXTUREPARAMETERFVPROC               glGetTextureParameterfv;
extern PFNGLGETTEXTURELEVELPARAMETERFVPROC          glGetTextureLevelParameterfv;
extern PFNGLGETTEXTURELEVELPARAMETERIVPROC          glGetTextureLevelParameteriv;
extern PFNGLGETTEXTUREIMAGEPROC                     glGetTextureImage;
extern PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC           glGetCompressedTextureImage;

// GL_ARB_clear_texture
extern PFNGLCLEARTEXIMAGEPROC                       glClearTexImage;
extern PFNGLCLEARTEXSUBIMAGEPROC                    glClearTexSubImage;

// GL_ARB_get_texture_sub_image
extern PFNGLGETTEXTURESUBIMAGEPROC                  glGetTextureSubImage;
extern PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC        glGetCompressedTextureSubImage;

// GL_ARB_copy_image
extern PFNGLCOPYIMAGESUBDATAPROC                    glCopyImageSubData;

// GL_ARB_sampler_objects
extern PFNGLCREATESAMPLERSPROC                      glCreateSamplers;
extern PFNGLDELETESAMPLERSPROC                      glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC                         glBindSampler;
extern PFNGLBINDSAMPLERSPROC                        glBindSamplers;
extern PFNGLISSAMPLERPROC                           glIsSampler;
extern PFNGLSAMPLERPARAMETERIPROC                   glSamplerParameteri;
extern PFNGLSAMPLERPARAMETERIVPROC                  glSamplerParameteriv;
extern PFNGLSAMPLERPARAMETERFPROC                   glSamplerParameterf;
extern PFNGLSAMPLERPARAMETERFVPROC                  glSamplerParameterfv;
extern PFNGLGETSAMPLERPARAMETERIVPROC               glGetSamplerParameteriv;
extern PFNGLGETSAMPLERPARAMETERFVPROC               glGetSamplerParameterfv;

// GL_ARB_bindless_texture
extern PFNGLGETTEXTUREHANDLEARBPROC				    glGetTextureHandleARB;
extern PFNGLGETTEXTURESAMPLERHANDLEARBPROC		    glGetTextureSamplerHandleARB;
extern PFNGLMAKETEXTUREHANDLERESIDENTARBPROC	    glMakeTextureHandleResidentARB;
extern PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC     glMakeTextureHandleNonResidentARB;
extern PFNGLISTEXTUREHANDLERESIDENTARBPROC		    glIsTextureHandleResidentARB;

// GL_ARB_map_buffer_range
extern PFNGLMAPBUFFERRANGEPROC						glMapBufferRange;

extern PFNGLDRAWARRAYSPROC                          glDrawArrays;

// GL_ARB_draw_elements_base_vertex
extern PFNGLDRAWELEMENTSBASEVERTEXPROC  			glDrawElementsBaseVertex;

extern PFNGLDRAWARRAYSINSTANCEDPROC					glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC     glDrawElementsInstancedBaseVertex;

extern PFNGLDISPATCHCOMPUTEPROC                     glDispatchCompute;

// GL_ARB_vertex_array_object
extern PFNGLGENVERTEXARRAYSPROC						glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC						glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC                  glDeleteVertexArrays;

// TODO: Modernize
extern PFNGLCREATEVERTEXARRAYSPROC                  glCreateVertexArrays;
extern PFNGLDISABLEVERTEXARRAYATTRIBPROC            glDisableVertexArrayAttrib;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC             glEnableVertexArrayAttrib;
extern PFNGLVERTEXARRAYELEMENTBUFFERPROC            glVertexArrayElementBuffer;
extern PFNGLVERTEXARRAYVERTEXBUFFERPROC             glVertexArrayVertexBuffer;
extern PFNGLVERTEXARRAYATTRIBBINDINGPROC            glVertexArrayAttribBinding;
extern PFNGLVERTEXARRAYATTRIBFORMATPROC             glVertexArrayAttribFormat;

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
extern PFNGLSHADERBINARYPROC                        glShaderBinary;
extern PFNGLSPECIALIZESHADERPROC                    glSpecializeShader;
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

//GL_ARB_separate_shader_objects
// pipelines
extern PFNGLBINDPROGRAMPIPELINEPROC                 glBindProgramPipeline;
extern PFNGLCREATEPROGRAMPIPELINESPROC              glCreateProgramPipelines;
extern PFNGLDELETEPROGRAMPIPELINESPROC              glDeleteProgramPipelines;
extern PFNGLVALIDATEPROGRAMPIPELINEPROC             glValidateProgramPipeline;
extern PFNGLGETPROGRAMPIPELINEIVPROC                glGetProgramPipelineiv;
extern PFNGLGETPROGRAMPIPELINEINFOLOGPROC           glGetProgramPipelineInfoLog;
extern PFNGLUSEPROGRAMSTAGESPROC                    glUseProgramStages;
extern PFNGLACTIVESHADERPROGRAMPROC                 glActiveShaderProgram;
extern PFNGLPROGRAMUNIFORM1IPROC                    glProgramUniform1i;
extern PFNGLPROGRAMUNIFORM1IVPROC                   glProgramUniform1iv;
extern PFNGLPROGRAMUNIFORM1UIVPROC                  glProgramUniform1uiv;

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

// BEATO Begin: use direct state acess
extern PFNGLCREATERENDERBUFFERSPROC                 glCreateRenderbuffers;
extern PFNGLNAMEDRENDERBUFFERSTORAGEPROC            glNamedRenderbufferStorage;
extern PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC glNamedRenderbufferStorageMultisample;
extern PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC        glNamedFramebufferRenderbuffer;
extern PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC     glGetNamedRenderbufferParameteriv;
extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC         glCheckNamedFramebufferStatus;
extern PFNGLBLITNAMEDFRAMEBUFFERPROC                glBlitNamedFramebuffer;
// BEATO End

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

extern PFNGLMEMORYBARRIERPROC                       glMemoryBarrier;
extern PFNGLMEMORYBARRIERBYREGIONPROC               glMemoryBarrierByRegion;

//#include <SDL3/SDL_opengl.h>
//#include <GL/glext.h>

// TODO: Deprecate GL
extern void ( APIENTRYP glRasterPos2f )( GLfloat x, GLfloat y );
extern void ( APIENTRYP glDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void ( APIENTRYP glPushAttrib )(GLbitfield mask);
extern void ( APIENTRYP glPopAttrib )(void);
extern void ( APIENTRYP glArrayElement )(GLint i);

/// @brief OpenGL is a stete machine, that driver has his own command buffer, 
// and need SO ON... 
class glContextState
{
public:
    // state structures
    struct polygonMode_t
    {
        GLenum  face;
        GLenum  mode;
    };

    struct depthFuncState_t
    {
        GLboolean   depthTest = GL_FALSE;
        GLboolean   depthmask = GL_FALSE;
        GLenum      depthFunc = GL_ALWAYS;
    };

    struct stencilState_t
    {
        GLboolean   enable = GL_NONE;
        GLenum      face = GL_KEEP;
        GLenum      pass = GL_KEEP;
        GLenum      fail = GL_KEEP;
        GLenum      zfail = GL_KEEP;
    };

    struct blendingState_t
    {
        GLboolean   enable = GL_NONE;
        GLenum      srcFactor = GL_ONE;
        GLenum      srcFactorAlpha = GL_ONE;
		GLenum      dstFactor = GL_ZERO;
        GLenum      dstFactorAlpha = GL_ZERO;
        GLenum      blendOp = GL_FUNC_ADD;
    };

    struct cullFaceState_t
    {
        GLboolean   enable = true;
        GLenum      face = true;
    };

    struct programState_t
    {
        GLuint program;
        GLuint pipeline;
    };

    struct state_t
    {
        GLuint              vertexArray;
        GLuint              frameBuffer;
        programState_t      program;
        polygonMode_t       polygonMode;
        cullFaceState_t     cullFaceState;
        stencilState_t      stencilState;
        blendingState_t     blendingState;
        depthFuncState_t    depthFuncState;
    };

    glContextState( void );
    ~glContextState( void );

    // DG: R_GetModeListForDisplay is called before GLimp_Init(), but SDL needs SDL_Init() first.
    // So add PreInit for platforms that need it, others can just stub it.
    void PreInit( void );

    // If the desired mode can't be set satisfactorily, false will be returned.
    // If succesful, sets glConfig.nativeScreenWidth, glConfig.nativeScreenHeight, and glConfig.pixelAspect
    // The renderer will then reset the glimpParms to "safe mode" of 640x480
    // fullscreen and try again.  If that also fails, the error will be fatal.
    bool Init( const bool in_stereo, const uint8_t in_multiSamples );

    // will set up gl up with the new parms
    bool SetScreenParms( const bool in_stereo, const uint8_t in_multiSamples );

    // Destroys the rendering context, closes the window, resets the resolution,
    // and resets the gamma ramps.
    void Shutdown( void );

    // Sets the hardware gamma ramps for gamma and brightness adjustment.
    // These are now taken as 16 bit values, so we can take full advantage
    // of dacs with >8 bits of precision
    void    SetGamma( uint16_t red[256], uint16_t green[256], uint16_t blue[256] );

    void    EnableLogging( bool enable );

    void    SwapBuffers( void );


    /// @brief Flush the GPU command buffer
    void    Flush( void );

    /// @brief wait for the GPU to have executed all commands
    void    Finish( void );

    /// @brief 
    /// @param in_frameBuffer 
    void    BindFrameBuffer( const GLuint in_frameBuffer );

    /// @brief
    /// 
    void    BindVertexArray( const GLuint in_vertexArray );

    ///
    ///
    ///
    void    BindProgramPipeline( const GLuint in_programPipeline );

    /// @brief Enable/Disable face cull, and set face 
    /// @param in_face 
    void    FaceCull( const GLboolean in_enable, const GLenum in_face );

    ///
    void    DepthTest( const GLboolean in_enable, const GLenum in_func );

    ///
    void    Blending( const GLboolean in_enable, const GLenum in_SRCFactor, const GLenum in_SRCAlphaFactor, const GLenum in_DSTFactor, const GLenum in_DSTAlphaFactor, const GLenum in_blendOp );

    ///
    void    StencilTest( const GLboolean in_enable, const GLenum in_face,const GLenum in_pass,const GLenum in_fail,const GLenum in_Zfail );

    ///
    void    PolygonMode( const GLenum in_face, const GLenum in_mode );

private:
    state_t         m_state;
    SDL_Window*     m_window = nullptr;
    SDL_GLContext   m_context = nullptr;
};

#include "glProgram.hpp"
#include "glBuffer.hpp"
#include "glTexture.hpp"
#include "glShaderStorage.hpp"
#include "glPipeline.hpp"
#include "glFrameBuffer.hpp"
#include "glSwapchain.hpp"
#include "glCommandBuffer.hpp"

#endif //!__OPENGL_H__