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
#include "glProgram.hpp"

glProgram::glProgram( void ) : crProgram()
{
}

glProgram::~glProgram( void )
{
}

bool glProgram::Create(const type_t in_type, const void* in_source, const size_t in_size )
{
    GLenum shType = GL_NONE;
    m_type = in_type;
    switch ( m_type )
    {
    case PROG_VERTEX:
        shType = GL_VERTEX_SHADER;
        break;
    case PROG_GEOMETRY:
        shType = GL_GEOMETRY_SHADER;
    break;
    case PROG_FRAGMENT:
        shType = GL_FRAGMENT_SHADER;
    break;
    case PROG_COMPUTE:
        shType = GL_COMPUTE_SHADER;
        break;
    };

    GLuint shader = glCreateShader( shType ); // ou FRAGMENT_SHADER

    // SPIR-V binário
    glShaderBinary( 1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, in_source, in_size );

    // Marcar como separável
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    m_program = glCreateProgram();
    glProgramParameteri( m_program, GL_PROGRAM_SEPARABLE, GL_TRUE);

    // Anexar shader compilado
    glAttachShader( m_program, shader );
    glLinkProgram( m_program );

    // Verifique link
    GLint linked;
    glGetProgramiv( m_program, GL_LINK_STATUS, &linked );
    if (!linked) 
    {
        char log[1024];
        glGetProgramInfoLog( m_program, 1024, nullptr, log );
        common->Error("glProgram::Create Program link failed: %s\n", log);
        return false;
    }

    glDetachShader( m_program, shader );
    glDeleteShader( shader );

    return true;
}

void glProgram::Destroy(void)
{
    if ( m_program != 0 )
    {
        glDeleteProgram( m_program );
        m_program = 0;
    }
}
