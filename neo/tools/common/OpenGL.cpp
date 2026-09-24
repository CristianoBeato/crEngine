
#include "precompiled.h"
#include "OpenGL.hpp"

#include <SDL3/SDL_video.h>

PFNGLFINISHPROC                                 gl::Finish = nullptr;
PFNGLFLUSHPROC                                  gl::Flush = nullptr;

PFNGLDRAWARRAYSPROC                             gl::DrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC                           gl::DrawElements = nullptr;
PFNGLDRAWELEMENTSBASEVERTEXPROC                 gl::DrawElementsBaseVertex = nullptr;

/// 
PFNGLCREATEVERTEXARRAYSPROC                     gl::CreateVertexArrays = nullptr;
PFNGLDELETEVERTEXARRAYSPROC                     gl::DeleteVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC                        gl::BindVertexArray = nullptr;          

PFNGLENABLEVERTEXARRAYATTRIBPROC                gl::EnableVertexArrayAttrib = nullptr;
PFNGLDISABLEVERTEXARRAYATTRIBPROC               gl::DisableVertexArrayAttrib = nullptr;
PFNGLVERTEXARRAYATTRIBBINDINGPROC               gl::VertexArrayAttribBinding = nullptr;
PFNGLVERTEXARRAYATTRIBFORMATPROC                gl::VertexArrayAttribFormat = nullptr;
PFNGLVERTEXARRAYELEMENTBUFFERPROC               gl::VertexArrayElementBuffer = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERPROC                gl::VertexArrayVertexBuffer = nullptr; 
    
// GL_ARB_multi_bind
PFNGLVERTEXARRAYVERTEXBUFFERSPROC               gl::VertexArrayVertexBuffers = nullptr; 

    // shader 
PFNGLISSHADERPROC                               gl::IsShader = nullptr;
PFNGLCREATESHADERPROC                           gl::CreateShader = nullptr;
PFNGLDELETESHADERPROC                           gl::DeleteShader = nullptr;
PFNGLSHADERSOURCEPROC                           gl::ShaderSource = nullptr;
PFNGLSHADERBINARYPROC                           gl::ShaderBinary = nullptr;
PFNGLCOMPILESHADERPROC                          gl::CompileShader = nullptr;
PFNGLSPECIALIZESHADERPROC                       gl::SpecializeShader = nullptr;
PFNGLGETSHADERINFOLOGPROC                       gl::GetShaderInfoLog = nullptr;
PFNGLGETSHADERIVPROC                            gl::GetShaderiv = nullptr;

// program
PFNGLCREATEPROGRAMPROC                          gl::CreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC                          gl::DeleteProgram = nullptr;
PFNGLISPROGRAMPROC                              gl::IsProgram = nullptr;
PFNGLPROGRAMPARAMETERIPROC                      gl::ProgramParameteri = nullptr;
PFNGLATTACHSHADERPROC                           gl::AttachShader = nullptr;
PFNGLDETACHSHADERPROC                           gl::DetachShader = nullptr;
PFNGLLINKPROGRAMPROC                            gl::LinkProgram = nullptr;
PFNGLVALIDATEPROGRAMPROC                        gl::ValidateProgram = nullptr;
PFNGLGETPROGRAMIVPROC                           gl::GetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC                      gl::GetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC                             gl::UseProgram = nullptr;
PFNGLUNIFORM1IPROC                              gl::Uniform1i = nullptr;
PFNGLUNIFORM1IVPROC                             gl::Uniform1iv = nullptr;
PFNGLUNIFORM1UIVPROC                            gl::Uniform1uiv = nullptr;

// buffer
PFNGLISBUFFERPROC                               gl::IsBuffer = nullptr;
PFNGLBINDBUFFERPROC                             gl::BindBuffer = nullptr;
PFNGLBINDBUFFERBASEPROC                         gl::BindBufferBase = nullptr;
PFNGLBINDBUFFERRANGEPROC                        gl::BindBufferRange = nullptr;
PFNGLCREATEBUFFERSPROC                          gl::CreateBuffers = nullptr;
PFNGLDELETEBUFFERSPROC                          gl::DeleteBuffers = nullptr;
PFNGLNAMEDBUFFERSTORAGEPROC                     gl::NamedBufferStorage = nullptr;
PFNGLMAPNAMEDBUFFERRANGEPROC                    gl::MapNamedBufferRange = nullptr;
PFNGLUNMAPNAMEDBUFFERPROC                       gl::UnmapNamedBuffer = nullptr;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC            gl::FlushMappedNamedBufferRange = nullptr;
PFNGLNAMEDBUFFERSUBDATAPROC                     gl::NamedBufferSubData = nullptr;
PFNGLGETNAMEDBUFFERSUBDATAPROC                  gl::GetNamedBufferSubData = nullptr;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC                 gl::CopyNamedBufferSubData = nullptr;

