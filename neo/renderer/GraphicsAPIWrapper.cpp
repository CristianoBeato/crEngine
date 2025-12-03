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
#include "idlib/precompiled.h"
#include "renderer_common.h"
#include "GraphicsAPIWrapper.h"

/*
===========================================================================
    crBuffer
===========================================================================
*/
crBuffer::crBuffer( void ) : crResourceState(), 
    m_type( BUFFER_TYPE_UNDEFINED ),
    m_acess( BUFFER_ACCESS_NONE ),
    m_size( 0 ),
    m_data( nullptr )
{
}

crBuffer::~crBuffer( void )
{
}

void crBuffer::Upload(const void *in_data, const uintptr_t in_offset, const size_t in_size) const
{
    assert( m_data != nullptr );
    if ( ( in_offset + in_size ) > m_size )
        throw idException( "Buffer Overflow" );

    std::memcpy( static_cast<uint8_t*>( m_data ) + in_offset, in_data, in_size );    
}

void crBuffer::Download(void *in_data, const uintptr_t in_offset, const size_t in_size) const
{
    assert( m_data != nullptr );
    if ( ( in_offset + in_size ) > m_size )
        throw idException( "Buffer Overflow" );
    
    std::memcpy( in_data, static_cast<uint8_t*>( m_data ) + in_offset, in_size );
}

/*
===========================================================================
    crTexture
===========================================================================
*/
crTexture::crTexture( void ) :
    m_type( TEXTURE_NONE )
    m_format( TF_NONE )
{
    m_dimensions = { 0, 0, 0, 0, 0 };
}

crTexture::~crTexture( void )
{
}

/*
===========================================================================
    crSampler
===========================================================================
*/
crSampler::crSampler( void )
{
}

crSampler::~crSampler( void )
{
}

/*
===========================================================================
    crProgram
===========================================================================
*/
crProgram::crProgram( void )
{
}

crProgram::~crProgram( void )
{
}

/*
===========================================================================
    crPipeline
===========================================================================
*/
crPipeline::crPipeline( void ) : m_pipelineConfiguration()
{
}

crPipeline::~crPipeline( void )
{
}

/*
===========================================================================
    crCommandBuffer
===========================================================================
*/
crCommandBuffer::crCommandBuffer( void )
{
}

crCommandBuffer::~crCommandBuffer( void )
{
}

/*
===========================================================================
    crSwapchain
===========================================================================
*/
crSwapchain::crSwapchain( const uint32_t in_width, const uint32_t in_height ) : m_width( in_width ), m_height( in_height )
{
}

crSwapchain::~crSwapchain( void )
{
}

/*
===========================================================================
    crShaderStorage
===========================================================================
*/
crShaderStorage::crShaderStorage( void ) : 
    m_frame( 0 ),
    m_lastTextureIndex( 0 ),
    m_currentVBlock( 0 ),
    m_currentFSBlock( 0 ),
    m_currentLSBlock( 0 ),
    m_VTSSBO( nullptr ),
    m_FGSSBO( nullptr ),
    m_LHSSBO( nullptr )
{
}

crShaderStorage::~crShaderStorage( void )
{
}

void crShaderStorage::Submit(void)
{
    m_VTSSBO->Upload( &m_vertexUniformBlock, m_currentVBlock * sizeof( vertexUniformBlock_t ), sizeof( vertexUniformBlock_t ) );
    m_FGSSBO->Upload( &m_fragmentUniformBlock, m_currentFSBlock * sizeof( fragmentUniformBlock_t ), sizeof( fragmentUniformBlock_t ) );
    m_currentVBlock++;
    m_currentFSBlock++;
}

void crShaderStorage::SubmitLight(void)
{
    m_LHSSBO->Upload( &m_lightUnifomBlock, m_currentLSBlock * sizeof( lightUnifomBlock_t ), sizeof( lightUnifomBlock_t ) );
    m_currentLSBlock++;
}

void crShaderStorage::Begin(void)
{
}

void crShaderStorage::End(void)
{
    // "swap" buffer
    m_frame = m_frame + 1 % SMP_FRAMES;

    // swap buffer region
    m_currentVBlock = m_frame * MAX_UNIFORM_BLOCKS;
    m_currentFSBlock = m_frame * MAX_UNIFORM_BLOCKS;
    m_currentLSBlock = m_frame * MAX_LIGHT_BLOCKS;
}
