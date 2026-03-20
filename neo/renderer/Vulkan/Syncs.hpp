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

#ifndef __SYNCS_HPP__
#define __SYNCS_HPP__

class crFence
{
public:
    crFence( void );
    ~crFence( void );
    bool                Create( const uint16_t in_frameCount, const bool in_signaled = false );
    void                Destroy( void );

    /// @brief Reset the server signal
    void                Reset( void ) const;

    /// @brief Wait for a server signal
    VkResult            Wait( const uint64_t in_timeout = UINT64_MAX ) const;
    VkResult            Status( void ) const;
    ID_INLINE void      SwapFrame( void ) { m_frameID = ( m_frameID + 1 ) % m_frameCount; }
    ID_INLINE VkFence   Fence( void ) const { return m_fences[m_frameID]; }
    ID_INLINE operator  VkFence( void ) const { return m_fences[m_frameID]; }

private:
    uint16_t    m_frameID;
    uint16_t    m_frameCount;
    VkFence*    m_fences;
    VkDevice    m_device;
};

class crSemaphore
{
public:
    virtual VkSemaphore*            Pointer( void ) const { return nullptr; }
    virtual VkSemaphore             Semaphore( void ) const { return nullptr; }
    virtual VkSemaphoreSubmitInfo   SubmitInfo( void ) { return {}; }
    virtual operator    VkSemaphore( void ) const { return nullptr; }
    virtual operator    VkSemaphoreSubmitInfo( void ) const { return {};}
};

class crSemaphoreRoundRobin : public crSemaphore
{
public:
    crSemaphoreRoundRobin( void );
    ~crSemaphoreRoundRobin( void );
    bool                Create( const uint16_t in_frameCount );
    void                Destroy( void );

    /// @brief Send a client signal to Server
    void                Signal( void ) const;

    /// @brief Make client wait for a server signal
    VkResult            Wait( const uint64_t in_timeout = UINT64_MAX ) const;

    ID_INLINE void      SwapFrame( void ) { m_frameID = ( m_frameID + 1 ) % m_frameCount; }

    virtual VkSemaphore*            Pointer( void ) const { return m_semaphores[m_frameID]; }
    virtual VkSemaphoreSubmitInfo SubmitInfo( void );
    virtual VkSemaphore Semaphore( void ) const override { return m_semaphores[m_frameID]; }
    virtual operator    VkSemaphoreSubmitInfo( void ) const;
    virtual operator    VkSemaphore( void ) const override { return m_semaphores[m_frameID]; }

private:
    uint16_t        m_frameID;
    uint16_t        m_frameCount;
    VkSemaphore*    m_semaphores;
    VkDevice        m_device;
};

class crSemaphoreTimeline : public crSemaphore
{
public:
    crSemaphoreTimeline( void );
    ~crSemaphoreTimeline( void );
    bool                Create( void );
    void                Destroy( void );
    void                Signal( const uint64_t in_value ) const;
    VkResult            Wait( const uint64_t in_value, const uint64_t in_timeout = UINT64_MAX ) const;
    ID_INLINE void      SwapFrame( void ) { m_timeline++; }
    ID_INLINE uint64_t  Timeline( void ) const { return m_timeline; }
    virtual VkSemaphore*            Pointer( void ) const { return &m_semaphore; }
    virtual VkSemaphoreSubmitInfo   SubmitInfo( void );
    virtual VkSemaphore             Semaphore( void ) const override { return m_semaphore; }
    virtual operator                VkSemaphoreSubmitInfo( void ) const;
    virtual operator                VkSemaphore( void ) const override { return m_semaphore; }

private:
    uint64_t    m_timeline;
    VkSemaphore m_semaphore;
    VkDevice    m_device;
};

#endif //!__SYNCS_HPP__