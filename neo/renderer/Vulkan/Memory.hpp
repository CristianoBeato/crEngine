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

#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

/// @brief Suballocate from the pool
class crBuffer;
class crTexture;
class crMemoryPool;
class crMemoryPage
{
public:
    crMemoryPage( void );
    ~crMemoryPage( void );

    /// @brief Bind the memory page to buffer handle 
    /// @param in_buffer buffer bind 
    void                        Bind( const VkBuffer in_buffer );

    /// @brief Bind the memory page to image handle 
    /// @param in_image 
    void                        Bind( const VkImage in_image );

    /// @brief Retrieve a host virtual address pointer to a region of a 
    /// mappable memory ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT Only ) object
    /// @return if succes a virtual pointer to memory
    void*                       Map( void ) const;

    /// @brief Flush memory page operations ( for non VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
    /// guarantees that host writes to the memory are made available to the host memory domain.
    void                        Flush( void ) const;

    ID_INLINE size_t            Alignment( void ) const { return m_alignment; }
    ID_INLINE uintptr_t         Offset( void ) const { return m_offset; }
    ID_INLINE size_t            Size( void ) const { return m_size; }
    ID_INLINE VkDeviceMemory    Memory( void ) const { return m_memory; }

protected:
    friend class crMemoryPool;
    crMemoryPage( const size_t in_size, const size_t in_alignment, const uintptr_t in_offset, VkDeviceMemory in_memory, VkDevice in_device );

private:
    size_t          m_alignment;
    size_t          m_size;
    uintptr_t       m_offset;
    VkDeviceMemory  m_memory;
    VkDevice        m_device;

protected:
    crMemoryPage( const size_t in_size, const size_t in_alignment, const uintptr_t in_offset, const VkDeviceMemory  in_memory );
};

/// @brief Allocate a Device memory pool ( to use whit multiples images or buffer)
class crMemoryPool
{
public:
    struct memoryBlock_t 
    {
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    crMemoryPool( void );
    ~crMemoryPool( void );
    crMemoryPage*   AllocPage( const size_t in_size, const uintptr_t in_offset );
    void            DeallocPage( crMemoryPage* in_page );
    ID_INLINE size_t  Size( void ) const { return m_size; }
    crMemoryPage*   Alloc( const size_t in_size, const size_t in_alignment );
    void            Free( crMemoryPage* in_page );

protected:
    friend class crMemoryHeap;
    bool            Create( const size_t in_size, const size_t in_alignment, const uint32_t in_filter );
    void            Destroy( void );
    void            SetProperties( const uint32_t in_index, const uint32_t in_type );
    
    uint32_t        GetIndex( void ) const { return m_index; }
    uint32_t        GetType( void ) const { return m_type; }

private:
    uint32_t                            m_index;
    uint32_t                            m_type;
    size_t                              m_size;
    size_t                              m_alignment;
    uintptr_t                           m_offsets;
    VkDeviceMemory                      m_memory;
    VkDevice                            m_device;
    idList<crMemoryPage*, TAG_VULKAN>   m_freepages;
    idList<crMemoryPage*, TAG_VULKAN>   m_usedpages
};
class crMemoryPool* crMemoryPoolp;

class crMemoryHeap
{
public:
    struct memoryHeapInfo_t
    {
        size_t                  total = 0;  // total available heap
        size_t                  allocated = 0; // total used
        size_t                  free = 0;
        VkMemoryPropertyFlags   propertyFlags = 0;
    };

    struct memoryTypeInfo_t
    {
        uint32_t                            typeIndex = 0;
        uint32_t                            heapIndex = 0;
        VkMemoryPropertyFlags               propertyFlags = 0;
        idList<crMemoryPool*, TAG_VULKAN>   pools;
    };

    crMemoryHeap( void );
    ~crMemoryHeap( void );
    bool            Create( void );
    void            Destroy( void );
    /// @brief Allocate a memory page to be used by structures with the same configuration.
    crMemoryPool*   Alloc( const size_t in_size, const size_t in_alignament, const uint32_t in_filter, const VkMemoryPropertyFlags in_properties );
    /// @brief Releasse a memory block
    void            Free( crMemoryPool* in_pool );
    void            Defrag( void );

private:
    idList<memoryHeapInfo_t, TAG_VULKAN>    m_heaps;
    idList<memoryTypeInfo_t, TAG_VULKAN>    m_types;
};
typedef crMemoryHeap* crMemoryHeapp;

#endif //!__MEMORY_HPP__