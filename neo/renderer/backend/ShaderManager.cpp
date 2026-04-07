
///
#include "idlib/precompiled.h"
#include "renderer/renderer_common.h"
#include "ShaderManager.hpp"

// BUILT IN SHADER TABLE
static struct builtinshaders_t
{
    vkProgram::type_t type;
    const char* source;
} shaders[SHADER_FRAG_INTERATION + 1]
{
    { vkProgram::PROG_VERTEX,   "vertex_color_vs.spv" },          // SHADER_VERT_COLOR
    { vkProgram::PROG_VERTEX,   "vertex_skinned_vs.spv" },        // SHADER_VERT_COLOR_SKINED
    { vkProgram::PROG_FRAGMENT, "vertex_fs.spv" },                // SHADER_FRAG_COLOR,
    { vkProgram::PROG_VERTEX,   "textured_vs.spv" },              // SHADER_VERT_TEXTURED
    { vkProgram::PROG_VERTEX,   "textured_skinned_vs.spv" },      // SHADER_VERT_TEXTURED_SKINED
    { vkProgram::PROG_FRAGMENT, "textured_fs.spv" },              // SHADER_FRAG_TEXTURED
    { vkProgram::PROG_VERTEX,   "depth_vs.spv" },                 // SHADER_VERT_DEPTH
    { vkProgram::PROG_VERTEX,   "depth_skinned_vs.spv" },         // SHADER_VERT_DEPTH_SKINED
    { vkProgram::PROG_FRAGMENT, "depth_fs.spv" },                 // SHADER_FRAG_DEPTH
    { vkProgram::PROG_VERTEX,   "shadow_stencil_vs.spv" },        // SHADER_VERT_STENCIL_SHADOW
    { vkProgram::PROG_VERTEX,   "shadow_stencil_skinned_vs.spv" },// SHADER_VERT_STENCIL_SHADOW_SKINED
    { vkProgram::PROG_FRAGMENT, "shadow_stencil_fs.spv" },        // SHADER_FRAG_STENCIL_SHADOW
    { vkProgram::PROG_VERTEX,   "texgen_vs.spv" },                // SHADER_VERT_TEXTGEN
    { vkProgram::PROG_VERTEX,   "texgen_skinned_vs.spv" },        // SHADER_VERT_TEXTGEN_SKINED
    { vkProgram::PROG_FRAGMENT, "texgen_fs.spv" },                // SHADER_FRAG_TEXTGEN
    { vkProgram::PROG_VERTEX,   "interation_vs.spv" },            // SHADER_VERT_INTERATION
    { vkProgram::PROG_VERTEX,   "interation_skinned_vs.spv" },    // SHADER_VERT_INTERATION_SKINED
    { vkProgram::PROG_FRAGMENT, "interation_fs.spv" },            // SHADER_FRAG_INTERATION
};

crShaderManager::crShaderManager( void )
{
}

crShaderManager::~crShaderManager( void )
{
}

void crShaderManager::StartUp(void)
{
    for ( uint32_t i = 0; i < ( SHADER_FRAG_INTERATION + 1 ); i++)
    {
        // load builtin shaders
        if( !LoadShader( i, shaders[i].source, shaders[i].type ) )
            continue;
    }   
}

void crShaderManager::ShutDown(void)
{
    for ( uint32_t i = 0; i < MAX_SHADER_COUNT; i++)
    {
        /// ingnore if not created
        if ( m_shaders[i] == nullptr )
            continue;
    
        // release shader
        m_shaders[i]->Destroy();
    }
}

vkProgram *crShaderManager::GetShader(const uint32_t in_ID)
{
    /// todo: check shader and validade program ID 
    return m_shaders[in_ID];
}

bool crShaderManager::LoadShader(const uint32_t in_ID, const idStr in_sourcePath, const vkProgram::type_t in_type )
{
    size_t buffLen = 0;
    ID_TIME_T timeStamp = 0;
    idStr	fullPath = idStr( "renderprogs/vk/" );

    // get full source path 
    fullPath += in_sourcePath;

    

    return true;
}
