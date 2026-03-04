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

#include "Pipeline.hpp"
#include "Core.hpp"

vkPipeline::vkPipeline( void )
{
}

vkPipeline::~vkPipeline( void )
{
}

constexpr uint32_t NUM_DYNAMIC_STATE = 6;
constexpr uint32_t NUM_ATTRIBS_DESCR = 7;

bool vkPipeline::Create( const uint64_t in_flags )
{
    m_flags = in_flags;
    VkDynamicState dynamicStates[NUM_DYNAMIC_STATE] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,                  //
        VK_DYNAMIC_STATE_SCISSOR,                   //
        VK_DYNAMIC_STATE_CULL_MODE,                 // set via crBackend::Cull
        VK_DYNAMIC_STATE_DEPTH_BIAS,                // set via crBackend::PolygonOffset
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,              // set via crBackend::DepthBoundsTest
        VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE   // set via crBackend::DepthBoundsTest
    };

    /// Dynamic state
    /// While most of the pipeline state needs to be baked into the pipeline state, a
    /// limited amount of the state can actually be changed without recreating the
    /// pipeline at draw time.
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = NUM_DYNAMIC_STATE;
    dynamicState.pDynamicStates = dynamicStates;

    ///
    /// Bindings
    /// Describe vertex shader attributes buffer bindings
    VkVertexInputBindingDescription vertexInputBindingDescription[]
    {
        { VERTEX_BINDING, sizeof( idDrawVert ), VK_VERTEX_INPUT_RATE_VERTEX },
    };

    ///
    /// Attribute Description
    /// Describe vertex components
    VkVertexInputAttributeDescription vertexInputAttributeDescription[NUM_ATTRIBS_DESCR]
    {
        { VERTEX_ATTRIBUTE_POS, VERTEX_BINDING, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        { VERTEX_ATTRIBUTE_TEX, VERTEX_BINDING, VK_FORMAT_R16G16_SFLOAT, offsetof( idDrawVert, st ) },
        { VERTEX_ATTRIBUTE_NOR, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, offsetof( idDrawVert, normal ) },
        { VERTEX_ATTRIBUTE_TAN, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, offsetof( idDrawVert, tangent ) },
        { VERTEX_ATTRIBUTE_JOI, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, offsetof( idDrawVert, color ) },
        { VERTEX_ATTRIBUTE_WEI, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, offsetof( idDrawVert, color2 ) }
    };

    /// 
    /// Vertex input
    /// describes the format of the vertex data that will be passed to the vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
    vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;;
    vertexInputStateCI.pNext = nullptr;
    vertexInputStateCI.flags = 0;
    vertexInputStateCI.vertexBindingDescriptionCount = 1;
    vertexInputStateCI.pVertexBindingDescriptions = vertexInputBindingDescription;
    vertexInputStateCI.vertexAttributeDescriptionCount = NUM_ATTRIBS_DESCR;
    vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributeDescription;

    ///
    /// Input Assembly
    /// Describes what kind of geometry will be drawn from the vertices and if primitive restart
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;;
    inputAssemblyState.pNext = nullptr;
    inputAssemblyState.flags = 0;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    ///
    ///
    ///
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr; // Dynamically defined 
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr; // Dynamically defined 

    ///
    ///
    ///
    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.pNext = nullptr;
    rasterizationState.flags = 0;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = ( m_flags & PLS_POLYMODE_LINE ) == PLS_POLYMODE_LINE ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 0.0f;
    rasterizationState.lineWidth = 1.0f;

    ///
    ///
    ///
    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pNext = VK_FALSE;
    multisampleState.flags = 0;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; 
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 1.0f;
    multisampleState.pSampleMask = nullptr;
    multisampleState.alphaToCoverageEnable = VK_FALSE;
    multisampleState.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.pNext = nullptr;
    depthStencilState.flags = 0;
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_NEVER;
    depthStencilState.depthBoundsTestEnable = VK_FALSE; // enable in runtime
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    if ( m_flags & PLS_DEPTHFUNC_BITS ) 
    {
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        switch ( m_flags & PLS_DEPTHFUNC_BITS )
        {
        case PLS_DEPTHFUNC_LESS:    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS; break;
        case PLS_DEPTHFUNC_ALWAYS:  depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS; break;
        case PLS_DEPTHFUNC_GREATER: depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER; break;
        case PLS_DEPTHFUNC_EQUAL:   depthStencilState.depthCompareOp = VK_COMPARE_OP_EQUAL; break;   
        }
    }
    
    if ( m_flags & ( PLS_STENCIL_FUNC_BITS | PLS_STENCIL_OP_BITS ) )
    {
        
        ///
        ///
        VkStencilOpState stencilOpState{};
        stencilOpState.failOp = VK_STENCIL_OP_KEEP;
        stencilOpState.passOp = VK_STENCIL_OP_KEEP;
        stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP;
        stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS;
        stencilOpState.compareMask = 0;
        stencilOpState.writeMask = 0;
        stencilOpState.reference = 0;
        
        if ( m_flags & ( PLS_STENCIL_FUNC_BITS | PLS_STENCIL_FUNC_REF_BITS | PLS_STENCIL_FUNC_MASK_BITS ) ) 
        {
		    stencilOpState.reference = uint32_t( ( m_flags & PLS_STENCIL_FUNC_REF_BITS ) >> PLS_STENCIL_FUNC_REF_SHIFT );
		    stencilOpState.compareMask = uint32_t( ( m_flags & PLS_STENCIL_FUNC_MASK_BITS ) >> PLS_STENCIL_FUNC_MASK_SHIFT );
		    
		    switch ( m_flags & GLS_STENCIL_FUNC_BITS ) 
            {
			    case PLS_STENCIL_FUNC_NEVER:	stencilOpState.compareOp = VK_COMPARE_OP_NEVER; break;
			    case PLS_STENCIL_FUNC_LESS:		stencilOpState.compareOp = VK_COMPARE_OP_LESS; break;
			    case PLS_STENCIL_FUNC_EQUAL:	stencilOpState.compareOp = VK_COMPARE_OP_EQUAL; break;
			    case PLS_STENCIL_FUNC_LEQUAL:	stencilOpState.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
			    case PLS_STENCIL_FUNC_GREATER:	stencilOpState.compareOp = VK_COMPARE_OP_GREATER; break;
			    case PLS_STENCIL_FUNC_NOTEQUAL: stencilOpState.compareOp = VK_COMPARE_OP_NOT_EQUAL; break;
			    case PLS_STENCIL_FUNC_GEQUAL:	stencilOpState.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
			    case PLS_STENCIL_FUNC_ALWAYS:	stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS; break;
		    }
	    }

        if ( m_flags & ( PLS_STENCIL_OP_FAIL_BITS | PLS_STENCIL_OP_ZFAIL_BITS | PLS_STENCIL_OP_PASS_BITS ) ) 
        {

            /// Depth Fail Operation
            switch ( m_flags & PLS_STENCIL_OP_FAIL_BITS)
            {
                case PLS_STENCIL_OP_FAIL_KEEP:      stencilOpState.failOp = VK_STENCIL_OP_KEEP; break;
                case PLS_STENCIL_OP_FAIL_ZERO:      stencilOpState.failOp = VK_STENCIL_OP_ZERO; break;
                case PLS_STENCIL_OP_FAIL_REPLACE:   stencilOpState.failOp = VK_STENCIL_OP_REPLACE; break;
                case PLS_STENCIL_OP_FAIL_INCR:      stencilOpState.failOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; break;
                case PLS_STENCIL_OP_FAIL_DECR:      stencilOpState.failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP; break;
                case PLS_STENCIL_OP_FAIL_INVERT:    stencilOpState.failOp = VK_STENCIL_OP_INVERT; break;
                case PLS_STENCIL_OP_FAIL_INCR_WRAP: stencilOpState.failOp = VK_STENCIL_OP_INCREMENT_AND_WRAP; break;
                case PLS_STENCIL_OP_FAIL_DECR_WRAP: stencilOpState.failOp = VK_STENCIL_OP_DECREMENT_AND_WRAP; break;   
            }

            switch ( m_flags & PLS_STENCIL_OP_PASS_BITS )
            {
                case PLS_STENCIL_OP_PASS_KEEP:      stencilOpState.passOp = VK_STENCIL_OP_KEEP; break;
                case PLS_STENCIL_OP_PASS_ZERO:      stencilOpState.passOp = VK_STENCIL_OP_ZERO; break;
                case PLS_STENCIL_OP_PASS_REPLACE:   stencilOpState.passOp = VK_STENCIL_OP_REPLACE; break;
                case PLS_STENCIL_OP_PASS_INCR:      stencilOpState.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; break;
                case PLS_STENCIL_OP_PASS_DECR:      stencilOpState.passOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP; break;
                case PLS_STENCIL_OP_PASS_INVERT:    stencilOpState.passOp = VK_STENCIL_OP_INVERT; break;
                case PLS_STENCIL_OP_PASS_INCR_WRAP: stencilOpState.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP; break;
                case PLS_STENCIL_OP_PASS_DECR_WRAP: stencilOpState.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP; break;
            }

            switch ( m_flags & PLS_STENCIL_OP_ZFAIL_BITS )
            {
                case PLS_STENCIL_OP_ZFAIL_KEEP:         stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP; break;
                case PLS_STENCIL_OP_ZFAIL_ZERO:         stencilOpState.depthFailOp = VK_STENCIL_OP_ZERO; break;
                case PLS_STENCIL_OP_ZFAIL_REPLACE:      stencilOpState.depthFailOp = VK_STENCIL_OP_REPLACE; break;
                case PLS_STENCIL_OP_ZFAIL_INCR:         stencilOpState.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; break;
                case PLS_STENCIL_OP_ZFAIL_DECR:         stencilOpState.depthFailOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP; break;
                case PLS_STENCIL_OP_ZFAIL_INVERT:       stencilOpState.depthFailOp = VK_STENCIL_OP_INVERT; break;
                case PLS_STENCIL_OP_ZFAIL_INCR_WRAP:    stencilOpState.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_WRAP; break;
                case PLS_STENCIL_OP_ZFAIL_DECR_WRAP:    stencilOpState.depthFailOp = VK_STENCIL_OP_DECREMENT_AND_WRAP; break;
            }
        }
        
        depthStencilState.stencilTestEnable = VK_TRUE;
        depthStencilState.front = stencilOpState;
        depthStencilState.back = stencilOpState;
    }


    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = 0;
    
    //
	// check colormask
	//
	if ( m_flags & ( GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK | GLS_ALPHAMASK ) ) 
    {
        if ( m_flags & PLS_REDMASK )    colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		if ( m_flags & PLS_GREENMASK )  colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		if ( m_flags & PLS_BLUEMASK )   colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		if ( m_flags & PLS_ALPHAMASK )  colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
	}

    //
	// check blend bits
	//
	if ( m_flags & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) 
    {
		GLenum srcFactor = GL_ONE;
		GLenum dstFactor = GL_ZERO;

		switch ( stateBits & GLS_SRCBLEND_BITS ) {
			case GLS_SRCBLEND_ZERO:					srcFactor = GL_ZERO; break;
			case GLS_SRCBLEND_ONE:					srcFactor = GL_ONE; break;
			case GLS_SRCBLEND_DST_COLOR:			srcFactor = GL_DST_COLOR; break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:	srcFactor = GL_ONE_MINUS_DST_COLOR; break;
			case GLS_SRCBLEND_SRC_ALPHA:			srcFactor = GL_SRC_ALPHA; break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:	srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
			case GLS_SRCBLEND_DST_ALPHA:			srcFactor = GL_DST_ALPHA; break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:	srcFactor = GL_ONE_MINUS_DST_ALPHA; break;
			default:
				assert( !"GL_State: invalid src blend state bits\n" );
				break;
		}

		switch ( stateBits & GLS_DSTBLEND_BITS ) {
			case GLS_DSTBLEND_ZERO:					dstFactor = GL_ZERO; break;
			case GLS_DSTBLEND_ONE:					dstFactor = GL_ONE; break;
			case GLS_DSTBLEND_SRC_COLOR:			dstFactor = GL_SRC_COLOR; break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:	dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
			case GLS_DSTBLEND_SRC_ALPHA:			dstFactor = GL_SRC_ALPHA; break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:	dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
			case GLS_DSTBLEND_DST_ALPHA:			dstFactor = GL_DST_ALPHA; break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:  dstFactor = GL_ONE_MINUS_DST_ALPHA; break;
			default:
				assert( !"GL_State: invalid dst blend state bits\n" );
				break;
		}

		// Only actually update GL's blend func if blending is enabled.
		if ( srcFactor == GL_ONE && dstFactor == GL_ZERO ) {
			qglDisable( GL_BLEND );
		} else {
			qglEnable( GL_BLEND );
			qglBlendFunc( srcFactor, dstFactor );
		}
	}

    return true;
}

bool vkPipeline::operator==(const vkPipeline &p)
{
    if ( m_flags != p.m_flags )
        return false;

    if ( m_vertexShader != p.m_vertexShader )
        return false;

    if ( m_fragmentShader != p.m_fragmentShader )
        return false;

    return true;
}
