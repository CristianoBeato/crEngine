#version 460 core

/// fragment output to render buffer
layout( location = 0 ) out vec4 fragment;

///
void main( void )
{
    /// It simply renders the mesh in black because we are only filling the depth buffer. 
    fragment = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 );
}