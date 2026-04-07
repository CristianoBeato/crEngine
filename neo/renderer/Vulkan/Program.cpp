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
#include "Program.hpp"
#include "Core.hpp"

crProgram::crProgram( void )
{
}

crProgram::~crProgram( void )
{
}

bool crProgram::Create( const type_t in_type, const void* in_source, const size_t in_size )
{
    VkResult result = VK_SUCCESS; 
    auto device = tr.GetRenderDevice();

    VkShaderModuleCreateInfo shaderModuleCI{};
    shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCI.codeSize = in_size / sizeof( uint32_t );
    shaderModuleCI.pCode = reinterpret_cast<const uint32_t*>( in_source );

    result = vkCreateShaderModule( *device, &shaderModuleCI, k_allocationCallbacks, &m_shaderModule);
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "crProgram::Create failed to create shader\n" );
        return false;
    }

    m_shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;;
    m_shaderStage.pNext = nullptr;
    m_shaderStage.flags = 0;
    m_shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    m_shaderStage.module = m_shaderModule;
    m_shaderStage.pName = "main";
    m_shaderStage.pSpecializationInfo = nullptr;

    switch ( in_type )
    {
        case PROG_VERTEX:
            m_shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;

        case PROG_GEOMETRY:
            m_shaderStage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            break;

        case PROG_FRAGMENT:
            m_shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;

        case PROG_COMPUTE:
            m_shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
    };

    return true;
}

void crProgram::Destroy( void )
{
    if ( m_shaderModule == nullptr )
    {
        auto device = tr.GetRenderDevice();
        vkDestroyShaderModule( *device, m_shaderModule, k_allocationCallbacks );
        m_shaderModule = nullptr;
    }
}
