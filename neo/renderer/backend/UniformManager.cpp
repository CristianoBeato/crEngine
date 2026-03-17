
#include "idlib/precompiled.h"
#include "Vulkan/Core.hpp"
#include "UniformManager.hpp"

static uint32_t k_BUFFERS_SIZES[MAX_BINDINGS] =
{
    sizeof( uMesh_t ) * 4086 * SMP_FRAMES,
    sizeof( uMaterial_t ) * 4086 * SMP_FRAMES,
    sizeof( uLight_t ) * 4086,
    sizeof( uJointMatrix_t ) * 256 * ( 4086 * SMP_FRAMES ), //neet to review this size
};

/*
================================================================================================
vkSamplerSlot
================================================================================================
*/
vkSamplerSlot::vkSamplerSlot( void ) : m_index( 0xFFFFFFFF ) 
{
}

vkSamplerSlot::vkSamplerSlot(const VkImageView in_imageView, const VkSampler in_sampler, const VkImageLayout in_imageLayout)
{
    m_descriptorImageInfo.imageLayout = in_imageLayout;
    m_descriptorImageInfo.imageView = in_imageView;
    m_descriptorImageInfo.sampler = in_sampler;
}

vkSamplerSlot::~vkSamplerSlot(void)
{
    m_descriptorImageInfo.imageView = nullptr;
    m_descriptorImageInfo.sampler = nullptr;
}

crUniformManager *crUniformManager::Get(void)
{
    static crUniformManager gUniformManager = crUniformManager();
    return &gUniformManager;
}

crUniformManager::crUniformManager(void) : m_bindlessSetLayout(nullptr),
                                           m_storageSetLayout(nullptr),
                                           m_bindlessPool(nullptr),
                                           m_storagePool(nullptr),
                                           m_bindlessSet(nullptr)
{
}

crUniformManager::~crUniformManager(void)
{
}

void crUniformManager::StartUp( void )
{
    VkResult result = VK_SUCCESS;
    auto device = tr.GetRenderDevice();
    
    CreateBindlessSet();
    CreateStorageSet();
    CreateStorageBuffers();
    
    VkDescriptorSetLayout descriptorSetLayout[2]{ nullptr, nullptr };
    descriptorSetLayout[0] = m_bindlessSetLayout; /// Set 0 (High Frequency)
    descriptorSetLayout[1] = m_storageSetLayout; /// Set 1 (Low Frequency/Global)

    ///
    /// Pipeline Layout
    ///
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.pNext = nullptr;
    pipelineLayoutCI.flags = 0;
    pipelineLayoutCI.setLayoutCount = 2;
    pipelineLayoutCI.pSetLayouts = descriptorSetLayout;
    pipelineLayoutCI.pushConstantRangeCount = 0;
    pipelineLayoutCI.pPushConstantRanges = nullptr;
    result = vkCreatePipelineLayout( *device, &pipelineLayoutCI, k_allocationCallbacks, &m_layout );
    if( result != VK_SUCCESS )
    {
        idLib::FatalError( "crUniformManager::StartUp:vkCreatePipelineLayout Failed\n%s\n", VulkanErrorString( result ).c_str() );
    }
}

void crUniformManager::ShutDown(void)
{
    auto device = tr.GetRenderDevice();
    if( m_layout != nullptr )
    {
        vkDestroyPipelineLayout( *device, m_layout, k_allocationCallbacks );
        m_layout = nullptr;
    }

    if( m_bindlessSet != nullptr )
    {
        vkFreeDescriptorSets( *device, m_bindlessPool, 1, &m_bindlessSet );
        m_bindlessSet = nullptr;
    }

    if ( m_bindlessPool != nullptr )
    {
        vkDestroyDescriptorPool( *device, m_bindlessPool, k_allocationCallbacks );
        m_bindlessPool = nullptr;
    }

    if( m_bindlessSetLayout != nullptr )
    {
        vkDestroyDescriptorSetLayout( *device, m_bindlessSetLayout, k_allocationCallbacks );
        m_bindlessSetLayout = nullptr;
    }

    for ( uint32_t i = 0; i < MAX_BINDINGS; i++)
    {
        m_shaderStorageBuffers[i]->Destroy();   
        delete m_shaderStorageBuffers[i];
    }
    
}

