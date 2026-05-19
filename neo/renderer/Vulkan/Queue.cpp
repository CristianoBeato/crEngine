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
#include "Core.hpp"
#include "Queue.hpp"


/*
==============
crQueue::crQueue
==============
*/
crQueue::crQueue( const uint32_t in_family, const uint32_t in_index ) : 
    m_index( in_index ),
    m_family( in_family ),
    m_queue( nullptr ),
    m_commandPool( nullptr ),
    m_device( nullptr )
{
}

/*
==============
crQueue::~crQueue
==============
*/
crQueue::~crQueue( void )
{
    if ( m_semaphore )
    {
        vkDestroySemaphore( m_device, m_semaphore, k_allocationCallbacks );
        m_semaphore = nullptr;
    }

    if ( m_commandPool != nullptr )
    {
        vkDestroyCommandPool( m_device, m_commandPool, k_allocationCallbacks );
        m_commandPool = nullptr;
    }

    m_queue = nullptr;
    m_device = nullptr;
    m_family = 0;
    m_index = 0;
}

/*
==============
crQueue::Init
==============
*/
bool crQueue::Init( const VkDevice in_device )
{
    m_device = in_device;

    ///
    ///
    /// get device queue
    VkDeviceQueueInfo2 queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
    queueInfo.pNext = nullptr;
    queueInfo.flags = 0;
    queueInfo.queueFamilyIndex = m_family;
    queueInfo.queueIndex = m_index;
    vkGetDeviceQueue2( m_device, &queueInfo, &m_queue );

    ///
    ///
    /// Create queue command pool
    VkCommandPoolCreateInfo commandPoolCI{};
    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCI.pNext = nullptr;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //
    commandPoolCI.queueFamilyIndex = m_family;
    auto result = vkCreateCommandPool( m_device, &commandPoolCI, k_allocationCallbacks, &m_commandPool );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkCreateCommandPool failed! %s\n", VulkanErrorString( result).c_str() );
        return false;
    }
    
    return true;    
}

/*
==============
crQueue::ResetPool
==============
*/
void crQueue::ResetPool(void) const
{
    auto result = vkResetCommandPool( m_device, m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT );
    if( result != VK_SUCCESS )
        idLib::Error( VulkanErrorString( result ).c_str() );
}
