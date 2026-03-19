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

#ifndef __QUERIES_HPP__
#define __QUERIES_HPP__

class vkTimeQueries
{
public:
    vkTimeQueries( void );
    ~vkTimeQueries( void );
    void        Create( void );
    void        Destroy( void );
    void        BeginRegister( const vkCommandbuffer* in_cmd );
    void        EndRegister( const vkCommandbuffer* in_cmd );
    uint64_t    Retrieve( void );

private:
    float       m_timestampPeriod;
    uint32_t    m_bufferID;
    VkQueryPool m_pools[SMP_FRAMES];
};

#endif //!__QUERIES_HPP__