void crUniformManager::SetFrame( const uint32_t in_frameID, const vkCommandbuffer * in_commandBuffer )
{
    auto device = tr.GetRenderDevice();
    idTempArray<VkDescriptorBufferInfo> bufferDescriptors = idTempArray<VkDescriptorBufferInfo>( MAX_BINDINGS ); 
    idTempArray<VkWriteDescriptorSet>   writeDescriptors = idTempArray<VkWriteDescriptorSet>( 5 );

    m_frameID = in_frameID;

    /// ------------------------------------------------------------------------------------
    /// Combined sampler binding
    writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptors[0].pNext = nullptr;
    writeDescriptors[0].dstSet = m_bindlessSet;
    writeDescriptors[0].dstBinding = BINDING_SAMP;
    writeDescriptors[0].dstArrayElement = 0;
    writeDescriptors[0].descriptorCount = MAX_BINDING_SAMPLERS;
    writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptors[0].pImageInfo = m_combinedSamplersLocations.Ptr();

    /// ------------------------------------------------------------------------------------
    /// Bind the mesh shader storage buffer
    bufferDescriptors[1].buffer = m_shaderStorageBuffers[BINDING_MESH]->Handle();
    bufferDescriptors[1].offset = 0;
    bufferDescriptors[1].range = VK_WHOLE_SIZE; /// access whole buffer

    /// Mesh buffer binding decriptor
    writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptors[1].pNext = nullptr;
    writeDescriptors[1].dstSet = m_storageSet[m_frameID];
    writeDescriptors[1].dstBinding = BINDING_MESH;
    writeDescriptors[1].dstArrayElement = 0;
    writeDescriptors[1].descriptorCount = 1;
    writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptors[1].pBufferInfo = &bufferDescriptors[BINDING_MESH];

    /// ------------------------------------------------------------------------------------
    /// Bind the material shader storage buffer
    bufferDescriptors[2].buffer = m_shaderStorageBuffers[BINDING_MATE]->Handle();
    bufferDescriptors[2].offset = 0;
    bufferDescriptors[2].range = VK_WHOLE_SIZE; /// access whole buffer

    /// Material buffer binding decriptor
    writeDescriptors[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptors[2].pNext = nullptr;
    writeDescriptors[2].dstSet = m_storageSet[m_frameID];
    writeDescriptors[2].dstBinding = BINDING_MATE;
    writeDescriptors[2].dstArrayElement = 0;
    writeDescriptors[2].descriptorCount = 1;
    writeDescriptors[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptors[2].pBufferInfo = &bufferDescriptors[BINDING_MATE];

    /// ------------------------------------------------------------------------------------
    /// Bind the light shader storage buffer
    bufferDescriptors[3].buffer = m_shaderStorageBuffers[BINDING_LIGH]->Handle();;
    bufferDescriptors[3].offset = 0;
    bufferDescriptors[3].range = VK_WHOLE_SIZE; /// access whole buffer

    /// Light buffer binding decriptor
    writeDescriptors[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptors[3].pNext = nullptr;
    writeDescriptors[3].dstSet = m_storageSet[m_frameID];
    writeDescriptors[3].dstBinding = BINDING_LIGH;
    writeDescriptors[3].dstArrayElement = 0;
    writeDescriptors[3].descriptorCount = 1;
    writeDescriptors[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptors[3].pBufferInfo = &bufferDescriptors[BINDING_LIGH];

    /// ------------------------------------------------------------------------------------
    /// Bind the joint shader storage buffer
    bufferDescriptors[4].buffer = m_shaderStorageBuffers[BINDING_JOIN]->Handle();
    bufferDescriptors[4].offset = 0;
    bufferDescriptors[4].range = VK_WHOLE_SIZE; /// access whole buffer

    /// Joint buffer binding decriptor
    writeDescriptors[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptors[4].pNext = nullptr;
    writeDescriptors[4].dstSet = m_storageSet[m_frameID];
    writeDescriptors[4].dstBinding = BINDING_JOIN;
    writeDescriptors[4].dstArrayElement = 0;
    writeDescriptors[4].descriptorCount = 1;
    writeDescriptors[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptors[4].pBufferInfo = &bufferDescriptors[BINDING_JOIN];

    vkUpdateDescriptorSets( *device, 5, writeDescriptors.Ptr(), 0, nullptr );
}

void crUniformManager::SubmitOffsets( const vkCommandbuffer* in_commandBuffer )
{
    vkCmdBindDescriptorSets( in_commandBuffer->CommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, 1, &m_storageSet[m_frameID], MAX_BINDINGS, m_dynamicOffsets.Ptr() );
}

void crUniformManager::CreateStorageBuffers(void)
{
    for ( uint32_t i = 0; i < MAX_BINDINGS; i++)
    {
        m_shaderStorageBuffers[i] = new vkBuffer();
        m_shaderStorageBuffers[i]->Create( vkBuffer::BUFFER_TYPE_SHADER, vkBuffer::BUFFER_ACCESS_WRITE, k_BUFFERS_SIZES[i] );
    }
}

void crUniformManager::CreateStorageSet(void)
{
    VkResult result = VK_SUCCESS;
    uint32_t i = 0;
    uint32_t bindingCount = MAX_BINDINGS;
    auto device = tr.GetRenderDevice();
    idStaticList<VkDescriptorBindingFlags, MAX_BINDINGS>     bindingFlagsArr;
    idStaticList<VkDescriptorSetLayoutBinding, MAX_BINDINGS> bindings;

    /// configure bindings and flags
    for ( i = 0; i < MAX_BINDINGS; i++)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = i;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // generic for now
        binding.pImmutableSamplers = nullptr;

        bindings[i] = binding;
        bindingFlagsArr[i] = 0; // Shader Storage buffers, has no active flags
    }
    
    ///
    /// VkDescriptorSetLayoutBindingFlagsCreateInfo 
    /// configure layout bind flags 
    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo{};
    flagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsCreateInfo.bindingCount = bindingFlagsArr.Num();
    flagsCreateInfo.pBindingFlags = bindingFlagsArr.Ptr();

    ///
    /// VkDescriptorSetLayoutCreateInfo
    /// Explicitly describe the interface of the resources accessed by the shader.
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &flagsCreateInfo;
    layoutInfo.bindingCount = bindings.Num();
    layoutInfo.pBindings = bindings.Ptr();
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; // enable update-after-bind pool behavior:
    result = vkCreateDescriptorSetLayout( *device, &layoutInfo, k_allocationCallbacks, &m_storageSetLayout );
    if ( result != VK_SUCCESS )
        idLib::FatalError( "vkCreateDescriptorSetLayout %s\n", VulkanErrorString( result ).c_str() );

    ///
    /// VkDescriptorPoolSize
    /// Describe which descriptor types our descriptor sets are going to contain and how many of them
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;  // storage buffers 
    poolSize.descriptorCount = MAX_BINDINGS * SMP_FRAMES; // 4 storages buffers by 3 frames   

    /// VkDescriptorPoolCreateInfo
    /// It reserves blocks of GPU resources in advance so that the allocation of descriptor 
    /// sets is fast and does not cause crashes during the rendering loop.
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = SMP_FRAMES; // probably 1 "global" set per frame
    result = vkCreateDescriptorPool( *device, &poolInfo, k_allocationCallbacks, &m_storagePool );
    if( result != VK_SUCCESS )
        idLib::FatalError( "vkCreateDescriptorPool %s\n", VulkanErrorString( result ).c_str() );

    /// 
    /// VkDescriptorSetVariableDescriptorCountAllocateInfo
    ///    
    VkDescriptorSetVariableDescriptorCountAllocateInfo  varCountAlloc{};
    varCountAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    varCountAlloc.descriptorSetCount = 1;
    varCountAlloc.pDescriptorCounts = &bindingCount;

    ///
    /// VkDescriptorSetAllocateInfo
    ///
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &varCountAlloc;
    allocInfo.descriptorPool = m_storagePool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_storageSetLayout;
    for( i = 0; i < SMP_FRAMES; i++ )
    {
        result = vkAllocateDescriptorSets( *device, &allocInfo, &m_storageSet[i] );
        if( result != VK_SUCCESS )
            idLib::FatalError( "vkAllocateDescriptorSets %s\n", VulkanErrorString( result ).c_str() );
    }
}

void crUniformManager::CreateBindlessSet(void)
{
    VkResult result = VK_SUCCESS;
    auto device = tr.GetRenderDevice();
    
    ///
    /// VkDescriptorBindingFlags
    /// configure descriptor binding flags 
    VkDescriptorBindingFlags bindingFlagsArr = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo{};
    flagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsCreateInfo.bindingCount = 1;
    flagsCreateInfo.pBindingFlags = &bindingFlagsArr;

    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0; // layout( set = 0, binding = 0 ) uniform sampler texturasGlobais[N]; 
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1; // placeholder (variable at alloc time)
    binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // can be acessed by the vertex or frament shaders
    binding.pImmutableSamplers = nullptr;

    ///
    /// VkDescriptorSetLayoutCreateInfo
    /// configure layout bindless descriptor
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &flagsCreateInfo;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; // enable update-after-bind pool behavior:
    result = vkCreateDescriptorSetLayout( *device, &layoutInfo, k_allocationCallbacks, &m_bindlessSetLayout );
    if ( result != VK_SUCCESS )
        idLib::FatalError( "vkCreateDescriptorSetLayout %s\n", VulkanErrorString( result ).c_str() );

    ///
    /// VkDescriptorPoolSize
    /// store total image sampler binding count
    VkDescriptorPoolSize    poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = MAX_BINDING_SAMPLERS; 
    
    ///
    /// VkDescriptorPoolCreateInfo
    ///
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT; /// we can update the descriptor after binding 
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1; /// A global set at begin of the render 
    result = vkCreateDescriptorPool( *device, &poolInfo, nullptr, &m_bindlessPool );
    if( result != VK_SUCCESS )
        idLib::FatalError( "vkCreateDescriptorPool %s\n", VulkanErrorString( result ).c_str() );

    ///
    /// VkDescriptorSetVariableDescriptorCountAllocateInfo
    ///
    static uint32_t descriptorCountForBinding0 = MAX_BINDING_SAMPLERS;
    VkDescriptorSetVariableDescriptorCountAllocateInfo  varCountAlloc{};    
    varCountAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    varCountAlloc.descriptorSetCount = 1;
    varCountAlloc.pDescriptorCounts = &descriptorCountForBinding0;
    
    ///
    /// VkDescriptorSetAllocateInfo
    ///
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &varCountAlloc;
    allocInfo.descriptorPool = m_bindlessPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_bindlessSetLayout;
    result = vkAllocateDescriptorSets( *device, &allocInfo, &m_bindlessSet );
    if( result != VK_SUCCESS )
        idLib::FatalError( "vkAllocateDescriptorSets %s\n", VulkanErrorString( result ).c_str() );
}
