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

#ifndef __GL_PROGRAM_HPP__
#define __GL_PROGRAM_HPP__

class glProgram : public crProgram
{
public:
    
    glProgram( void );
    ~glProgram( void );

    virtual bool    Create( const type_t in_type, const void* in_source, const size_t in_size );
    virtual void    Destroy( void );

private:
    GLuint  m_program;
};

#endif //!__PROGRAM_HPP__