
#ifndef __OPENGL_HPP__
#define __OPENGL_HPP__

#include <GL/glcorearb.h>

/// BEATO: For now just a foward notes to implement tools gui using 
/// OpenGL rendering whit Vulkan interop
// GL_EXT_memory_object
// GL_EXT_semaphore
// VK_KHR_external_memory
// VK_KHR_external_semaphore

class gl
{    
public:
    /// Global state configuration
    static PFNGLGETINTEGERVPROC                             GetIntegerv;
    static PFNGLGETINTEGER64VPROC                           GetInteger64v;
    static PFNGLISENABLEDPROC                               IsEnabled;
    static PFNGLDISABLEPROC                                 Disable;
    static PFNGLENABLEPROC                                  Enable;
    static PFNGLENABLEIPROC                                 Enablei;
    static PFNGLDISABLEIPROC                                Disablei;
    static PFNGLFINISHPROC                                  Finish;
    static PFNGLFLUSHPROC                                   Flush;

    static PFNGLDRAWARRAYSPROC                              DrawArrays;
    static PFNGLDRAWELEMENTSPROC                            DrawElements;
    static PFNGLDRAWELEMENTSBASEVERTEXPROC                  DrawElementsBaseVertex;

    /// @brief Vertex Array Objets
    static PFNGLCREATEVERTEXARRAYSPROC                      CreateVertexArrays;
    static PFNGLDELETEVERTEXARRAYSPROC                      DeleteVertexArrays;
    static PFNGLBINDVERTEXARRAYPROC                         BindVertexArray;
    static PFNGLENABLEVERTEXARRAYATTRIBPROC                 EnableVertexArrayAttrib;
    static PFNGLDISABLEVERTEXARRAYATTRIBPROC                DisableVertexArrayAttrib;
    static PFNGLVERTEXARRAYATTRIBBINDINGPROC                VertexArrayAttribBinding;
    static PFNGLVERTEXARRAYATTRIBFORMATPROC                 VertexArrayAttribFormat;
    static PFNGLVERTEXARRAYELEMENTBUFFERPROC                VertexArrayElementBuffer;
    static PFNGLVERTEXARRAYVERTEXBUFFERPROC                 VertexArrayVertexBuffer; 
    
    // GL_ARB_multi_bind
    static PFNGLVERTEXARRAYVERTEXBUFFERSPROC                VertexArrayVertexBuffers; 

    // shader 
    static PFNGLISSHADERPROC                                IsShader;
    static PFNGLCREATESHADERPROC                            CreateShader;
    static PFNGLDELETESHADERPROC                            DeleteShader;
    static PFNGLSHADERSOURCEPROC                            ShaderSource;
    static PFNGLSHADERBINARYPROC                            ShaderBinary;
    static PFNGLCOMPILESHADERPROC                           CompileShader;
    static PFNGLSPECIALIZESHADERPROC                        SpecializeShader;
    static PFNGLGETSHADERINFOLOGPROC                        GetShaderInfoLog;
    static PFNGLGETSHADERIVPROC                             GetShaderiv;

    // program
    static PFNGLCREATEPROGRAMPROC                           CreateProgram;
    static PFNGLDELETEPROGRAMPROC                           DeleteProgram;
    static PFNGLISPROGRAMPROC                               IsProgram;
    static PFNGLPROGRAMPARAMETERIPROC                       ProgramParameteri;
    static PFNGLATTACHSHADERPROC                            AttachShader;
    static PFNGLDETACHSHADERPROC                            DetachShader;
    static PFNGLLINKPROGRAMPROC                             LinkProgram;
    static PFNGLVALIDATEPROGRAMPROC                         ValidateProgram;
    static PFNGLGETPROGRAMIVPROC                            GetProgramiv;
    static PFNGLGETPROGRAMINFOLOGPROC                       GetProgramInfoLog;
    static PFNGLUSEPROGRAMPROC                              UseProgram;
    static PFNGLUNIFORM1IPROC                               Uniform1i;
    static PFNGLUNIFORM1IVPROC                              Uniform1iv;
    static PFNGLUNIFORM1UIVPROC                             Uniform1uiv;

    // buffer
    static PFNGLISBUFFERPROC                                IsBuffer;
    static PFNGLBINDBUFFERPROC                              BindBuffer;
    static PFNGLBINDBUFFERBASEPROC                          BindBufferBase;
    static PFNGLBINDBUFFERRANGEPROC                         BindBufferRange;
    static PFNGLCREATEBUFFERSPROC                           CreateBuffers;
    static PFNGLDELETEBUFFERSPROC                           DeleteBuffers;
    static PFNGLNAMEDBUFFERSTORAGEPROC                      NamedBufferStorage;
    static PFNGLMAPNAMEDBUFFERRANGEPROC                     MapNamedBufferRange;
    static PFNGLUNMAPNAMEDBUFFERPROC                        UnmapNamedBuffer;
    static PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC             FlushMappedNamedBufferRange;
    static PFNGLNAMEDBUFFERSUBDATAPROC                      NamedBufferSubData;
    static PFNGLGETNAMEDBUFFERSUBDATAPROC                   GetNamedBufferSubData;
    static PFNGLCOPYNAMEDBUFFERSUBDATAPROC                  CopyNamedBufferSubData;

    // sampler
    static PFNGLCREATESAMPLERSPROC                          CreateSamplers;
    static PFNGLDELETESAMPLERSPROC                          DeleteSamplers;
    static PFNGLBINDSAMPLERPROC                             BindSampler;
    static PFNGLBINDSAMPLERSPROC                            BindSamplers;
    static PFNGLISSAMPLERPROC                               IsSampler;
    static PFNGLSAMPLERPARAMETERIPROC                       SamplerParameteri;
    static PFNGLSAMPLERPARAMETERIVPROC                      SamplerParameteriv;
    static PFNGLSAMPLERPARAMETERFPROC                       SamplerParameterf;
    static PFNGLSAMPLERPARAMETERFVPROC                      SamplerParameterfv;
    static PFNGLSAMPLERPARAMETERIIVPROC                     SamplerParameterIiv;
    static PFNGLSAMPLERPARAMETERIUIVPROC                    SamplerParameterIuiv;
    static PFNGLGETSAMPLERPARAMETERIVPROC                   GetSamplerParameteriv;
    static PFNGLGETSAMPLERPARAMETERIIVPROC                  GetSamplerParameterIiv;
    static PFNGLGETSAMPLERPARAMETERFVPROC                   GetSamplerParameterfv;
    static PFNGLGETSAMPLERPARAMETERIUIVPROC                 GetSamplerParameterIuiv;

    static void    LoadFuntions( void );

    /// @brief If no enable/disable, enable/disble the state
    /// @param in_state the openGL context state to be set 
    /// @param in_enable Enable the state or Disable
    /// @return The previous state 
    static GLboolean       SetState( const GLenum in_state, const GLboolean in_enable );

    static GLuint           CreateShaderStage( const GLenum in_stage, const char* in_source );
    static GLuint           CreateShaderProgram( const GLuint* in_shaders, const uint32_t in_count );

private:
    gl( void ) = delete;
    ~gl( void ) = delete;
};



#endif //!__OPENGL_HPP__