#version 460 core

layout (location = 0) out vec4 fragment;

layout( binding = 0 ) uniform sampler2D uTexture;

layout( location = 0 ) in VertexData
{
   vec4 color;
   vec4 normal;
   vec4 texCoord;
} vs_in;

void main()
{
    fragment = texture( uTexture, vs_in.texCoord ) * fs_in.color;
}