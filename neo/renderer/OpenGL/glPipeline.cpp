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

#include "precompiled.h"
#include "renderer_common.h"
#include "glPipeline.hpp"

glPipeline::glPipeline( void ) : crPipeline()
{
}

glPipeline::~glPipeline( void )
{
}

bool glPipeline::Create(  const PipelineInfo_t in_pipelineInfo )
{
    m_pipelineConfiguration = in_pipelineInfo;

    switch (m_pipelineConfiguration.faceCull)
    {
        case FC_BACK: // back face cull
            m_cullFace = GL_TRUE;
            m_cullFaceMode = GL_BACK;
            break;
        case FC_FRONT: // front face cull
            m_cullFace = GL_TRUE;
            m_cullFaceMode = GL_FRONT;
            break;
        case FC_TWO_FACES:
            m_cullFace = GL_FALSE;
            break;
    };

    switch (m_pipelineConfiguration.polygonMode)
    {
        case PM_POINT:
            m_polygonMode = GL_POINT;
            break;
        case PM_LINE:
            m_polygonMode = GL_LINE;
            break;
        case PM_FILL:
            m_polygonMode = GL_FILL;
            break;
    };

    switch ( m_pipelineConfiguration.polygonModeFace)
    {
        case FC_BACK:
            m_polygonFace = GL_BACK;
            break;
        case FC_FRONT:
            m_polygonFace = GL_FRONT;
            break;
        case FC_TWO_FACES:
            m_polygonFace = GL_FRONT_AND_BACK;
            break;
    };

    switch ( m_pipelineConfiguration.depthFunc )
    {
        case DF_NONE:
            m_depthTest = GL_FALSE;
            break;
        case DF_ALWAYS:
            m_depthTest = GL_TRUE;
            m_depthFunc = GL_ALWAYS;
            break;
        case DF_LESS:
            m_depthTest = GL_TRUE;
            m_depthFunc = GL_LEQUAL;
            break;
        case DF_GREATER:
            m_depthTest = GL_TRUE;
            m_depthFunc = GL_GEQUAL;
            break;
        case DF_EQUAL:
            m_depthTest = GL_TRUE;
            m_depthFunc = GL_EQUAL;
            break;
    };

    // Only actually update GL's blend func if blending is enabled.
    if( !( m_pipelineConfiguration.blendSource == BLEND_SRC_ONE && m_pipelineConfiguration.blendDestination == BLEND_DST_ZERO ) )
    {
        switch( m_pipelineConfiguration.blendSource )
        {
        case BLEND_SRC_ONE:
            m_blendSRCFactor = GL_ONE;
            m_blendSRCAlphaFactor = GL_ONE;
            break;

        case BLEND_SRC_ZERO:
            m_blendSRCFactor = GL_ZERO;
            m_blendSRCAlphaFactor = GL_ZERO;
            break;

        case BLEND_SRC_DST_COLOR:
            m_blendSRCFactor = GL_DST_COLOR;
            m_blendSRCAlphaFactor = GL_DST_COLOR;
            break;

        case BLEND_SRC_ONE_MINUS_DST_COLOR:
            m_blendSRCFactor = GL_ONE_MINUS_DST_COLOR;
            m_blendSRCAlphaFactor = GL_ONE_MINUS_DST_COLOR;
            break;

        case BLEND_SRC_SRC_ALPHA:
            m_blendSRCFactor = GL_SRC_ALPHA;
            m_blendSRCAlphaFactor = GL_SRC_ALPHA;
            break;

        case BLEND_SRC_ONE_MINUS_SRC_ALPHA:
            m_blendSRCFactor = GL_ONE_MINUS_SRC_ALPHA;
            m_blendSRCAlphaFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;

        case BLEND_SRC_DST_ALPHA:
            m_blendSRCFactor = GL_DST_ALPHA;
            m_blendSRCAlphaFactor = GL_DST_ALPHA;
            break;

        case BLEND_SRC_ONE_MINUS_DST_ALPHA:
            m_blendSRCFactor = GL_ONE_MINUS_DST_ALPHA;
            m_blendSRCAlphaFactor = GL_ONE_MINUS_DST_ALPHA;
            break;
        };

        switch( m_pipelineConfiguration.blendDestination )
        {
        case BLEND_DST_ZERO:
            m_blendDSTFactor = GL_ZERO;
            m_blendDSTAlphaFactor = GL_ZERO;
            break;

        case BLEND_DST_ONE:
            m_blendDSTFactor = GL_ONE;
            m_blendDSTAlphaFactor = GL_ONE;
            break;

        case BLEND_DST_SRC_COLOR:
            m_blendDSTFactor = GL_SRC_COLOR;
            m_blendDSTAlphaFactor = GL_SRC_COLOR;
            break;

        case BLEND_DST_ONE_MINUS_SRC_COLOR:
            m_blendDSTFactor = GL_ONE_MINUS_SRC_COLOR;
            m_blendDSTAlphaFactor = GL_ONE_MINUS_SRC_COLOR;
            break;

        case BLEND_DST_SRC_ALPHA:
            m_blendDSTFactor = GL_SRC_ALPHA;
            m_blendDSTAlphaFactor = GL_SRC_ALPHA;
            break;

        case BLEND_DST_ONE_MINUS_SRC_ALPHA:
            m_blendDSTFactor = GL_ONE_MINUS_SRC_ALPHA;
            m_blendDSTAlphaFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;

        case BLEND_DST_DST_ALPHA:
            m_blendDSTFactor = GL_DST_ALPHA;
            m_blendDSTAlphaFactor = GL_DST_ALPHA;
            break;

        case BLEND_DST_ONE_MINUS_DST_ALPHA:
            m_blendDSTFactor = GL_ONE_MINUS_DST_ALPHA;
            m_blendDSTAlphaFactor = GL_ONE_MINUS_DST_ALPHA;
            break;
        }

        switch( m_pipelineConfiguration.blendOperation )
        {
            case BLEND_OP_ADD:
                m_blendOp = GL_FUNC_ADD;
                break;
            case BLEND_OP_SUB:
                m_blendOp = GL_FUNC_SUBTRACT;
                break;
            case BLEND_OP_MIN:
                m_blendOp = GL_MIN;
                break;
            case BLEND_OP_MAX:
                m_blendOp = GL_MAX;
                break;
        };
    }
    else
    {
        m_blendSRCFactor = GL_ONE;
        m_blendSRCAlphaFactor = GL_ONE; 
        m_blendDSTFactor = GL_ZERO;
        m_blendDSTAlphaFactor = GL_ZERO;
        m_blendEnable = GL_FALSE;
    }

	glColorMask( ( in_pipelineInfo.colorMask & CM_RED_MASK ) ? GL_FALSE : GL_TRUE,
                 ( in_pipelineInfo.colorMask & CM_GREEN_MASK ) ? GL_FALSE : GL_TRUE,
                 ( in_pipelineInfo.colorMask & CM_BLUE_MASK ) ? GL_FALSE : GL_TRUE,
                 ( in_pipelineInfo.colorMask & CM_ALPHA_MASK ) ? GL_FALSE : GL_TRUE );

    switch ( m_pipelineConfiguration.stencilFace )
    {
        case FC_BACK:
            m_stencilFace = GL_BACK;
            break;
        case FC_FRONT:
            m_stencilFace = GL_FRONT;
            break;
        case FC_TWO_FACES:
            m_stencilFace = GL_FRONT_AND_BACK;
            break;
    }   

    switch( m_pipelineConfiguration.stencilPass )
    {
        case STENCIL_OP_KEEP:
            m_stencilPass = GL_KEEP;
            break;
        case STENCIL_OP_ZERO:
            m_stencilPass = GL_ZERO;
            break;
        case STENCIL_OP_REPLACE:
            m_stencilPass = GL_REPLACE;
            break;
        case STENCIL_OP_INCR:
            m_stencilPass = GL_INCR;
            break;
        case STENCIL_OP_DECR:
            m_stencilPass = GL_DECR;
            break;
        case STENCIL_OP_INVERT:
            m_stencilPass = GL_INVERT;
            break;
        case STENCIL_OP_INCR_WRAP:
            m_stencilPass = GL_INCR_WRAP;
            break;
        case STENCIL_OP_DECR_WRAP:
            m_stencilPass = GL_DECR_WRAP;
            break;
    };

    switch( m_pipelineConfiguration.stencilFail )
    {
       case STENCIL_OP_KEEP:
            m_stencilFail = GL_KEEP;
            break;
        case STENCIL_OP_ZERO:
            m_stencilFail = GL_ZERO;
            break;
        case STENCIL_OP_REPLACE:
            m_stencilFail = GL_REPLACE;
            break;
        case STENCIL_OP_INCR:
            m_stencilFail = GL_INCR;
            break;
        case STENCIL_OP_DECR:
            m_stencilFail = GL_DECR;
            break;
        case STENCIL_OP_INVERT:
            m_stencilFail = GL_INVERT;
            break;
        case STENCIL_OP_INCR_WRAP:
            m_stencilFail = GL_INCR_WRAP;
            break;
        case STENCIL_OP_DECR_WRAP:
            m_stencilFail = GL_DECR_WRAP;
            break;
    };

    switch( m_pipelineConfiguration.stencilZFail )
    {
        case STENCIL_OP_KEEP:
            m_stencilZfail = GL_KEEP;
            break;
        case STENCIL_OP_ZERO:
            m_stencilZfail = GL_ZERO;
            break;
        case STENCIL_OP_REPLACE:
            m_stencilZfail = GL_REPLACE;
            break;
        case STENCIL_OP_INCR:
            m_stencilZfail = GL_INCR;
            break;
        case STENCIL_OP_DECR:
            m_stencilZfail = GL_DECR;
            break;
        case STENCIL_OP_INVERT:
            m_stencilZfail = GL_INVERT;
            break;
        case STENCIL_OP_INCR_WRAP:
            m_stencilZfail = GL_INCR_WRAP;
            break;
        case STENCIL_OP_DECR_WRAP:
            m_stencilZfail = GL_DECR_WRAP;
            break;
    };

    

    switch( m_pipelineConfiguration.alphaFunc )
    {
        // DONE VIA SHADER
    };

    glCreateVertexArrays( 1, &m_vertexArray );
    glCreateProgramPipelines( 1, &m_programPipeline );

    // Todo: validate pipeline and vertex array

    return true;
}

void glPipeline::Destroy(void)
{
    if ( m_programPipeline )
    {
        glDeleteProgramPipelines( 1, &m_programPipeline );
        m_programPipeline = 0;
    }
    
    if( m_vertexArray )
    {
        glDeleteVertexArrays( 1, &m_vertexArray );
        m_vertexArray = 0;
    }

}
