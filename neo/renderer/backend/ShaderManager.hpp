
#ifndef __SHADER_MANAGER_HPP__
#define __SHADER_MANAGER_HPP__

enum shaderid_e : int16_t
{
    SHADER_NONE = -1,
    
    /// @brief Draw coolor only geometry ( useful for debug )
    SHADER_VERT_COLOR,
    SHADER_VERT_COLOR_SKINNED,
    SHADER_FRAG_COLOR,

    /// @brief Draw single texture surface 
    SHADER_VERT_TEXTURED,
    SHADER_VERT_TEXTURED_SKINNED,
    SHADER_FRAG_TEXTURED,
    
    /// @brief Depth pass shader
    SHADER_VERT_DEPTH,
    SHADER_VERT_DEPTH_SKINNED,
    SHADER_FRAG_DEPTH,

    /// @brief Stencil shadow render 
    SHADER_VERT_STENCIL_SHADOW,
    SHADER_VERT_STENCIL_SHADOW_SKINNED,
    SHADER_FRAG_STENCIL_SHADOW,

    /// @brief Texgens
    SHADER_VERT_TEXTGEN,
    SHADER_VERT_TEXTGEN_SKINNED,
    SHADER_FRAG_TEXTGEN,

    /// @brief Vertex shader interation
    SHADER_VERT_INTERATION,
    SHADER_VERT_INTERATION_SKINNED,
    SHADER_FRAG_INTERATION,

    SHADER_CUSTOM_ID,  
    MAX_SHADER_COUNT = 128,
};

/// @brief load, store and manage shader programs
class crShaderManager
{
public:
    crShaderManager( void );
    ~crShaderManager( void );

    // initialize shaders
    void        StartUp( void );
    void        ShutDown( void );
    void        ReloadShaders( void );
    vkProgram*  GetShader( const uint32_t in_ID );

private:
    idStaticList<vkProgram*, MAX_SHADER_COUNT>   m_shaders;

    bool    LoadShader( const uint32_t in_ID, const idStr in_path, const vkProgram::type_t in_type );
};

#endif // __SHADER_MANAGER_HPP__