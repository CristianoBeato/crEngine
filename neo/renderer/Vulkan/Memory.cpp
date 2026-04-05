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
void crMemoryPage::Bind( const crBuffer *in_buffer )
{
    VkBuffer buffer = *in_buffer;
    auto result = vkBindBufferMemory( m_device, buffer, m_memory, static_cast<VkDeviceSize>( m_offset ) );
    if( result != VK_SUCCESS )
        idLib::FatalError("crMemoryPage::Bind vkBindBufferMemory failed %s\n", VulkanErrorString( result ) );
}

/*
==============
crMemoryPage::Bind
==============
*/
void crMemoryPage::Bind( const crTexture *in_texture )
{
    VkImage image = *in_texture;
    auto result = vkBindImageMemory( m_device, image, m_memory, static_cast<VkDeviceSize>( m_offset ) );
    if( result != VK_SUCCESS )
        idLib::FatalError("crMemoryPage::Bind vkBindImageMemory failed %s\n", VulkanErrorString( result ) );
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
        idLib::FatalError("crMemoryPage::Map vkMapMemory failed %s\n", VulkanErrorString( result ) );

    return pointer;
}

/*
==============
crMemoryPage::Flush
==============
*/
void crMemoryPage::Flush( const size_t in_size, const uintptr_t in_offset ) const
{
    VkMappedMemoryRange memoryRange{};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.pNext = nullptr;
    memoryRange.memory = m_memory;
    memoryRange.offset = static_cast<VkDeviceSize>( m_offset + in_offset );
    memoryRange.size = static_cast<VkDeviceSize>( in_size );
    VkResult result = vkFlushMappedMemoryRanges( m_device, 1, &memoryRange );
    if( result != VK_SUCCESS )
        idLib::Error("crMemoryPage::Flush vkFlushMappedMemoryRanges failed %s\n", VulkanErrorString( result ) );
}

crMemoryPage::crMemoryPage(const size_t in_size, const size_t in_alignment, const uintptr_t in_offset, VkDeviceMemory in_memory, VkDevice in_device ) :
    m_size( in_size ),
    m_alignment( in_alignment ),
    m_offset( in_offset ),
    m_memory( in_memory ),
    m_device( in_device )
{
}

/*
==============
crMemoryPool::crMemoryPool
==============
*/
crMemoryPool::crMemoryPool( void ) : m_size( 0 )
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
        idLib::Error("crMemoryPool::Create vkAllocateMemory failed %s\n", VulkanErrorString( result ) );
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
        vkFreeMemory( m_device, m_memory, k_allocationCallbacks );
        m_memory = nullptr;
    }
}

/*
==============
crMemoryPool::SetProperties
==============
*/
void crMemoryPool::SetProperties(const uint32_t in_index, const uint32_t in_type)
{
    m_index = in_index;
    m_type = in_type;
}

/*
==============
crMemoryHeap::Map
==============
*/
void *crMemoryPool::Map(void)
{
    vkMapMemory( m_device, m_memory, 0, VK_WHOLE_SIZE, 0, &m_mapped );
    return m_mapped;
}

/*
==============
crMemoryHeap::Unmap
==============
*/
void crMemoryPool::Unmap( void )
{
    vkUnmapMemory( m_device, m_memory );
    m_mapped = nullptr;
}

/*
==============
crMemoryHeap::crMemoryHeap
==============
*/
crMemoryHeap::crMemoryHeap( void )
{
}

/*
==============
crMemoryHeap::~crMemoryHeap
==============
*/
crMemoryHeap::~crMemoryHeap( void )
{
}

