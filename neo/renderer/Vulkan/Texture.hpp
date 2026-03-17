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

class vkTexture : public vkResourceState
{
public:
    struct dimensions_t
    {
        uint16_t    levels = 0;
        uint16_t    layers = 0;
        uint32_t    width = 0;
        uint32_t    heigth = 0;
        uint32_t    depth = 0;
    };
    
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

    virtual bool                Create( const image_type_t in_type, const dimensions_t in_dimensions, const crInternalFormat in_format );
    virtual void                Destroy( void );
    const uint16_t              Levels( void ) const { return m_dimensions.levels; }
    const uint16_t              Layers( void ) const { return m_dimensions.layers; }
    const uint32_t              CurrentQueue( void ) const { return m_state.queueFamily; }
    const VkImage               Image( void ) const { return m_image; }
    const VkImageView           ImageView( void ) const { return m_view; }
    const VkImageLayout         Layout( void ) const { return m_state.layout; }
    const VkPipelineStageFlags2 Stage( void ) const { return m_state.stage; }
    const VkAccessFlags2        Access( void ) const {return m_state.access; }
    const VkImageAspectFlags    Aspect( void ) const { return m_state.aspect; }
    void                        SetState( const textureState_t &in_state, const VkCommandBuffer in_commandBuffer );
    const image_type_t          GetType( void ) const { return m_type; }
    const crInternalFormat      GetFormat( void ) const { return m_format; }

protected:
    image_type_t        m_type;
    crInternalFormat    m_format;
    dimensions_t        m_dimensions;
    textureState_t      m_state;
    VkImage             m_image;
    VkImageView         m_view;
    VkDeviceMemory      m_memory;
};

class vkSampler
{
public:
    vkSampler( void );
    ~vkSampler( void );

    enum filter_t
    {
        FILTER_NEAREST,
        FILTER_LINEAR,
        FILTER_BILINEAR,
        FILTER_TRILINEAR,
        FILTER_ANISOTROPIC2X,
        FILTER_ANISOTROPIC4X,
        FILTER_ANISOTROPIC8X,
        FILTER_ANISOTROPIC16X
    };

    enum wrapping_t 
    {
        WRAP_NONE,
        WRAP_REPEAT,
        WRAP_MIRRORED,
        WRAP_EDGE,
        WRAP_BORDER        
    };

    bool    Create( const filter_t in_filtering, const wrapping_t in_Swrap, const wrapping_t in_Twrap, const wrapping_t in_Rwrap );
    void    Destroy( void );
    void*   Handler( void ) const { return reinterpret_cast<void*>( m_sampler ); };

private:
    VkSampler   m_sampler;
};

#endif //!__VK_TEXTURE_HPP__