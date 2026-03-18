
#ifndef __PIPELINE_MANAGER_HPP__
#define __PIPELINE_MANAGER_HPP__

enum pipeline_t : uint16_t
{
    PIPELINE_NONE = 0,
    PIPELINE_COLOR,             /// vertex color only shaders
    PIPELINE_TEXTURED,          /// 
    PIPELINE_TEXTURED_COLOR,
};


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


    vkPipeline* GetPipeline( const pipeline_t in_ID );
    uint32_t    FindShader( const idStr &in_program, const shader_type_e in_type );

private:

};

#endif //__PIPELINE_MANAGER_HPP__