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

#include "Core.hpp"
#include "Syncs.hpp"

crFence::crFence( void ) : 
    m_frameID( 0 ),
    m_frameCount( 0 ),
    m_fences( nullptr ),
    m_device( nullptr )
{
}

crFence::~crFence( void )
{
    Destroy();
}

bool crFence::Create( const uint16_t in_frameCount, const bool in_signaled )
{
    auto device = tr.GetRenderDevice();
    m_device = *device;
    m_frameCount = in_frameCount;
    m_fences = static_cast<VkFence*>( Mem_Alloc( sizeof( VkFence ) * m_frameCount, TAG_VULKAN ) );

    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = in_signaled ? VK_FENCE_CREATE_SIGNALED_BIT: 0; /// create a fence already signaled

    for ( uint32_t i = 0; i < m_frameCount; i++)
    {
        auto result = vkCreateFence( m_device, &fenceCI, k_allocationCallbacks, &m_fences[i] );
        if( result != VK_SUCCESS )
        {
            idLib::Error( "crFence::Create::vkCreateFence Failed %s\n", VulkanErrorString( result ) );
            return false;
        }
    }
    
    return true;
}

void crFence::Destroy(void)
{
    if ( m_fences != nullptr )
    {
        ///
        vkResetFences( m_device, m_frameCount, m_fences );
        for ( uint32_t i = 0; i < m_frameCount; i++)
        {
            if ( m_fences[i] == nullptr )
                continue; 

            vkDestroyFence( m_device, m_fences[i], k_allocationCallbacks );
        }
        
        Mem_Free( m_fences );
        m_fences = nullptr;
    }
    
    m_frameCount = 0;
    m_frameID = 0;
    m_device = nullptr;
}

void crFence::Reset( void ) const
{
    VkResult result = vkResetFences( m_device, 1, &m_fences[m_frameID] );
    if ( result != VK_SUCCESS )
        idLib::Warning( "crFence::Reset Failed! %s\n", VulkanErrorString( result ) );
}

VkResult crFence::Wait( const uint64_t in_timeout ) const
{
    return vkWaitForFences( m_device, 1, &m_fences[m_frameID], VK_TRUE, in_timeout );
}

VkResult crFence::Status( void ) const
{
    return vkGetFenceStatus( m_device, m_fences[m_frameID] );
}

/*
==========================
crSemaphoreRoundRobin::crSemaphoreRoundRobin
==========================
*/
crSemaphoreRoundRobin::crSemaphoreRoundRobin( void ) :
    m_frameID( 0 ),
    m_frameCount( 0 ),
    m_semaphores( nullptr ),
    m_device( nullptr )
{
}

crSemaphoreRoundRobin::~crSemaphoreRoundRobin( void )
{
    Destroy();
}

bool crSemaphoreRoundRobin::Create( const uint16_t in_frameCount )
{
    auto device = tr.GetRenderDevice();
    m_device = *device;
    m_frameCount = in_frameCount;
    m_semaphores = static_cast<VkSemaphore*>( Mem_Alloc( sizeof( VkSemaphore ) * m_frameCount, TAG_VULKAN ) );

    VkSemaphoreCreateInfo semaphoreCI{};
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = nullptr;
    semaphoreCI.flags = 0;

    for ( uint32_t i = 0; i < m_frameCount; i++ )
    {
        auto result = vkCreateSemaphore( m_device, &semaphoreCI, k_allocationCallbacks, &m_semaphores[i] );
        if ( result != VK_SUCCESS )
        {
            idLib::Warning( "crFence::Reset Failed! %s\n", VulkanErrorString( result ) );
            return false;
        }
    }
    
    return true;
}

void crSemaphoreRoundRobin::Destroy(void)
{
    if( m_semaphores != nullptr )
    {
        for ( uint32_t i = 0; i < m_frameCount; i++)
        {
            vkDestroySemaphore( m_device, m_semaphores[i], k_allocationCallbacks );
            m_semaphores[i] = nullptr;
        }

        Mem_Free( m_semaphores );
        m_semaphores = nullptr;
    }

    m_frameID = 0;
    m_frameCount = 0;
    m_device = 0;
}

