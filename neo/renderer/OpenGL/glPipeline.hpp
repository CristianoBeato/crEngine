/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

This file is part of the crEngine GPL Source Code ("crEngine Source Code").

crEngine Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

crEngine Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef __GL_PIPELINE_HPP__
#define __GL_PIPELINE_HPP__

class glPipeline : public crPipeline
{

public:
    glPipeline( void );
    ~glPipeline( void );

    virtual void    Create( const crPipeline::PipelineInfo_t in_pipelineInfo );
    virtual void    Destroy( void );

protected:
    friend class glCommandBuffer;
    GLboolean       m_cullFace;
    GLboolean       m_depthTest;
    GLboolean       m_blendEnable;       
    GLboolean       m_stencilEnable;   
    GLenum          m_cullFaceMode;
    GLenum          m_polygonMode;
    GLenum          m_polygonFace;
    GLenum          m_depthFunc;
    GLenum          m_blendSRCFactor;
    GLenum          m_blendSRCAlphaFactor;
	GLenum          m_blendDSTFactor;
	GLenum          m_blendDSTAlphaFactor;
    GLenum          m_blendOp;
    GLenum          m_stencilFace;
    GLenum          m_stencilPass;
    GLenum          m_stencilFail;
    GLenum          m_stencilZfail;
    GLuint          m_programPipeline;
    GLuint          m_vertexArray;
};

#endif //!__GL_PIPELINE_HPP__