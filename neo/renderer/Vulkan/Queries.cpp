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
#include "Queries.hpp"

vkTimeQueries::vkTimeQueries( void )
{
}

vkTimeQueries::~vkTimeQueries( void )
{
}

void vkTimeQueries::Create( void )
{
    crVulkanRenderDevicep device = tr.GetRenderDevice();

    m_timestampPeriod = device->Properties().timestampPeriod;

    VkQueryPoolCreateInfo queryPoolCI{};
    queryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCI.pNext = nullptr;
    queryPoolCI.flags = 0;
    queryPoolCI.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCI.queryCount = 2; // time begin, time end
    queryPoolCI.pipelineStatistics = 0;

    for ( uint32_t i = 0; i < SMP_FRAMES; ++i)
    {
        vkCreateQueryPool( *device, &queryPoolCI, k_allocationCallbacks, &m_pools[i] );
        vkResetQueryPool( *device, m_pools[i], 0, 2 );      // Reset all queries
    }
}

void vkTimeQueries::Destroy(void)
{
    crVulkanRenderDevicep device = tr.GetRenderDevice();
    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        vkDestroyQueryPool( *device, m_pools[i], k_allocationCallbacks);
        m_pools[i] = nullptr;
    }
}

void vkTimeQueries::BeginRegister( const vkCommandbuffer* in_cmd )
{
    vkCmdWriteTimestamp( *in_cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_pools[m_bufferID], 0 );
}

void vkTimeQueries::EndRegister( const vkCommandbuffer* in_cmd )
{
    vkCmdWriteTimestamp( *in_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_pools[m_bufferID], 1 ); 
}

uint64_t vkTimeQueries::Retrieve( void )
{
    uint64_t results[2] { 0, 0 };
    crVulkanRenderDevicep device = tr.GetRenderDevice();
    // Reset and get results - we should be sure that the GPU is done at this point, for example by waiting on a semaphore
    // VK_QUERY_RESULT_64_BIT to get 64 bit timestamps
    vkGetQueryPoolResults( *device, m_pools[m_bufferID], 0, 2, sizeof(uint64_t) * 2, results, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT );
    vkResetQueryPool( *device, m_pools[m_bufferID], 0, 2 );
    m_bufferID = ( m_bufferID + 1 ) % SMP_FRAMES;

    /// calc microseconds
    return ( results[1] - results[0] ) * m_timestampPeriod;
}
