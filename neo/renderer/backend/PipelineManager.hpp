
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
    static crPipelineManager*  Get( void );

    crPipelineManager( void );
    ~crPipelineManager( void );


    vkPipeline* GetPipeline( const pipeline_t in_ID );

private:

};

#endif //__PIPELINE_MANAGER_HPP__