void crSemaphoreRoundRobin::Signal(void) const
{
    VkSemaphoreSignalInfo semaphoreSignal{};
    semaphoreSignal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    semaphoreSignal.pNext = nullptr;
    semaphoreSignal.semaphore = m_semaphores[m_frameID];
    semaphoreSignal.value = 0;
    auto result = vkSignalSemaphore( m_device, &semaphoreSignal );
    if ( result != VK_SUCCESS )
        idLib::Warning( "crSemaphoreRoundRobin::Signal Failed! %s\n", VulkanErrorString( result ) );
}

VkResult crSemaphoreRoundRobin::Wait( const uint64_t in_timeout ) const
{
    VkSemaphoreWaitInfo semaphoreWait{};
    semaphoreWait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    semaphoreWait.pNext = nullptr;
    semaphoreWait.flags = 0;
    semaphoreWait.semaphoreCount = 1;
    semaphoreWait.pSemaphores = &m_semaphores[m_frameID];
    semaphoreWait.pValues = 0;
    return vkWaitSemaphores( m_device, &semaphoreWait, in_timeout );
}

VkSemaphoreSubmitInfo crSemaphoreRoundRobin::SubmitInfo(void)
{
    VkSemaphoreSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    submit.pNext = nullptr;
    submit.semaphore = m_semaphores[m_frameID];
    submit.value = 0;
    submit.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    submit.deviceIndex = 0; // TODO get it 
    return submit;
}

crSemaphoreRoundRobin::operator VkSemaphoreSubmitInfo(void) const
{
    VkSemaphoreSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    submit.pNext = nullptr;
    submit.semaphore = m_semaphores[m_frameID];
    submit.value = 0;
    submit.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    submit.deviceIndex = 0; // TODO get it 
    return submit;
}

/*
==========================
crSemaphoreTimeline::crSemaphoreTimeline
==========================
*/
crSemaphoreTimeline::crSemaphoreTimeline( void ) : 
    m_timeline( 0 ),
    m_semaphore( nullptr )
{
}

crSemaphoreTimeline::~crSemaphoreTimeline( void )
{
}

bool crSemaphoreTimeline::Create(void)
{
    auto device = tr.GetRenderDevice();
    m_device = *device;

    VkSemaphoreTypeCreateInfo typeCreateInfo{};
    typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeCreateInfo.initialValue = 0; // initial value

    VkSemaphoreCreateInfo semaphoreCI{};
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = &typeCreateInfo;
    auto result = vkCreateSemaphore( m_device, &semaphoreCI, k_allocationCallbacks, &m_semaphore );
    if ( result != VK_SUCCESS )
    {
        idLib::Warning( "crFence::Reset Failed! %s\n", VulkanErrorString( result ) );
        return false;
    }

    return true;
}

void crSemaphoreTimeline::Destroy(void)
{
    if ( m_semaphore != nullptr )
    {
        vkDestroySemaphore( m_device, m_semaphore, k_allocationCallbacks );
        m_semaphore = nullptr;
    }
    
    m_device = nullptr; 
    m_timeline = 0;    
}

void crSemaphoreTimeline::Signal(const uint64_t in_value) const
{
    VkSemaphoreSignalInfo semaphoreSignal{};
    semaphoreSignal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    semaphoreSignal.pNext = nullptr;
    semaphoreSignal.semaphore = m_semaphore;
    semaphoreSignal.value = in_value;
    auto result = vkSignalSemaphore( m_device, &semaphoreSignal );
    if ( result != VK_SUCCESS )
        idLib::Warning( "crSemaphoreRoundRobin::Signal Failed! %s\n", VulkanErrorString( result ) );
}

VkResult crSemaphoreTimeline::Wait(const uint64_t in_value, const uint64_t in_timeout) const
{
    VkSemaphoreWaitInfo semaphoreWait{};
    semaphoreWait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    semaphoreWait.pNext = nullptr;
    semaphoreWait.flags = 0;
    semaphoreWait.semaphoreCount = 1;
    semaphoreWait.pSemaphores = &m_semaphore;
    semaphoreWait.pValues = &in_value;
    return vkWaitSemaphores( m_device, &semaphoreWait, in_timeout );
}
