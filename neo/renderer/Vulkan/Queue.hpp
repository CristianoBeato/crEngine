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
#ifndef __DEVICE_QUEUE_HPP__
#define __DEVICE_QUEUE_HPP__

struct queueInfo_t
{
    bool        present = false;    // is a present queue
    bool        graphic = false;    // is a graphic queue
    bool        transfer = false;   // is a transfer queue
    bool        compute = false;    // compute queue 
    uint32_t    index = 0;          // queue index 
    uint32_t    family = 0;         // quque family
};

typedef class crQueue
{
public:
    crQueue( const uint32_t in_family, const uint32_t in_index );
    ~crQueue( void );
    bool            Init( const VkDevice in_device );
    void            ResetPool( void ) const;
    uint32_t        Index( void ) const { return m_index; }
    uint32_t        Family( void ) const { return m_index; }
    VkQueue         Queue( void ) const { return m_queue; }
    VkCommandPool   CommandPool( void ) const { return m_commandPool; }

private:
    uint32_t                m_index;        // index in the family 
    uint32_t                m_family;       // the family index
    VkQueue                 m_queue;        // queue hanlde
    VkCommandPool           m_commandPool;  // queue command pool
    VkSemaphore             m_semaphore;    // queue semaphore
    VkDevice                m_device;       // parent device
} * crQueuep;


#endif //!__DEVICE_QUEUE_HPP__