/*
==============
crMemoryHeap::Create
==============
*/
bool crMemoryHeap::Create( void )
{
    uint32_t i = 0;
    auto device = tr.GetRenderDevice();

    // query device memory properties
    VkPhysicalDeviceMemoryProperties2 properties{};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    properties.pNext = nullptr;
    vkGetPhysicalDeviceMemoryProperties2( *device, &properties );

    auto props = properties.memoryProperties;
    if( props.memoryHeapCount < 1 || props.memoryTypeCount < 1 )
        return false;
    
    /// In the Vulkan ecosystem, the term "heap" refers to large, 
    /// contiguous blocks of physical memory available in the hardware 
    /// (such as the video card's VRAM or system RAM) that the 
    /// API manages to store resource data such as buffers and images.
    /// Memory Heaps (Device Memory)
    /// These represent the actual physical memory installed in 
    /// the computer or mobile device. When you query the device 
    /// memory properties (VkPhysicalDeviceMemoryProperties), 
    /// Vulkan returns a list of available heaps:

    /// VK_DEVICE_LOCAL_BIT: Indicates memory that is physically located on the GPU. 
    /// It is the fastest memory for graphics processing, 
    /// but is generally not directly accessible by the CPU.

    /// VK_HOST_VISIBLE_BIT: Usually associated with system RAM 
    /// or a portion of VRAM that the CPU can "see" and write to directly.
    
    /// get memory heap
    m_heaps.Resize( props.memoryHeapCount );
    for ( i = 0; i < props.memoryHeapCount; i++)
    {
        /// aquire heap size
        m_heaps[i].total = props.memoryHeaps[i].size;
        m_heaps[i].free = m_heaps[i].total;
        m_heaps[i].propertyFlags = props.memoryHeaps[i].flags;
    }
    
    /// get types
    m_types.Resize( props.memoryTypeCount );
    for ( i = 0; i < props.memoryTypeCount; i++)
    {
        m_types[i].typeIndex = i;
        m_types[i].heapIndex = props.memoryTypes[i].heapIndex;
        m_types[i].propertyFlags = props.memoryTypes[i].propertyFlags;
    }
    
    return true;
}

/*
==============
crMemoryHeap::Alloc
==============
*/
crMemoryPool* crMemoryHeap::Alloc( const size_t in_size, const size_t in_alignment, const uint32_t in_filter, const VkMemoryPropertyFlags in_properties )
{
    memoryTypeInfo_t*   type = nullptr; 
    crMemoryPool*       memoryPage = nullptr;
    size_t alignedSize = ( in_size + ( in_alignment - 1)) & ~( in_alignment - 1 );

    /// Find the suitabe memory type
    for ( uint32_t i = 0; i < m_types.Num(); i++)
    {
        memoryTypeInfo_t memoryType = m_types[i];    
		if ( in_filter & ( 1 << i ) && ( memoryType.propertyFlags & in_properties ) == in_properties )
				type = &m_types[i];
    }

    /// no suitable found
    if( type == nullptr )
    {
        idLib::Error( "Failed to find a suitabe memory type to alloc a device Memory Heap\n" );
        return nullptr;
    }

    /// check for available heap memory
    // TODO: But perhaps that's not the total amount of contiguous memory available. Something to think about in the future.
    if( m_heaps[type->heapIndex].free < alignedSize ) 
    {
        idLib::Error( "Failed to find a suitabe memory type to alloc a device Memory Heap\n" );
        return nullptr;
    }

    /// Alloc the memory
    crMemoryPool* memoryPage = new crMemoryPool();
    if( !memoryPage->Create( alignedSize, in_alignment, in_properties ) )
    {
        delete memoryPage;
        return nullptr;
    }

    /// update heap info
    m_heaps[type->heapIndex].free -= in_size;
    m_heaps[type->heapIndex].allocated += in_size;

    /// store the page structure for future management ( defragment )
    memoryPage->SetProperties( type->pools.Append( memoryPage ), type->typeIndex );
    return memoryPage;
}

/*
==============
crMemoryHeap::Free
==============
*/
void crMemoryHeap::Free( crMemoryPool* in_pool )
{
    idassert( in_pool != nullptr );
    uint32_t index = in_pool->GetIndex();
    uint32_t type = in_pool->GetType();
    
    /// Remove from the list of used pools
    m_types[type].pools.RemoveIndex( index );

    /// "relase" memory sizes  
    m_heaps[m_types[type].heapIndex].allocated -= in_pool->Size();
    m_heaps[m_types[type].heapIndex].free += in_pool->Size();

    /// release Vulkan device Memory
    in_pool->Destroy();
    delete in_pool;
}

/*
==============
crMemoryHeap::Defrag
==============
*/
void crMemoryHeap::Defrag( void )
{
    /// TODO :P 
    // TODO: Create a separathed thread to move memory, and write to a sub command buffer, and then send at frame beging
}
