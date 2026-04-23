
#include "precompiled.h"
#include "Vulkan/Core.hpp"
#include "PipelineManager.hpp"

crPipelineManager *crPipelineManager::Get(void)
{
    static crPipelineManager gPipelineManager = crPipelineManager();
    return &gPipelineManager;
}

crPipelineManager::crPipelineManager(void)
{
}

crPipelineManager::~crPipelineManager( void )
{
}

struct PipelineKey 
{
    uint64_t shaders; // vsID em 32 bits altos, fsID em 32 bits baixos
    uint64_t flags;   // Suas 64 flags de configuração

    PipelineKey( const uint32_t in_vertexShader, const uint32_t in_fragmentShader, const uint64_t in_flags )
    {
        shaders = ( static_cast<uint64_t>( in_vertexShader ) << 32 ) | in_fragmentShader;
        flags = in_flags;
    }

    bool operator==(const PipelineKey& other) const 
    {
        return shaders == other.shaders && flags == other.flags;
    }
};

inline void hash_combine( size_t& seed, uint64_t v ) 
{
    seed ^= std::hash<uint64_t>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t generateKey(uint32_t vs, uint32_t fs, uint64_t flags) 
{
    size_t seed = 0;
    uint64_t shaders = (static_cast<uint64_t>(vs) << 32) | fs;
    
    hash_combine( seed, shaders);
    hash_combine( seed, flags);
    
    return seed;
}

crPipelinep crPipelineManager::GetPipeline( const uint32_t in_vertexShader, const uint32_t in_fragmentShader, const uint64_t in_flags )
{
    int index = 0;
    int key = 0;
    crPipelinep pipeline = nullptr;

#if 1
    /// try find the same pipeline in the list
    for ( uint32_t i = 0; i < m_pipelinesList.Num(); i++)
    {
        auto pipe = m_pipelinesList[i];
        
        /// Ignore if vertex shader don't match
        if( pipe->VertexProgramID() != in_vertexShader )
            continue;

        /// Ignore if fragment shader don't match
        if( pipe->FragmentProgramID() != in_fragmentShader )
            continue;

        /// well, shaders match and flags match
        if( pipe->Flags() == in_flags )
        {
            pipeline = pipe;
            break; 
        }
    }
#else

#endif 

    if( pipeline == nullptr )
    {
        /// look for a reference pipeline
        crPipelinep reference = nullptr;

        for ( uint32_t i = 0; i < m_pipelinesList.Num(); i++)
        {
            auto vert = m_pipelinesList[i]->VertexProgramID();
            auto frag = m_pipelinesList[i]->FragmentProgramID();

            /// if we found another pipeline that use same prograns,
            /// re use that pipeline to create the new one 
            if ( vert == in_vertexShader && frag == in_fragmentShader )
            {
                index = i;
                reference = m_pipelinesList[i];
                break;
            }
        }
        
        /// get vertex program
        crProgramp vertex = this->GetProgram( in_vertexShader );
        if( vertex == nullptr )
        {
            idLib::Error( "Vertex program Index %u not found!\n", in_vertexShader );
            return nullptr;
        }
        
        /// get fragment program
        crProgramp fragment = this->GetProgram( in_fragmentShader );
        if( vertex == nullptr )
        {
            idLib::Error( "Fragment program Index %u not found!\n", in_fragmentShader );
            return nullptr;
        }

        pipeline = new crPipeline();
        if( !pipeline->Create( in_flags, vertex, fragment, reference ) )
        {
            delete pipeline;
            return nullptr;
        }

        /// append new pipeline to the list
        index = m_pipelinesList.Append( pipeline );
        // m_pipelinesIndex.Add( key, index );
    }

    return pipeline;
}

crProgramp crPipelineManager::GetProgram( const uint32_t in_programID )
{
    if( in_programID >= m_programs.Num() )
    {
        idLib::Warning( "Invalid program ID %u\n", in_programID );
        return nullptr;
    }

    crProgramp prog = nullptr;
    prog = m_programs[in_programID];
    return prog;
}

crSamplerp crPipelineManager::GetSampler(const crSampler::filter_t in_filter, const crSampler::wrapping_t in_repeat)
{
    crSamplerp sampler;

    /// try find a match sampler
    for ( uint32_t i = 0; i < m_samplers.Num(); i++)
    {
        sampler = m_samplers[i];
        if( sampler->Filtering() == in_filter && sampler->WrapS() == in_repeat )
            break;
    }
    
    /// no match found, create a new sampler
    if( sampler == nullptr )
    {
        sampler = new crSampler();
        if( !sampler->Create( in_filter, in_repeat, in_repeat, in_repeat ) )
            idLib::FatalError( "Failed to create sampler!!!\n" );
            
        m_samplers.Append( sampler );
    }

    return sampler;
}

uint32_t crPipelineManager::FindShader(const idStr &in_program, const crProgram::type_t in_type)
{
    ID_TIME_T timeStamp = 0;
    idStr path = idStr( "renderprogs/spirv/" );
    path += in_program;
	
    switch ( in_type )
    {
        case crProgram::PROG_VERTEX:
            path += "_vert.spv";
            break;
        case crProgram::PROG_GEOMETRY:
            path += "_geom.spv";
            break;
        case crProgram::PROG_FRAGMENT:
            path += "_frag.spv";
            break;
        case crProgram::PROG_COMPUTE:
            path += "_comp.spv";
            break;
    };
    
    //Read shade file     
    uint32_t *spirVbuff = nullptr;
	auto buffLen = fileSystem->ReadFile( path.c_str(), reinterpret_cast<void**>( &spirVbuff ), &timeStamp );
    if ( buffLen == 0 )
    {
        common->Warning( "crShaderManager::LoadShader::Error( can't open shader binary file %s)\n", path.c_str() );
        return false;
    }
    
    /// Try Create shader    
    crProgramp newProgram = new( TAG_RENDER ) crProgram();
    if( !newProgram->Create( in_type, spirVbuff, buffLen ) )
    {
        delete newProgram;
        return -1;
    }

    int index = m_programs.Append( newProgram );
    int hash = path.FileNameHash();
    m_programIndex.Add( hash, index );
    return index;
}
