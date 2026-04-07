#version 460 core

/// vertex input
layout( location = 0 ) in vec3 attrb_position;

struct vertexDepthPrePassUniforms_t
{
    mat4    MVPmatrix;
};

// vertex shader storage buffer 
layout( std430, binding = 1 ) buffer depth_vbo
{
  vertexDepthPrePassUniforms_t vertUnifom[];
};

///
void main( void )
{
    /// draw command index
    int commandIndex = gl_DrawID;

    /// get the current Model View Projection matrix
    mat4 MVP = vertUnifom[commandIndex].MVPmatrix;

    /// calculate vertex position
    vec4 position = MVP * vec4( attrb_position, 1.0f );

    /// submit vertex position to gl pipeline raster
    gl_Position = position;
}