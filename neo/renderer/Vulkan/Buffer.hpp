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

#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

/// @brief Base buffer class
typedef class crBuffer
{
public:
    enum type_t : uint8_t
    {
        BUFFER_TYPE_UNDEFINED,  // unknow buffer
        BUFFER_TYPE_INDEX,      // primitive index buffer 
        BUFFER_TYPE_VERTEX,     // vertex buffer
        BUFFER_TYPE_COMMANDS,   // indirec draw comand buffer
        BUFFER_TYPE_SHADER,     // shader storage data
        BUFFER_TYPE_SOURCE,     // staging buffer source
        BUFFER_TYPE_DESTINATION // staging buffer destination
    };

    struct state_t 
    {
        uint32_t                queueFamily = 0;
        VkPipelineStageFlags2   stage = 0;
        VkAccessFlags2          access = 0;

        inline bool operator == ( const state_t & in_state )
        {
            return ( in_state.queueFamily == queueFamily ) && ( in_state.stage == stage ) && ( in_state.access == access );
        }
    };

    crBuffer( void );
    ~crBuffer( void );

    /// @brief Create a fixed size buffer storage
    /// @param in_type  the buffer storage usage type 
    /// @param in_acess buffer acess for read or write 
    /// @param in_size buffer size
    /// @return true on sucess 
    virtual bool    Create( const type_t in_type, const size_t in_size );
        
    virtual bool    Storage( crMemoryPool* in_bufferPool );

    /// @brief Destroy the buffer releasing the map and the memory 
    virtual void    Destroy( void );

    /// @brief Insert a barrier and perform a state change of buffer and/or transfer the buffer
    /// queue family owership.   
    /// @param in_commandBuffer command buffer  
    /// @param in_newState 
    void            SetState( const crCommandBufferp in_commandBuffer, const state_t in_newState );

    /// @brief Flush buffer data ( Make visible to device/CPU, or copy source to destination device )
    /// @param in_offset begin offset to be flushed 
    /// @param in_size size to be flushed 
    virtual void    Flush( const uintptr_t in_offset, const size_t in_size ) const;

    /// @brief Send data to buffer 
    /// @param in_data pointer to data to be uploaded.
    /// @param in_offset offset in buffer to be copied.
    /// @param in_size size of the data to be copied;
    virtual void    Upload( const void* in_data, const uintptr_t in_offset, const size_t in_size ) const {};
 
    /// @brief Retrieve the data from the buffer 
    /// @param in_data pointer to destination to be copied 
    /// @param in_offset offset in buffer to be copied.
    /// @param in_size size of the data to be copied;
    virtual void    Download( void* in_data, const uintptr_t in_offset, const size_t in_size ) const {};

    /// @brief buffer allocated size. 
    ID_INLINE size_t    Size( void ) { return m_page->Size(); }

    /// @brief return the memory requeriments of the buffer
    ID_INLINE VkMemoryRequirements  MemoryRequirements( void ) const { return m_memoryRequirements; }

    ID_INLINE crMemoryPagep         GetMemoryPage( void ) const { return m_page; }

    /// @brief access buffer handle object
    /// @return nullptr if object has destroyed or failed to created, 
    /// or the native buffer handle
    ID_INLINE VkBuffer  Handle( void ) const { return m_buffer; }
    
    /// @brief easy access to handle 
    ID_INLINE  operator VkBuffer( void ) const { return m_buffer; }
    
private:
    type_t                  m_type;     // Buffer type
    VkBufferUsageFlags      m_usage;      // current vulkan buffer usage
    state_t                 m_state;
    VkBuffer                m_buffer;   // buffer host side handler 
    VkMemoryRequirements    m_memoryRequirements;
    crMemoryPage*           m_page;
}* crBufferp;

#endif //!__VK_BUFFER_HPP__