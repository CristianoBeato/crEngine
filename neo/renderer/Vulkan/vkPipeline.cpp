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
#include "vkPipeline.hpp"

vkPipeline::vkPipeline( void ) : crPipeline()
{
}

vkPipeline::~vkPipeline( void )
{
}

bool vkPipeline::Create( const PipelineInfo_t in_pipelineInfo )
{
    VkResult result = VK_SUCCESS;
    VkPipelineDynamicStateCreateInfo        dynamicState{};
    VkPipelineVertexInputStateCreateInfo    vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo  inputAssembly{};
    VkPipelineTessellationStateCreateInfo   tessellationState{};
    VkPipelineViewportStateCreateInfo       viewportState{};
    VkPipelineRasterizationStateCreateInfo  rasterizer{};
    VkPipelineMultisampleStateCreateInfo    multisampling{};
    VkPipelineDepthStencilStateCreateInfo   depthStencilState{};
    VkPipelineColorBlendAttachmentState     colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo     colorBlending{};
    VkGraphicsPipelineCreateInfo            pipelineCI{};
    idList<VkPipelineShaderStageCreateInfo> shaderStages;

    auto device = tr.vkContext->Device();
    m_device = *device;

    // get program stages
    shaderStages.Resize( in_pipelineInfo.numPrograms );
    for ( uint32_t i = 0; i < in_pipelineInfo.numPrograms; i++)
    {
        shaderStages[i] = static_cast<vkProgram*>( in_pipelineInfo.shaderPrograms[i] )->ShaderStage();
    }
 
    //
    // Dynamic states
    // these are the pipelines states, that we can change whitout need change the pipeline
    VkDynamicState dynamicStates[]
    {
        VK_DYNAMIC_STATE_VIEWPORT,      // we can update viewport to subdraw
        VK_DYNAMIC_STATE_SCISSOR,       // we can set scissor based on light bounds
        VK_DYNAMIC_STATE_LINE_WIDTH,    // set line width for tools
        VK_DYNAMIC_STATE_DEPTH_BIAS,    // change depths bias
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,  // depth bounds 
        VK_DYNAMIC_STATE_CULL_MODE,
        VK_DYNAMIC_STATE_FRONT_FACE
    };

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = SDL_arraysize( dynamicStates );
    dynamicState.pDynamicStates = dynamicStates;

    //
    //
    // Pipeline Vertex Input State
    VkVertexInputBindingDescription vertexInputBindingDescription{};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.stride = sizeof( idDrawVert );
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // idDrawVert attribs
    VkVertexInputAttributeDescription vertexInputAttributeDescription[]
    {
        { PC_ATTRIB_INDEX_VERTEX, 0, VK_FORMAT_R32G32B32_SFLOAT, DRAWVERT_XYZ_OFFSET },
        { PC_ATTRIB_INDEX_NORMAL, 0, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_NORMAL_OFFSET },
        { PC_ATTRIB_INDEX_COLOR, 0, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_COLOR_OFFSET },
        { PC_ATTRIB_INDEX_COLOR2, 0, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_COLOR2_OFFSET },
        { PC_ATTRIB_INDEX_ST, 0, VK_FORMAT_R16G16_SFLOAT, DRAWVERT_ST_OFFSET },
        { PC_ATTRIB_INDEX_TANGENT, 0, VK_FORMAT_R8G8B8A8_UNORM, DRAWVERT_TANGENT_OFFSET },
    };

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = SDL_arraysize( vertexInputAttributeDescription );
    vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

    //
    //
    // PipelineI nput Assembly State
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //
    //
    // Pipeline Tessellation State 
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState.pNext = nullptr;
    tessellationState.flags = 0;
    tessellationState.patchControlPoints = 0; // not set ...for now!
    
    //
    //
    // Pipeline Viewport State
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr; // we specify in command buffer 
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr; // we specify in command buffer 

    //
    //
    // Pipeline Rasterization State
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_TRUE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.lineWidth = 1.0f;
    
    switch ( in_pipelineInfo.polygonMode )
    {
    case PM_POINT:
        rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
        break;
    case PM_LINE:
        rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
        break;
    case PM_FILL:
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        break;
    }

    switch ( in_pipelineInfo.faceCull )
    {
    case FC_BACK:
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    case FC_FRONT:
        rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        break;
    case FC_TWO_FACES:
        rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
        break;
    }

    //
    //
    // VkPipeline Multisample State
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = glConfig.multisamples > 1 ? VK_TRUE : VK_FALSE;
    multisampling.minSampleShading = 1.0; // todo: configuere via cvar
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = GL_FALSE;
    multisampling.alphaToOneEnable = GL_TRUE;

    switch ( glConfig.multisamples )
    {
    case 1:
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        break;

    case 2:
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
        break;

    case 4:
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
        break;

    case 8:
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
        break;

    case 16:
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;
        break;
    
    default:
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
    }    

    //
    //
    // Pipeline Depth Stencil State
    VkStencilOpState    stencilOpState{};
    stencilOpState.failOp = VK_STENCIL_OP_KEEP;
    stencilOpState.passOp = VK_STENCIL_OP_KEEP;
    stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP;
    stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS;
    stencilOpState.compareMask = 0xFFFFFFFF;
    stencilOpState.writeMask = 0xFFFFFFFF;
    stencilOpState.reference = 0x00000000;

    switch( m_pipelineConfiguration.stencilPass )
    {
        case STENCIL_OP_KEEP:
            stencilOpState.passOp = VK_STENCIL_OP_KEEP;
            break;
        case STENCIL_OP_ZERO:
            stencilOpState.passOp = VK_STENCIL_OP_ZERO;
            break;
        case STENCIL_OP_REPLACE:
            stencilOpState.passOp = VK_STENCIL_OP_REPLACE;
            break;
        case STENCIL_OP_INCR:
            stencilOpState.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            break;
        case STENCIL_OP_DECR:
            stencilOpState.passOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            break;
        case STENCIL_OP_INVERT:
            stencilOpState.passOp = VK_STENCIL_OP_INVERT;
            break;
        case STENCIL_OP_INCR_WRAP:
            stencilOpState.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
            break;
        case STENCIL_OP_DECR_WRAP:
            stencilOpState.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
            break;
    };

    switch( m_pipelineConfiguration.stencilFail )
    {
       case STENCIL_OP_KEEP:
            stencilOpState.failOp = VK_STENCIL_OP_KEEP;
            break;
        case STENCIL_OP_ZERO:
            stencilOpState.failOp = VK_STENCIL_OP_ZERO;
            break;
        case STENCIL_OP_REPLACE:
            stencilOpState.failOp = VK_STENCIL_OP_REPLACE;
            break;
        case STENCIL_OP_INCR:
            stencilOpState.failOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            break;
        case STENCIL_OP_DECR:
            stencilOpState.failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            break;
        case STENCIL_OP_INVERT:
            stencilOpState.failOp = VK_STENCIL_OP_INVERT;
            break;
        case STENCIL_OP_INCR_WRAP:
            stencilOpState.failOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
            break;
        case STENCIL_OP_DECR_WRAP:
            stencilOpState.failOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
            break;
    };
    
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.pNext = nullptr;
    depthStencilState.flags = 0;
    depthStencilState.depthTestEnable = ( in_pipelineInfo.depthFunc != DF_NONE ) ? VK_TRUE : VK_FALSE; // Enable depth test
    depthStencilState.depthWriteEnable = ( in_pipelineInfo.depthFunc != DF_NONE ) ? VK_TRUE : VK_FALSE;;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS; // defalt depth test 
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = ( in_pipelineInfo.stencilPass ) ? VK_TRUE : VK_FALSE;
    depthStencilState.front = stencilOpState;
    depthStencilState.back = stencilOpState;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    switch ( in_pipelineInfo.depthFunc )
    {
    case DF_ALWAYS:
        depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        break;
    case DF_LESS:
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        break;
    case DF_GREATER:
        depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
        break;
    case DF_EQUAL:
        depthStencilState.depthCompareOp = VK_COMPARE_OP_EQUAL;
        break;
    default:
        break;
    };

    //
    //
    // Pipeline Color Blend Attachment State
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = 0;// VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    if( !( m_pipelineConfiguration.blendSource == BLEND_SRC_ONE && m_pipelineConfiguration.blendDestination == BLEND_DST_ZERO ) )
    {
        switch( m_pipelineConfiguration.blendSource )
        {
        case BLEND_SRC_ONE:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            break;

        case BLEND_SRC_ZERO:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;

        case BLEND_SRC_DST_COLOR:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
            break;

        case BLEND_SRC_ONE_MINUS_DST_COLOR:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            break;

        case BLEND_SRC_SRC_ALPHA:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            break;

        case BLEND_SRC_ONE_MINUS_SRC_ALPHA:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;

        case BLEND_SRC_DST_ALPHA:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            break;

        case BLEND_SRC_ONE_MINUS_DST_ALPHA:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            break;
        };

        switch( m_pipelineConfiguration.blendDestination )
        {
        case BLEND_DST_ZERO:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;

        case BLEND_DST_ONE:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            break;

        case BLEND_DST_SRC_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
            break;

        case BLEND_DST_ONE_MINUS_SRC_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            break;

        case BLEND_DST_SRC_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            break;

        case BLEND_DST_ONE_MINUS_SRC_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;

        case BLEND_DST_DST_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            break;

        case BLEND_DST_ONE_MINUS_DST_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            break;
        }

        switch( m_pipelineConfiguration.blendOperation )
        {
            case BLEND_OP_ADD:
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
                break;
            case BLEND_OP_SUB:
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_SUBTRACT;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
                break;
            case BLEND_OP_MIN:
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_MIN;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_MIN;
                break;
            case BLEND_OP_MAX:
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_MAX;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_MAX;
                break;
        };
    }

    // enable red mask
    if ( in_pipelineInfo.colorMask & CM_RED_MASK )
        colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
    
    // enable green mask
    if ( in_pipelineInfo.colorMask & CM_GREEN_MASK )
        colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
    
    // enable blue mask
    if ( in_pipelineInfo.colorMask & CM_BLUE_MASK )
        colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
    
    // enable alpha mask
    if ( in_pipelineInfo.colorMask & CM_ALPHA_MASK )
        colorBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

    ///
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.flags = VK_LOGIC_OP_COPY;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    vkShaderStorage* st = dynamic_cast<vkShaderStorage*>( backEnd.GetShaderStorage() );
    
    //
    //
    // Pipeline Create Info
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // todo: aquire from context
    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &format;
    // use depth and stencil
    renderingInfo.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = &renderingInfo;
    pipelineCI.flags = 0;
    pipelineCI.stageCount = shaderStages.Num();
    pipelineCI.pStages = shaderStages.Ptr();
    pipelineCI.pVertexInputState = &vertexInputInfo;
    pipelineCI.pInputAssemblyState = &inputAssembly;
    pipelineCI.pTessellationState = &tessellationState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizer;
    pipelineCI.pMultisampleState = &multisampling;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = st->PipelineLayout();
    pipelineCI.renderPass = VK_NULL_HANDLE;
    pipelineCI.subpass = 0;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.basePipelineIndex = -1;

    m_device = *device;
    result = vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &pipelineCI, k_allocationCallbacks, &m_pipeline );
    if( !ResultCheck( result,"vkCreateGraphicsPipelines" ) )
        return false;

    return true;
}

void vkPipeline::Destroy( void )
{
    if( m_pipeline != nullptr )
    {
        vkDestroyPipeline( m_device, m_pipeline, k_allocationCallbacks );
        m_pipeline = nullptr;
    }
}
