#version 460 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aTex;
layout (location = 2) in vec4 aCol;
layout (location = 3) in vec4 aNor;

layout(std140, binding = 0) uniform Matrices
{
   mat4 uViewMode;
   mat4 uProjection;
   mat4 uTexture;
};

layout( location = 0 ) out VertexData
{
   vec4 color;
   vec4 normal;
   vec4 texCoord;
} vs_out;

void main()
{
   mat4 mvp = uProjection * uViewMode;
   gl_Position = mvp * aPos;
   vs_out.texCoord = uTexture * aTex;
   vs_out.color = aCol * aTex;
   vs_out.normal = aNor;
}