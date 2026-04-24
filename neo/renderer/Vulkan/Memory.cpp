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
#include "Core.hpp"
#include "Memory.hpp"

/// Helper ( i just can't save the do Right now )
/// https://www.khronos.org/assets/uploads/developers/library/2018-vulkan-devday/03-Memory.pdf

/*
==============
crMemoryPage::crMemoryPage
==============
*/
crMemoryPage::crMemoryPage( void )
{
}

/*
==============
crMemoryPage::~crMemoryPage
==============
*/
crMemoryPage::~crMemoryPage( void )
{
}

/*
==============
crMemoryPage::Bind
==============
*/
void crMemoryPage::Bind( const VkBuffer in_buffer )
{
    auto result = vkBindBufferMemory( m_device, in_buffer, m_memory, static_cast<VkDeviceSize>( m_offset ) );
    if( result != VK_SUCCESS )
        idLib::FatalError("crMemoryPage::Bind vkBindBufferMemory failed %s\n", VulkanErrorString( result ).c_str() );
}

/*
==============
crMemoryPage::Bind
==============
*/
void crMemoryPage::Bind( const VkImage in_image )
{
    auto result = vkBindImageMemory( m_device, in_image, m_memory, static_cast<VkDeviceSize>( m_offset ) );
    if( result != VK_SUCCESS )
        idLib::FatalError("crMemoryPage::Bind vkBindImageMemory failed %s\n", VulkanErrorString( result ).c_str() );
}

/*
==============
crMemoryPage::Map
==============
*/
void*   crMemoryPage::Map(void) const
{
    void* pointer = nullptr;

    /// just get memory pointer
    auto result = vkMapMemory( m_device, m_memory,  static_cast<VkDeviceSize>( m_offset ),  static_cast<VkDeviceSize>( m_size ), 0, &pointer );
    if( result != VK_SUCCESS )
        idLib::FatalError("crMemoryPage::Map vkMapMemory failed %s\n", VulkanErrorString( result ).c_str() );

    return pointer;
}

/*
==============
crMemoryPage::Flush
==============
*/
void crMemoryPage::Flush( const uintptr_t in_offset, const size_t in_size ) const
{   
    VkMappedMemoryRange memoryRange{};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.pNext = nullptr;
    memoryRange.memory = m_memory;
    memoryRange.offset = static_cast<VkDeviceSize>( std::min( m_size, in_size ) );
    memoryRange.size = static_cast<VkDeviceSize>( std::max( m_offset, in_offset ) );
    VkResult result = vkFlushMappedMemoryRanges( m_device, 1, &memoryRange );
    if( result != VK_SUCCESS )
        idLib::Error("crMemoryPage::Flush vkFlushMappedMemoryRanges failed %s\n", VulkanErrorString( result ).c_str() );
}

crMemoryPage::crMemoryPage(const size_t in_size, const size_t in_alignment, const uintptr_t in_offset, VkDeviceMemory in_memory, VkDevice in_device) :
    m_size( in_size ),
    m_alignment( in_alignment ),
    m_offset( in_offset ),
    m_memory( in_memory ),
    m_device( in_device )
{
    m_device = *tr.GetRenderDevice();
}

/*
==============
crMemoryPool::crMemoryPool
==============
*/
crMemoryPool::crMemoryPool( void ) :
    m_index( UINT32_MAX ),
    m_type( 0 ),
    m_size( 0 ),
    m_alignment( 0 ),
    m_offsets( 0 ),
    m_memory( nullptr ),
    m_device( nullptr )
{
}

/*
==============
crMemoryPool::~crMemoryPool
==============
*/
crMemoryPool::~crMemoryPool( void )
{
    Destroy();
}

/*
==============
crMemoryPool::AllocPage
==============
*/
crMemoryPage *crMemoryPool::AllocPage( const size_t in_size, const uintptr_t in_alignment )
{
    size_t      align = std::max( in_alignment, m_alignment );
    size_t      size = __align( in_size, align );
       
    memoryBlock_t outBlock{};
    for ( uint32_t i = 0; i < m_freeBlocks.Num(); i++ )
    {
        auto& block = m_freeBlocks[i];

        size_t alignedOffset = __align( block.offset, align );
        size_t padding = alignedOffset - block.offset;

        if ( block.size < size + padding)
            continue;

        // encontrou espaço
        outBlock.offset = alignedOffset;
        outBlock.size = size;

        // ajustar bloco livre
        size_t remaining = block.size - ( size + padding );

        if (padding > 0)
            block.size = padding;
        else
            m_freeBlocks.RemoveIndex( i );

        if (remaining > 0)
            m_freeBlocks.Append( { alignedOffset + size, remaining } );

        m_usedBlocks.Append( outBlock );
        
        break;
    }

    ///
    crMemoryPage* page = new crMemoryPage( outBlock.size, align, outBlock.offset, m_memory, m_device );
    m_pages.Append( page );
    return page;
}

/*
==============
crMemoryPool::DeallocPage
==============
*/
void crMemoryPool::DeallocPage( crMemoryPage *in_page )
{
    uint32_t i = 0, j = 0, k = 0;
    for ( i = 0; i < m_usedBlocks.Num(); i++)
    {
        auto& block = m_freeBlocks[i];
        if( block.offset == in_page->Offset() )
        {
            m_freeBlocks.Append( block );
            
            // merge blocos (coalescing)
            for ( k = 0; k < m_freeBlocks.Num(); k++)
            {
                for ( j = i + 1; j < m_freeBlocks.Num(); j++)
                {
                    auto& a = m_freeBlocks[i];
                    auto& b = m_freeBlocks[j];

                    if (a.offset + a.size == b.offset)
                    {
                        a.size += b.size;
                        m_freeBlocks.RemoveIndex( j );
                        j--;
                    }
                    else if (b.offset + b.size == a.offset)
                    {
                        b.size += a.size;
                        m_freeBlocks.RemoveIndex( k );
                        i--;
                        break;
                    }
                }
            }
        }
    }

    if ( in_page )
    {
        m_pages.Remove( in_page );
        delete in_page;
    }
}

/*
==============
crMemoryPool::Create
==============
*/
bool crMemoryPool::Create( const size_t in_size, const size_t in_alignment, const uint32_t in_filter )
{
    auto device = tr.GetRenderDevice();
    m_device = *device;
    m_size = in_size;
    m_alignment = in_alignment;
    
    ///
    VkMemoryAllocateInfo allocateInfo; 
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = in_size;
    allocateInfo.memoryTypeIndex = m_index;
    auto result = vkAllocateMemory( m_device, &allocateInfo, k_allocationCallbacks, &m_memory );
    if( result != VK_SUCCESS )
    {
        idLib::Error("crMemoryPool::Create vkAllocateMemory failed %s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    // bloco livre inicial = tudo
    m_freeBlocks.Append( { 0, in_size } );

    return true;
}

/*
==============
crMemoryPool::Destroy
==============
*/
void crMemoryPool::Destroy(void)
{
    if( m_memory != nullptr )
    {
        /// Release all mapped pages 
        vkUnmapMemory( m_device, m_memory );
        vkFreeMemory( m_device, m_memory, k_allocationCallbacks );
        m_memory = nullptr;
    }
}
