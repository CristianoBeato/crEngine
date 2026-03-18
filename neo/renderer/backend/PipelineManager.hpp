
#ifndef __PIPELINE_MANAGER_HPP__
#define __PIPELINE_MANAGER_HPP__

class crPipelineManager
{
public:

    enum shader_type_e : uint8_t
    {
        ST_VERTEX,
        ST_FRAGMENT
    };

    static crPipelineManager*  Get( void );

    crPipelineManager( void );
    ~crPipelineManager( void );
    void    StartUp( void );
    void    Release( void );

    vkPipelinep GetPipeline( const uint32_t in_vertexShader, const uint32_t in_fragmentShader, const uint64_t in_flags );
    vkProgramp  GetProgram( const uint32_t in_programID );
    vkSamplerp  GetSampler( const vkSampler::filter_t in_filter, const vkSampler::wrapping_t in_repeat );
    uint32_t    FindShader( const idStr &in_program, const shader_type_e in_type );

private:
    //idHashIndex         m_pipelinesIndex;
    idList<vkPipelinep> m_pipelinesList;
    idList<vkSamplerp>  m_samplers;
    idList<vkProgramp>  m_programs;
};

#endif //__PIPELINE_MANAGER_HPP__