PFNGLCREATESAMPLERSPROC                         gl::CreateSamplers = nullptr;
PFNGLDELETESAMPLERSPROC                         gl::DeleteSamplers = nullptr;
PFNGLBINDSAMPLERPROC                            gl::BindSampler = nullptr;
PFNGLBINDSAMPLERSPROC                           gl::BindSamplers = nullptr;
PFNGLISSAMPLERPROC                              gl::IsSampler = nullptr;
PFNGLSAMPLERPARAMETERIPROC                      gl::SamplerParameteri = nullptr;
PFNGLSAMPLERPARAMETERIVPROC                     gl::SamplerParameteriv = nullptr;
PFNGLSAMPLERPARAMETERFPROC                      gl::SamplerParameterf = nullptr;
PFNGLSAMPLERPARAMETERFVPROC                     gl::SamplerParameterfv = nullptr;
PFNGLSAMPLERPARAMETERIIVPROC                    gl::SamplerParameterIiv = nullptr;
PFNGLSAMPLERPARAMETERIUIVPROC                   gl::SamplerParameterIuiv = nullptr;
PFNGLGETSAMPLERPARAMETERIVPROC                  gl::GetSamplerParameteriv = nullptr;
PFNGLGETSAMPLERPARAMETERIIVPROC                 gl::GetSamplerParameterIiv = nullptr;
PFNGLGETSAMPLERPARAMETERFVPROC                  gl::GetSamplerParameterfv = nullptr;
PFNGLGETSAMPLERPARAMETERIUIVPROC                gl::GetSamplerParameterIuiv = nullptr;

template< typename __type___>
bool    GetFnProc( const char* name, __type___ proc )
{
    proc = reinterpret_cast<__type__>( SDL_GL_GetProcAddress( name ) );
    return proc != nullptr;
}

void gl::LoadFuntions(void)
{
    SDL_assert( GetFnProc( "glFinish", Finish ) );
    SDL_assert( GetFnProc( "glFlush", Flush ) );
}

GLboolean gl::SetState(const GLenum in_state, const GLboolean in_enable)
{
    GLboolean current = IsEnabled( in_state );
    if( !current && in_enable )
        Enable( in_state );
    else if( current && !in_enable )
        Disable( in_state );

    return current;
}

GLuint gl::CreateShaderStage(const GLenum in_stage, const char *in_source)
{   
    GLint success = GL_FALSE;

    /// Create shader handle
    GLuint shader = CreateShader( in_stage );

    /// Load shader source
    ShaderSource( shader, 1, &in_source, nullptr );

    /// Compile shader source
    CompileShader( shader );

    /// Validate shader compilation
    GetShaderiv( shader, GL_COMPILE_STATUS, &success );
    if( !success )
    {
        GLint len = 0;
        GetShaderiv( shader, GL_INFO_LOG_LENGTH, &len );
        char* log = static_cast<char*>( std::malloc( len ) );
        GetShaderInfoLog(shader, len, NULL, log );

        idLib::Error( "Create Shader Failed! (%s)\n", log );

        std::free( log );

        // Failed
        return 0;
    }

    return shader;
}

GLuint gl::CreateShaderProgram(const GLuint *in_shaders, const uint32_t in_count)
{
    uint32_t i = 0;
    GLint success = GL_FALSE;

    /// Create the program handle 
    GLuint program = CreateProgram();

    /// Attach shaders to shader program 
    for ( i = 0; i < in_count; i++)
    {
        AttachShader( program, in_shaders[i] );
    }
    
    LinkProgram( program );

    /// Release shaders
    for( i = 0; i < in_count; i++ )
    {
        DetachShader( program, in_shaders[i] );
    }

    /// Validate shader compilation
    GetProgramiv( program, GL_COMPILE_STATUS, &success );
    if( !success )
    {
        GLint len = 0;
        GetProgramiv( program, GL_INFO_LOG_LENGTH, &len );
        char* log = static_cast<char*>( std::malloc( len ) );
        GetProgramInfoLog( program, len, NULL, log );

        idLib::Error( "Create Program Failed! (%s)\n", log );

        std::free( log );

        // Failed
        return 0;
    }

    return program;
}
