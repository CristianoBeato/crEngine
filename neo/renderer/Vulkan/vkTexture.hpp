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

#ifndef __VK_TEXTURE_HPP__
#define __VK_TEXTURE_HPP__

class vkSampler : public crSampler
{
public:
    vkSampler( void );
    ~vkSampler( void );

    virtual bool    Create( const filter_t in_filtering, const wrapping_t in_Swrap, const wrapping_t in_Twrap, const wrapping_t in_Rwrap );
    virtual void    Destroy( void );
    virtual void*   Handler( void ) const;

private:
    VkDevice    m_device;
    VkSampler   m_sampler;
};

class vkTexture : public crTexture
{
public:

    /// @brief store texture state
    struct textureState_t
    {
        VkImageLayout           layout;
        VkImageAspectFlags      aspect; 
        VkPipelineStageFlags2   stage;  
        VkAccessFlags2          access;
        uint32_t                queueFamily;
    };

    vkTexture( void );
    ~vkTexture( void );

    virtual bool                Create( const type_t in_type, const dimensions_t in_dimensions, const format_t in_format );
    virtual void                Destroy( void );
    virtual void*               Handler( void ) const;

    /// @brief Texture mip count  
    /// @return number of mipmaps by layer
    const uint16_t              Levels( void ) const { return m_dimensions.levels; }
    
    /// @brief Texture layer count 
    /// @return number of layer ( faces if a cubemap )
    const uint16_t              Layers( void ) const { return m_dimensions.layers; }
    const uint32_t              CurrentQueue( void ) const { return m_state.queueFamily; }
    const VkImage               Image( void ) const { return m_image; }
    const VkImageView           ImageView( void ) const { return m_view; }
    const VkImageLayout         Layout( void ) const { return m_state.layout; }
    const VkPipelineStageFlags2 Stage( void ) const { return m_state.stage; }
    const VkAccessFlags2        Access( void ) const {return m_state.access; }
    const VkImageAspectFlags    Aspect( void ) const { return m_state.aspect; }
    void                        SetState( const textureState_t &in_state, const VkCommandBuffer in_commandBuffer );

private:
    textureState_t          m_state;
    dimensions_t            m_dimensions;
    VkImage                 m_image;
    VkImageView             m_view;
    VkDeviceMemory          m_memory;
    VkDevice                m_device;
};

#endif //!__VK_TEXTURE_HPP__