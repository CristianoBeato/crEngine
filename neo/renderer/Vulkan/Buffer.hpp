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

/// crBuffer
/// @brief base class abstraction for common graphic buffer storage
///
class vkBuffer : public vkResourceState
{
public:
    enum access_t : uint8_t
    {
        BUFFER_ACCESS_NONE,
        BUFFER_ACCESS_WRITE,
        BUFFER_ACCESS_READ
    };

    enum type_t : uint8_t
    {
        BUFFER_TYPE_UNDEFINED,  // unknow buffer
        BUFFER_TYPE_INDEX,      // primitive index buffer 
        BUFFER_TYPE_VERTEX,     // vertex buffer
        BUFFER_TYPE_SHADER,     // shader storage data
        BUFFER_TYPE_COMMANDS,   // indirec draw comand buffer
        BUFFER_TYPE_PIXEL       // pixel storage buffer
    };

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

    /// @brief create a fixed size buffer storage, data will remain maped till buffer are Destroyeds
    /// @param in_type  the buffer storage usage type 
    /// @param in_acess buffer acess for read or write 
    /// @param in_size buffer size
    /// @return true on sucess 
    bool    Create( const type_t in_type, const access_t in_acess, const size_t in_size );
    
    /// @brief try recreate the buffer and copy old content from old buffer
    /// @param in_newSize new buffer size 
    /// @return true on sucess
    bool    Resize( const size_t in_newSize );
    
    /// @brief Destroy the buffer releasing the map and the memory 
    void    Destroy( void );
        
    /// @brief flush buffer data ( send to device )
    /// @param in_offset 
    /// @param in_size 
    void    Flush( const uintptr_t in_offset, const size_t in_size ) const;
    
    /// @brief Send data to buffer, perform a cpu copy of the data to the dst buffer 
    /// @param in_data 
    /// @param in_offset 
    /// @param in_size 
    void    Upload( const void* in_data, const uintptr_t in_offset, const size_t in_size ) const;
    
    /// @brief 
    /// @param in_data 
    /// @param in_offset 
    /// @param in_size 
    void    Download( void* in_data, const uintptr_t in_offset, const size_t in_size ) const;
    
    virtual void    StateTransition( const state_t in_state ) override;
    
    /// @brief 
    /// @param  
    /// @return 
    VkBuffer    Handle( void ) const { return m_bufferHost; }
    
    type_t      Type( void ) const { return m_type; }
    size_t      Size( void ) const { return m_size; }
    void*       Map( void ) const { return m_data; }
    
protected:
    uint32_t                m_family;       // buffer current queue
    type_t                  m_type;         // type of buffer data storage
    access_t                m_access;       // buffer access type
    size_t                  m_size;         // bufer whole size
    bufferState_t           m_bestate;      //
    VkMemoryPropertyFlags   m_property;     // memory properties 
    VkCommandBuffer         m_copyCmd;      // auxiliar command buffer, to store state transitins and copy commands 
    VkBuffer                m_bufferHost;   // buffer host side handler 
    VkBuffer                m_bufferClient; // buffer client side handler 
    VkDeviceMemory          m_memoryHost;   // host side memory storage
    VkDeviceMemory          m_memoryClient; // client side memory storage
    void*                   m_data;         // pointer from buffer mapped data
    void    SetState( const bufferState_t &in_state );
    
};

#endif //!__VK_BUFFER_HPP__