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

#include "idlib/precompiled.h"
#include "renderer/renderer_common.h"
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

bool vkPipeline::Create( const uint64_t in_flags, const vkProgramp in_vertexProgram, const vkProgramp in_fragmentProgram, const vkPipeline* in_reference )
{
    uint32_t attachmentCount = 0;
    m_flags = in_flags;
    auto device = tr.GetRenderDevice();
    m_vProgram = in_vertexProgram;
    m_fProgram = in_fragmentProgram;

    if( !m_vProgram || !m_fProgram )
        return false;

    ///
    ///
    ///
    VkPipelineShaderStageCreateInfo shaderStageCI[2];
    shaderStageCI[0] = m_vProgram->ShaderStage();
    shaderStageCI[1] = m_fProgram->ShaderStage();

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
        { VERTEX_BINDING, DRAWVERT_SIZE, VK_VERTEX_INPUT_RATE_VERTEX },
    };

    ///
    /// Attribute Description
    /// Describe vertex components
    VkVertexInputAttributeDescription vertexInputAttributeDescription[NUM_ATTRIBS_DESCR]
    {
        { VERTEX_ATTRIBUTE_POS, VERTEX_BINDING, VK_FORMAT_R32G32B32_SFLOAT, DRAWVERT_XYZ_OFFSET },
        { VERTEX_ATTRIBUTE_TEX, VERTEX_BINDING, VK_FORMAT_R16G16_SFLOAT, DRAWVERT_ST_OFFSET },
        { VERTEX_ATTRIBUTE_NOR, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_NORMAL_OFFSET },
        { VERTEX_ATTRIBUTE_TAN, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_TANGENT_OFFSET },
        { VERTEX_ATTRIBUTE_JOI, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_COLOR_OFFSET },
        { VERTEX_ATTRIBUTE_WEI, VERTEX_BINDING, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_COLOR2_OFFSET }
    };

    /// 
    /// Vertex input
    /// describes the format of the vertex data that will be passed to the vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;;
    vertexInputState.pNext = nullptr;
    vertexInputState.flags = 0;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = vertexInputBindingDescription;
    vertexInputState.vertexAttributeDescriptionCount = NUM_ATTRIBS_DESCR;
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributeDescription;

    ///
    /// Input Assembly
    /// Describes what kind of geometry will be drawn from the vertices and if primitive restart
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;;
    inputAssemblyState.pNext = nullptr;
    inputAssemblyState.flags = 0;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    /// TODO: implement by material tesselation, and vertex displacement
    /// Tessellation State
    /// Control tesselation path
    VkPipelineTessellationStateCreateInfo tessellationState{};
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState.pNext = nullptr;
    tessellationState.flags = 0;
    tessellationState.patchControlPoints = 0;

    ///
    /// Viewport State
    /// Viewport and scissor configuration ( not set in pipeline, dynamic )
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr; // Dynamically defined 
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr; // Dynamically defined 

    ///
    /// Rasterization State
    ///
    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.pNext = nullptr;
    rasterizationState.flags = 0;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = ( m_flags & PLS_POLYMODE_LINE ) == PLS_POLYMODE_LINE ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 0.0f;
    rasterizationState.lineWidth = 1.0f;

    uint64_t cullVal = (m_flags & PLS_CULLFACE_BITS) >> 0;
    if (cullVal == ( PLS_CULLFACE_BACK >> 0 )) 
        rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    else if (cullVal == ( PLS_CULLFACE_FRONT >> 0 ) )
        rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
    else 
        rasterizationState.cullMode = VK_CULL_MODE_NONE;
    
    /// TODO:
    /// Multisample State
    /// configure multisample state
    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pNext = nullptr;
    multisampleState.flags = 0;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //TODO: cvar for multisampling ( on that we required )
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 0.1;
    multisampleState.pSampleMask = 0;
    multisampleState.alphaToCoverageEnable = VK_FALSE;
    multisampleState.alphaToOneEnable = VK_FALSE;

    ///
    /// Pipeline Depth Stencil State 
    ///
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
		    
		    switch ( m_flags & PLS_STENCIL_FUNC_BITS ) 
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

    ////
    VkPipelineColorBlendAttachmentState attachmentsStates[3]{};
    
    ////
    if( m_flags & PLS_COLOR_ATTACHAMENT )
    {
        attachmentsStates[attachmentCount].blendEnable = VK_FALSE;
        attachmentsStates[attachmentCount].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentsStates[attachmentCount].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentsStates[attachmentCount].colorBlendOp = VK_BLEND_OP_ADD;
        attachmentsStates[attachmentCount].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentsStates[attachmentCount].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentsStates[attachmentCount].alphaBlendOp = VK_BLEND_OP_ADD;
        attachmentsStates[attachmentCount].colorWriteMask = 0;
        
        //
        // check colormask
        //
        if ( m_flags & ( PLS_REDMASK | PLS_GREENMASK | PLS_BLUEMASK | PLS_ALPHAMASK ) ) 
        {
            if ( m_flags & PLS_REDMASK )    attachmentsStates[attachmentCount].colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            if ( m_flags & PLS_GREENMASK )  attachmentsStates[attachmentCount].colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            if ( m_flags & PLS_BLUEMASK )   attachmentsStates[attachmentCount].colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            if ( m_flags & PLS_ALPHAMASK )  attachmentsStates[attachmentCount].colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
        }

        //
        // check blend bits
        //
        if ( m_flags & ( PLS_SRCBLEND_BITS | PLS_DSTBLEND_BITS ) ) 
        {
            VkBlendFactor srcFactor;
            VkBlendFactor dstFactor;
            switch ( m_flags & PLS_SRCBLEND_BITS ) 
            {
                case PLS_SRCBLEND_ZERO:
                    srcFactor = VK_BLEND_FACTOR_ZERO; 
                    break;
                case PLS_SRCBLEND_ONE:
                    srcFactor = VK_BLEND_FACTOR_ONE; 
                    break;
                case PLS_SRCBLEND_DST_COLOR:
                    srcFactor = VK_BLEND_FACTOR_DST_COLOR; 
                    break;
                case PLS_SRCBLEND_ONE_MINUS_DST_COLOR:
                    srcFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR; 
                    break;
                case PLS_SRCBLEND_SRC_ALPHA:
                    srcFactor = VK_BLEND_FACTOR_SRC_ALPHA; 
                    break;
                case PLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
                    srcFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
                    break;
                case PLS_SRCBLEND_DST_ALPHA:
                    srcFactor = VK_BLEND_FACTOR_DST_ALPHA; 
                    break;
                case PLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
                    srcFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; 
                    break;
                default:
                    idassert( !"GL_State: invalid src blend state bits\n" );
                    break;
            }

            switch ( m_flags & PLS_DSTBLEND_BITS ) 
            {
                case PLS_DSTBLEND_ZERO:					
                    dstFactor = VK_BLEND_FACTOR_ZERO; 
                    break;
                case PLS_DSTBLEND_ONE:					
                    dstFactor = VK_BLEND_FACTOR_ONE; 
                    break;
                case PLS_DSTBLEND_SRC_COLOR:			
                    dstFactor = VK_BLEND_FACTOR_SRC_COLOR; 
                    break;
                case PLS_DSTBLEND_ONE_MINUS_SRC_COLOR:	
                    dstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR; 
                    break;
                case PLS_DSTBLEND_SRC_ALPHA:			
                    dstFactor = VK_BLEND_FACTOR_SRC_ALPHA; 
                    break;
                case PLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:	
                    dstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
                    break;
                case PLS_DSTBLEND_DST_ALPHA:			
                    dstFactor = VK_BLEND_FACTOR_DST_ALPHA; 
                    break;
                case PLS_DSTBLEND_ONE_MINUS_DST_ALPHA:  
                    dstFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; 
                    break;
                default:
                    assert( !"GL_State: invalid dst blend state bits\n" );
                    break;
            }

            // Only actually update GL's blend func if blending is enabled.
            if ( srcFactor == VK_BLEND_FACTOR_ONE && dstFactor == VK_BLEND_FACTOR_ZERO ) 
            {
                attachmentsStates[attachmentCount].blendEnable = VK_FALSE;
            } 
            else 
            {
                attachmentsStates[attachmentCount].blendEnable = VK_TRUE;
                attachmentsStates[attachmentCount].srcColorBlendFactor = srcFactor;
                attachmentsStates[attachmentCount].dstColorBlendFactor = dstFactor;
                attachmentsStates[attachmentCount].srcAlphaBlendFactor = srcFactor;
                attachmentsStates[attachmentCount].dstAlphaBlendFactor = dstFactor;
            }
        }

        attachmentCount++;
    }

    if( m_flags & PLS_NORMAL_ATTACHAMENT )
    {
        attachmentsStates[attachmentCount].blendEnable = VK_FALSE;
        attachmentsStates[attachmentCount].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentsStates[attachmentCount].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentsStates[attachmentCount].colorBlendOp = VK_BLEND_OP_ADD;
        attachmentsStates[attachmentCount].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentsStates[attachmentCount].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentsStates[attachmentCount].alphaBlendOp = VK_BLEND_OP_ADD;
        attachmentsStates[attachmentCount].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        attachmentCount++;
    }

    ///
    ///
    ///
    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.pNext = nullptr;
    colorBlendState.flags = 0;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
    colorBlendState.attachmentCount = attachmentCount; // TODO: defferred light pass need 4 attachaments...
    colorBlendState.pAttachments = attachmentsStates;
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    ///
    ///
    ///
    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = nullptr;
    pipelineCI.flags = in_reference != nullptr ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStageCI;
    pipelineCI.pVertexInputState = &vertexInputState;
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pTessellationState = &tessellationState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = crUniformManager::Get()->Layout();
    pipelineCI.renderPass = VK_NULL_HANDLE; /// VK_KHR_dynamic_rendering 
    pipelineCI.subpass = 0;
    pipelineCI.basePipelineHandle = *in_reference;
    pipelineCI.basePipelineIndex = -1;
    auto result = vkCreateGraphicsPipelines( *device, device->PipelineCache(), 1, &pipelineCI, k_allocationCallbacks, &m_pipeline );
    if( result != VK_SUCCESS )
    {
        idLib::Error( "crPipeline::Create::vkCreateGraphicsPipelines Failed\n %s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    return true;
}

bool vkPipeline::operator==(const vkPipeline &p)
{
    if ( m_flags != p.m_flags )
        return false;

    if ( m_vProgram->ID() != p.m_vProgram->ID() )
        return false;

    if ( m_fProgram->ID() != p.m_fProgram->ID() )
        return false;

    return true;
}
