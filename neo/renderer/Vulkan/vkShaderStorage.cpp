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
#include "vkShaderStorage.hpp"

constexpr uint32_t MAX_BINDINGS = 4;
constexpr uint32_t BINDING_SAMP = 0;    // binding 0: combined image sampler array (runtime-sized)
constexpr uint32_t BINDING_VERT = 1;    // binding 1: SSBO para dados de vertex
constexpr uint32_t BINDING_FRAG = 2;    // binding 2: SSBO para dados de fragment
constexpr uint32_t BINDING_LIGH = 3;    // binding 4: SSBO para dados de luz

vkShaderStorage::vkShaderStorage( void ) : crShaderStorage(),
    m_descriptorSetLayout( nullptr ),
    m_pipelineLayout( nullptr ),
    m_descriptorPool( nullptr ),
    m_bindlessSet( nullptr ),
    m_device( nullptr )
{
    VkResult result = VK_SUCCESS;
    VkDescriptorSetLayoutBinding    bindings[MAX_BINDINGS]{};
    VkDescriptorBindingFlags    bindingFlagsArr[MAX_BINDINGS]{};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo{};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    VkDescriptorPoolSize    poolSizes[2]{};
    VkDescriptorPoolCreateInfo  poolInfo{};
    VkDescriptorSetVariableDescriptorCountAllocateInfo  varCountAlloc{};
    VkDescriptorSetAllocateInfo allocInfo{};

    /*
    =================================================================================================
        Create Shader Storage Buffers Buffers
    =================================================================================================
    */
    m_VTSSBO = new vkBuffer();
    m_FGSSBO = new vkBuffer();
    m_LHSSBO = new vkBuffer();
    m_VTSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_VERT_SIZE * SMP_FRAMES );
    m_FGSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_FRAG_SIZE * SMP_FRAMES );
    m_LHSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_LIGH_SIZE * SMP_FRAMES );

    
    /*
    =================================================================================================
        Descriptor Set Layout
    =================================================================================================
    */

    // binding 0: combined image sampler array (runtime-sized)
    // binding 1: SSBO para dados de vertex
    // binding 2: SSBO para dados de fragment
    // binding 4: SSBO para dados de luz

    // BINDING 0 - bindless images (combined image sampler)
    bindings[BINDING_SAMP].binding = 0;
    bindings[BINDING_SAMP].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[BINDING_SAMP].descriptorCount = 1; // placeholder (variable at alloc time)
    bindings[BINDING_SAMP].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[BINDING_SAMP].pImmutableSamplers = nullptr;

    bindingFlagsArr[BINDING_SAMP] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    // BINDING 1 - SSBO vertex
    bindings[BINDING_VERT].binding = 1;
    bindings[BINDING_VERT].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[BINDING_VERT].descriptorCount = 1;
    bindings[BINDING_VERT].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[BINDING_VERT].pImmutableSamplers = nullptr;

    bindingFlagsArr[BINDING_VERT] = 0; // SSBOs não precisam de flags especiais


    // BINDING 2 - SSBO fragment
    bindings[BINDING_FRAG].binding = 2;
    bindings[BINDING_FRAG].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[BINDING_FRAG].descriptorCount = 1;
    bindings[BINDING_FRAG].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[BINDING_FRAG].pImmutableSamplers = nullptr;

    bindingFlagsArr[BINDING_FRAG] = 0;

    // BINDING 3 - SSBO de light
    bindings[BINDING_LIGH].binding = 2;
    bindings[BINDING_LIGH].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[BINDING_LIGH].descriptorCount = 1;
    bindings[BINDING_LIGH].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[BINDING_LIGH].pImmutableSamplers = nullptr;

    bindingFlagsArr[BINDING_LIGH] = 0;   

    ///
    /// configure layout bind flags 
    flagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsCreateInfo.bindingCount = MAX_BINDINGS;
    flagsCreateInfo.pBindingFlags = bindingFlagsArr;

    ///
    ///
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &flagsCreateInfo;
    layoutInfo.bindingCount = MAX_BINDINGS;
    layoutInfo.pBindings = bindings;
    // permitir update-after-bind pool behavior:
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    result = vkCreateDescriptorSetLayout( m_device, &layoutInfo, k_allocationCallbacks, &m_descriptorSetLayout );
    if ( result != VK_SUCCESS )
        common->FatalError( "vkCreateDescriptorSetLayout %s\n", crvkGetVulkanError( result ) );

    /*
    =================================================================================================
        Pipeline Layout
    =================================================================================================
    */
    VkPushConstantRange pcRange{};
    pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pcRange.offset = 0;
    pcRange.size = sizeof(uint32_t); // ex: materialID ou objectID

    VkPipelineLayoutCreateInfo  pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.pNext = nullptr;
    pipelineLayoutCI.flags = 0;
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutCI.pushConstantRangeCount = 1;
    pipelineLayoutCI.pPushConstantRanges = &pcRange;

    result = vkCreatePipelineLayout( m_device, &pipelineLayoutCI, k_allocationCallbacks, &m_pipelineLayout );
    if ( result != VK_SUCCESS )
        common->FatalError( "vkCreatePipelineLayout %s\n", crvkGetVulkanError( result ) );
    
    /*
    =================================================================================================
        Pipeline Descriptor Pool
    =================================================================================================
    */
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = MAX_BINDING_SAMPLERS; 
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 16; // quantos SSBOs você planeja (normalmente 1 por binding)

    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1; // provavelmente 1 set global

    result = vkCreateDescriptorPool( m_device, &poolInfo, nullptr, &m_descriptorPool );
    if( result != VK_SUCCESS )
        common->FatalError( "vkCreateDescriptorPool %s\n", crvkGetVulkanError( result ) );

    /*
    =================================================================================================
    variable descriptor count
    =================================================================================================
    */

    ///
    uint32_t descriptorCountForBinding0 = 4096; // ex: 4096
    varCountAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    varCountAlloc.descriptorSetCount = 1;
    varCountAlloc.pDescriptorCounts = &descriptorCountForBinding0;

    ///
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &varCountAlloc;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    result = vkAllocateDescriptorSets( m_device, &allocInfo, &m_bindlessSet );
    if( result != VK_SUCCESS )
        common->FatalError( "vkAllocateDescriptorSets %s\n", crvkGetVulkanError( result ) );

}

