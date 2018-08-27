#ifndef TEXTURESARRAYMANAGER_H
#define TEXTURESARRAYMANAGER_H

#include "Valag/vulkanImpl/vulkanImpl.h"

#include <map>
#include <list>

/** I could try to work with multiple arrays... and create new descriptor sets **/

namespace vlg
{

class TexturesArrayManager
{
    public:
        TexturesArrayManager();
        virtual ~TexturesArrayManager();

        bool bindTexture(AssetTypeID id, size_t frameIndex, int *texArrayID);

        VkDescriptorSetLayout   getDescriptorSetLayout();
        VkDescriptorSet         getDescriptorSet(size_t frameIndex);
        size_t                  getDescriptorSetVersion(size_t frameIndex);

        void checkUpdateDescriptorSets(size_t frameIndex);

    protected:
        bool    createDescriptorSetLayouts();
        bool    createSampler();
        bool    createDescriptorPool();
        bool    createDescriptorSets();

        void    updateDescriptorSet(size_t frameIndex);

        bool    init();
        void    cleanup();

    private:
       // std::vector<std::map<AssetTypeID, size_t> >       m_texturesArray;
        std::map<AssetTypeID, size_t> m_texturesArray;

        std::list<size_t>                   m_availableImageInfos;
        std::vector<VkDescriptorImageInfo>  m_imageInfos;

       // std::list<std::pair<AssetTypeID, size_t> >   m_texturesToAdd;
        std::map<AssetTypeID, size_t> m_texturesToAdd;

       // std::vector<VkImageView>            m_imageViews;

        VkSampler                       m_sampler;
        std::vector<bool>               m_needToUpdateDescSet;
        std::vector<size_t>             m_descSetVersion;
        std::vector<VkDescriptorSet>    m_descriptorSets;
        VkDescriptorSetLayout           m_descriptorSetLayout;
        VkDescriptorPool                m_descriptorPool;

    public:
        static const size_t TEXTURES_ARRAY_SIZE;
};

}

#endif // TEXTURESARRAYMANAGER_H
