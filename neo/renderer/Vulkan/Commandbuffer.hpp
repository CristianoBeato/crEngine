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

#ifndef __VK_COMMAND_BUFFER_HPP__
#define __VK_COMMAND_BUFFER_HPP__

typedef class vkDeviceQueue* vkDeviceQueuep;
class vkCommandbuffer
{
public:
    vkCommandbuffer( void );
    ~vkCommandbuffer( void );
    bool    Create( void );
    void    Destroy( void );
    void                        Begin( void );
    void                        Submit( const crSemaphore* in_imageAvailable, const crSemaphore* in_renderDone, const crFence* in_frameFence );
    ID_INLINE void              SwapFrame( void ) { m_frameID = ( m_frameID + 1 ) % m_frameCount; }
    ID_INLINE VkCommandBuffer   CommandBuffer( void ) const { return m_commandBuffers[m_frameID]; }
    ID_INLINE operator VkCommandBuffer( void ) const { return m_commandBuffers[m_frameID]; }

private:
    uint16_t        m_frameID;
    uint16_t        m_frameCount;
    vkDeviceQueuep  m_graphicQueue;
    VkCommandBuffer m_commandBuffers[MAX_SMP_FRAMES];
};

typedef vkCommandbuffer* vkCommandbufferp;

#endif //!__VK_COMMAND_BUFFER_HPP__