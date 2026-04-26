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

#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

class crTexture
{
public:
    enum type_t : uint8_t
    {
	    IMAGE_NONE,
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
        IMAGE_CUBEMAP,
        IMAGE_TYPE_COUNT
    };

    struct dimensions_t
    {
        uint16_t    levels = 0;
        uint16_t    layers = 0;
        uint32_t    width = 0;
        uint32_t    heigth = 0;
        uint32_t    depth = 0;
    };
    
    struct state_t 
    {
        uint32_t                family;
        VkImageLayout           layout;
        VkPipelineStageFlags2   stage;
        VkAccessFlags2          access;

        bool operator==( const state_t & r ) const
        {
            return ( family == r.family ) && ( layout == r.layout ) && ( stage == r.stage ) && ( access == r.access );
        }

        bool operator==( const state_t & r )
        {
            return ( family == r.family ) && ( layout == r.layout ) && ( stage == r.stage ) && ( access == r.access );
        }
    };

    crTexture( void );
    ~crTexture( void );
    
    bool                        Create( const type_t in_type, const dimensions_t in_dimensions, const crInternalFormat in_format );
    bool                        Create( const VkImage in_image, const crInternalFormat in_format, const VkImageViewType in_viewType );
    bool                        Storage( crMemoryPoolp in_bufferPool );
    void                        Destroy( void );
    void                        SetState( const crCommandBufferp in_commandBuffer, const state_t in_state );
    dimensions_t                Dimensions( void ) const { return m_dimensions; }
    const VkImage               Image( void ) const { return m_image; }
    const VkImageView           View( void ) const { return m_view; }
    const VkImageLayout         Layout( void ) const { return m_state.layout; }
    const VkPipelineStageFlags2 Stage( void ) const { return m_state.stage; }
    const VkAccessFlags2        Access( void ) const {return m_state.access; }
    const VkImageAspectFlags    Aspect( void ) const { return m_aspect; }
    const type_t                GetType( void ) const { return m_type; }
    const crInternalFormat      GetFormat( void ) const { return m_format; }

    operator VkImage( void ) const { return m_image; }
    operator VkImageView( void ) const { return m_view; }

protected:
    type_t                  m_type;
    VkImageAspectFlags      m_aspect;
    crInternalFormat        m_format;
    state_t                 m_state;
    dimensions_t            m_dimensions;
    VkMemoryRequirements    m_memoryRequirements;
    VkImage                 m_image;
    VkImageView             m_view;
    crMemoryPage*           m_page;
};

class crSampler
{
public:
    crSampler( void );
    ~crSampler( void );

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

    const filter_t      Filtering( void ) const { return m_filtering; }
    const wrapping_t    WrapS( void ) const { return m_wrapS; }

private:
    filter_t    m_filtering;
    wrapping_t  m_wrapS;
    wrapping_t  m_wrapT;
    wrapping_t  m_wrapR;
    VkSampler   m_sampler;
};

typedef crSampler* crSamplerp;

#endif //!__VK_TEXTURE_HPP__