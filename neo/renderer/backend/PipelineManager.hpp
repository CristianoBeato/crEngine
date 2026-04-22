
#ifndef __PIPELINE_MANAGER_HPP__
#define __PIPELINE_MANAGER_HPP__

class crPipelineManager
{
public:
    static crPipelineManager*  Get( void );

    crPipelineManager( void );
    ~crPipelineManager( void );
    void    StartUp( void );
    void    Release( void );

    crPipelinep GetPipeline( const uint32_t in_vertexShader, const uint32_t in_fragmentShader, const uint64_t in_flags );
    crProgramp  GetProgram( const uint32_t in_programID );
    crSamplerp  GetSampler( const crSampler::filter_t in_filter, const crSampler::wrapping_t in_repeat );
    uint32_t    FindShader( const idStr &in_program, const crProgram::type_t in_type );

private:
    idHashIndex         m_programIndex;
    idList<crPipelinep> m_pipelinesList;
    idList<crSamplerp>  m_samplers;
    idList<crProgramp>  m_programs;
};

#endif //__PIPELINE_MANAGER_HPP__