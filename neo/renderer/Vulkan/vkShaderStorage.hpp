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

#ifndef __VK_SHADER_STORAGE_HPP__
#define __VK_SHADER_STORAGE_HPP__
 
class vkBindlessTextureSlot : public crBindlessTextureSlot 
{
public:
    vkBindlessTextureSlot( const VkImageView in_imageView, const VkSampler in_sampler, const VkImageLayout in_imageLayout )
    {
        m_descriptorImageInfo.imageLayout = in_imageLayout;
        m_descriptorImageInfo.imageView = in_imageView;
        m_descriptorImageInfo.sampler = in_sampler;;
    }

    ~vkBindlessTextureSlot( void )
    {
        m_descriptorImageInfo.imageView = nullptr;
        m_descriptorImageInfo.sampler = nullptr;
    }

    VkDescriptorImageInfo GetHandle( void ) const { return m_descriptorImageInfo; };

private:
    VkDescriptorImageInfo   m_descriptorImageInfo;
};

class vkShaderStorage : public crShaderStorage
{
public:
    vkShaderStorage( void );
    ~vkShaderStorage( void );
    virtual void                    Begin( void );
    virtual void                    End( void );
    virtual crBindlessTextureSlot*  BindTexture( const crTexture* in_texture, const crSampler* in_sampler );
    virtual void                    FreeSlot( crBindlessTextureSlot* &in_handle );
    VkPipelineLayout    PipelineLayout( void ) const { return m_pipelineLayout; }

private:
    VkDescriptorSetLayout                                       m_descriptorSetLayout;
    VkPipelineLayout                                            m_pipelineLayout;
    VkDescriptorPool                                            m_descriptorPool;
    VkDescriptorSet                                             m_bindlessSet;
    VkDevice                                                    m_device;        
    idStaticList<VkDescriptorImageInfo, MAX_BINDING_SAMPLERS>   m_imageInfos;
};

#endif //!__VK_SHADER_STORAGE_HPP__