vkShaderStorage::~vkShaderStorage( void )
{    
    if ( m_bindlessSet != nullptr )
    {
        vkFreeDescriptorSets( m_device, m_descriptorPool, 1, &m_bindlessSet );
        m_bindlessSet = nullptr;
    }
 
    if ( m_descriptorPool != nullptr )
    {
        vkDestroyDescriptorPool( m_device, m_descriptorPool, k_allocationCallbacks );
        m_descriptorPool = nullptr;
    }

    if ( m_pipelineLayout != nullptr )
    {
        vkDestroyPipelineLayout( m_device, m_pipelineLayout, k_allocationCallbacks );
        m_pipelineLayout = nullptr;
    }
    
    if ( m_descriptorSetLayout != nullptr )
    {
        vkDestroyDescriptorSetLayout( m_device, m_descriptorSetLayout, k_allocationCallbacks );
        m_descriptorSetLayout = nullptr;
    }
    
    delete m_VTSSBO;
    m_VTSSBO = nullptr;
    delete m_FGSSBO;
    m_FGSSBO = nullptr;
    delete m_LHSSBO;
    m_LHSSBO = nullptr;
}


void vkShaderStorage::Begin(void)
{
    //
    // bindless texture array 
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_bindlessSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = m_imageInfos.Num();
    write.pImageInfo = m_imageInfos.Ptr();
    vkUpdateDescriptorSets( m_device, 1, &write, 0, nullptr);

    //
    // vertex uniforms buffer offset
    VkDescriptorBufferInfo buf1{};
    buf1.buffer = *static_cast<VkBuffer*>( m_VTSSBO->Handle() );
    buf1.offset = m_currentVBlock * sizeof( vertexUniformBlock_t );
    buf1.range = MAX_UNIFORM_BLOCKS * sizeof( vertexUniformBlock_t );
    
    VkWriteDescriptorSet writeSSBO1{};
    writeSSBO1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSSBO1.dstSet = m_bindlessSet;
    writeSSBO1.dstBinding = 1;
    writeSSBO1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeSSBO1.descriptorCount = 1;
    writeSSBO1.pBufferInfo = &buf1;

    //
    // fragment uniform buffer 
    VkDescriptorBufferInfo buf2{};
    buf2.buffer = *static_cast<VkBuffer*>( m_FGSSBO->Handle() );
    buf2.offset = m_currentFSBlock * sizeof( fragmentUniformBlock_t );
    buf2.range = MAX_UNIFORM_BLOCKS * sizeof( fragmentUniformBlock_t );
    
    VkWriteDescriptorSet writeSSBO2{};
    writeSSBO2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSSBO2.dstSet = m_bindlessSet;
    writeSSBO2.dstBinding = 2;
    writeSSBO2.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeSSBO2.descriptorCount = 1;
    writeSSBO2.pBufferInfo = &buf2;

    //
    // light uniform buffer
    VkDescriptorBufferInfo buf3{}; 
    buf3.buffer = *static_cast<VkBuffer*>( m_LHSSBO->Handle() );
    buf3.offset = m_currentLSBlock * sizeof( lightUnifomBlock_t ); 
    buf3.range = sizeof( lightUnifomBlock_t ) * MAX_LIGHT_BLOCKS;

    VkWriteDescriptorSet writeSSBO3{};
    writeSSBO3.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSSBO3.dstSet = m_bindlessSet;
    writeSSBO3.dstBinding = 3;
    writeSSBO3.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeSSBO3.descriptorCount = 1;
    writeSSBO3.pBufferInfo = &buf3;

    VkWriteDescriptorSet writes[] = { write, writeSSBO1, writeSSBO2, writeSSBO3 };
    vkUpdateDescriptorSets( m_device, 3, writes, 0, nullptr);
}

crBindlessTextureSlot *vkShaderStorage::BindTexture(const crTexture *in_texture, const crSampler *in_sampler)
{
    vkBindlessTextureSlot *freeSlot = nullptr;
    if( !m_freeList.Num() > 0 )
    { 
        uint32_t index = m_freeList.Last();       
        freeSlot = new vkBindlessTextureSlot( *static_cast<VkImageView*>( in_texture->Handler() ), *static_cast<VkSampler*>( in_sampler->Handler() ), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
        m_imageInfos[index] = freeSlot->GetHandle(); 
        
        // remove last 
        m_freeList.RemoveIndex( m_freeList.Num() - 1 );
    }
    else
    {
        uint32_t index = m_lastTextureIndex++;
        freeSlot = new vkBindlessTextureSlot( *static_cast<VkImageView*>( in_texture->Handler() ), *static_cast<VkSampler*>( in_sampler->Handler() ), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
        m_imageInfos[index] = freeSlot->GetHandle(); 
        freeSlot->SetIndex( index );
    }

    return freeSlot;
}

void vkShaderStorage::FreeSlot(crBindlessTextureSlot *&in_handle)
{
    m_freeList.Append( in_handle->GetIndex() );
    delete in_handle;
}
