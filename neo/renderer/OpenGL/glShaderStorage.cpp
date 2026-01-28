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
#include "renderer/renderer_common.h"
#include "glShaderStorage.hpp"

constexpr size_t FRAME_SSBO_SAMP_SIZE = MAX_BINDING_SAMPLERS * sizeof( GLuint64 );

glShaderStorage::glShaderStorage( void ) : 
    crShaderStorage(),
    m_TSSSBO( nullptr )
{
    m_TSSSBO = new glBuffer();
    m_VTSSBO = new glBuffer();
    m_FGSSBO = new glBuffer();
    m_LHSSBO = new glBuffer();
    m_TLSSBO = new glBuffer();
}

glShaderStorage::~glShaderStorage( void )
{
    delete m_TLSSBO;
    delete m_LHSSBO;
    delete m_FGSSBO;
    delete m_VTSSBO;
    delete m_TSSSBO;

    m_TLSSBO = nullptr;
    m_LHSSBO = nullptr;
    m_FGSSBO = nullptr;
    m_VTSSBO = nullptr;
    m_TSSSBO = nullptr;
}

void glShaderStorage::StartUp(void)
{
    // 
    m_TSSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_SAMP_SIZE * SMP_FRAMES );
    m_VTSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_VERT_SIZE * SMP_FRAMES );
    m_FGSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_FRAG_SIZE * SMP_FRAMES );
    m_LHSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_LIGH_SIZE * SMP_FRAMES );
    m_TLSSBO->Create( crBuffer::BUFFER_TYPE_SHADER, crBuffer::BUFFER_ACCESS_WRITE, FRAME_SSBO_TXLC_SIZE * SMP_FRAMES );
}

void glShaderStorage::ShutDown(void)
{
    m_TLSSBO->Destroy();
    m_LHSSBO->Destroy();
    m_FGSSBO->Destroy();
    m_VTSSBO->Destroy();
    m_TSSSBO->Destroy();
}

crBindlessTextureSlot *glShaderStorage::BindTexture(const crTexture *in_texture, const crSampler *in_sampler)
{
    glBindlessTextureSlot *freeSlot = nullptr;
    if( !m_freeList.Num() > 0 )
    {
        GLuint64 handler = 0; 
        uint32_t index = m_freeList.Last();
        
        freeSlot = new glBindlessTextureSlot( *static_cast<GLuint*>( in_texture->Handler() ), *static_cast<GLuint*>( in_sampler->Handler() ) );
        handler = freeSlot->GetHandle();
        m_TSSSBO->Upload( &handler, index * sizeof( GLuint64 ), sizeof( GLuint64 ) );
     
        // remove last 
        m_freeList.RemoveIndex( m_freeList.Num() - 1 );
    }
    else
    {
        GLuint64 handler = 0;
        uint32_t index = m_lastTextureIndex++;
        freeSlot = new glBindlessTextureSlot( *static_cast<GLuint*>( in_texture->Handler() ), *static_cast<GLuint*>( in_sampler->Handler() ) );
        handler = freeSlot->GetHandle();
        m_TSSSBO->Upload( &handler, index * sizeof( GLuint64 ), sizeof( GLuint64 ) );
        freeSlot->SetIndex( index );
    }

    return freeSlot;
}

void glShaderStorage::FreeSlot(crBindlessTextureSlot *&in_handle)
{
    m_freeList.Append( in_handle->GetIndex() );
    delete in_handle;
}
