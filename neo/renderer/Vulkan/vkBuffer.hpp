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

#ifndef __VK_BUFFER_HPP__
#define __VK_BUFFER_HPP__

class vkBuffer : public crBuffer
{
public:
    struct bufferState_t
    {
        VkBufferUsageFlags      usage;
        VkPipelineStageFlags2   stage;
        VkAccessFlags2          access;
        uint32_t                queueFamily;

        inline bool operator == ( const bufferState_t &ref ) const
        {
            return ( ( usage == ref.usage ) && ( stage != ref.stage ) && ( access != ref.access ) && ( queueFamily == ref.queueFamily ) );            
        }
    };

    vkBuffer( void );
    ~vkBuffer( void );

    virtual bool    Create( const type_t in_type, const access_t in_acess, const size_t in_size );
    virtual bool    Resize( const size_t in_newSize );
    virtual void    Destroy( void );
    virtual void    CopyBuffer( const crBuffer* in_source, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset, const size_t in_size ) const;
    virtual void    Flush( const uintptr_t in_offset, const size_t in_size ) const;
    virtual void*   Handle( void ) const;
    virtual void    StateTransition( const state_t in_state, const crCommandBuffer* in_commandBuffer );
    void            SetState( const bufferState_t &in_state, const VkCommandBuffer in_commandBuffer );

    VkBuffer        Buffer( void ) const { return m_bufferHost; }

private:
    uint32_t                m_family;         // buffer current queue
    bufferState_t           m_bestate;
    VkMemoryPropertyFlags   m_property;       // memory properties 
    VkCommandBuffer         m_copyCmd;        // auxiliar command buffer, to store state transitins and copy commands 
    VkBuffer                m_bufferHost;     // buffer host side handler 
    VkBuffer                m_bufferClient;   // buffer client side handler 
    VkDeviceMemory          m_memoryHost;     // host side memory storage
    VkDeviceMemory          m_memoryClient;   // client side memory storage
    VkDevice                m_device;         // device handler
};

#endif //__VK_BUFFER_HPP__