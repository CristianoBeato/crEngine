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

#ifndef __VK_PIPELINE_HPP__
#define __VK_PIPELINE_HPP__

class vkPipeline : crPipeline
{
public:
    vkPipeline( void );
    ~vkPipeline( void );

    virtual bool    Create( const PipelineInfo_t in_pipelineInfo );
    virtual void    Destroy( void );

    VkPipeline          Pipeline( void ) const { return m_pipeline; }

private:
    VkPipeline                          m_pipeline;
    VkDevice                            m_device;
};

#endif //!__VK_PIPELINE_HPP__