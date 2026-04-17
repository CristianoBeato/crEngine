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

#ifndef __PROGRAM_HPP__
#define __PROGRAM_HPP__

class crProgram
{
public:
    enum type_t
    {
        PROG_VERTEX,
        PROG_GEOMETRY,
        PROG_FRAGMENT,
        PROG_COMPUTE
    };
    
    crProgram( void );
    ~crProgram( void );

    virtual bool    Create( const type_t in_type, const void* in_source, const size_t in_size );
    virtual void    Destroy( void );

    const uint32_t  ID( void ) const { return m_ID; }
    VkPipelineShaderStageCreateInfo ShaderStage( void ) const { return m_shaderStage; }

private:
    uint32_t                        m_ID;
    type_t                          m_type;
    VkShaderModule                  m_shaderModule;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

typedef crProgram* crProgramp;

#endif //!__PROGRAM_HPP__