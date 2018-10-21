#ifndef VTEXTURESMANAGER_H
#define VTEXTURESMANAGER_H

#include "Valag/vulkanImpl/vulkanImpl.h"
#include "Valag/vulkanImpl/VTexture.h"

#include <map>
#include <list>

namespace vlg
{

struct VRenderableTexture
{
    VRenderableTexture() : renderTarget(nullptr){}

    VTexture                texture;
    VFramebufferAttachment  attachment;
    VRenderTarget          *renderTarget;
};

struct VTexture2DArrayFormat
{
    uint32_t width;
    uint32_t height;
    VkFormat vkFormat;
    bool     renderable;

    bool operator<( VTexture2DArrayFormat const& rhs ) const
    {
        if(renderable < rhs.renderable)
            return (true);
        if(vkFormat < rhs.vkFormat)
            return (true);
        if(height < rhs.height)
            return (true);
        if(width < rhs.width)
            return (true);
        return (false);
    }
};

struct VTexture2DArray
{
    VImage      image;
    VkImageView view;

    VkExtent2D  extent;

    std::mutex  mutex;

    std::list<size_t>   availableLayers;
};

///I need a cleaning list so that I free resource only after a full image_index*loop

class VTexturesManager : public Singleton<VTexturesManager>
{
    public:
        friend class Singleton<VTexturesManager>;
        friend class VApp;

        void update(size_t frameIndex, size_t imageIndex);

        static bool allocTexture(uint32_t width, uint32_t height, VkFormat format,
                                 VBuffer source, CommandPoolName cmdPoolName, VTexture *texture);
        static bool allocTexture(uint32_t width, uint32_t height,
                                 VBuffer source, CommandPoolName cmdPoolName, VTexture *texture);
        static bool allocRenderableTexture(uint32_t width, uint32_t height, VkFormat format,
                                           VRenderPass *renderPass, VRenderableTexture *texture);
        static void freeTexture(VTexture &texture);
        static void freeTexture(VRenderableTexture &texture);


        static VkSampler                sampler();
        static VkDescriptorSetLayout    descriptorSetLayout();
        static VkDescriptorSet          descriptorSet(); //Use last frame index
        static VkDescriptorSet          descriptorSet(size_t frameIndex);
        static VkDescriptorSet          imgDescriptorSet(size_t imageIndex);
        static size_t                   descriptorSetVersion(size_t frameIndex);
        static size_t                   imgDescriptorSetVersion(size_t imageIndex);

        static VTexture getDummyTexture();

    protected:
        VTexturesManager();
        virtual ~VTexturesManager();

        bool    allocTextureImpl(VTexture2DArrayFormat format, VBuffer source, CommandPoolName cmdPoolName, VTexture *texture);
        bool    allocRenderableTextureImpl(VTexture2DArrayFormat format, CommandPoolName cmdPoolName, VTexture *texture);
        size_t  createTextureArray(VTexture2DArrayFormat format);

        void    freeTextureImpl(VTexture &texture);
        void    freeTextureImpl(VRenderableTexture &texture);
        void    updateCleaningLists(bool cleanAll = false);

        bool    createDescriptorSetLayouts();
        bool    createSampler();
        bool    createDescriptorPool(size_t framesCount, size_t imagesCount);
        bool    createDescriptorSets(size_t framesCount, size_t imagesCount);
        bool    createDummyTexture();

        void    updateDescriptorSet(size_t frameIndex);
        void    updateImgDescriptorSet(size_t imageIndex);
        void    writeDescriptorSet(VkDescriptorSet &descSet);

        bool    init(size_t framesCount, size_t imagesCount);
        void    cleanup();

        VkDescriptorSetLayout   getDescriptorSetLayout();
        VkDescriptorSet         getDescriptorSet(size_t frameIndex);
        VkDescriptorSet         getImgDescriptorSet(size_t imageIndex);
        size_t                  getDescriptorSetVersion(size_t frameIndex);
        size_t                  getImgDescriptorSetVersion(size_t imageIndex);

    private:
        std::multimap<VTexture2DArrayFormat, size_t> m_formatToArray;
        std::vector<VTexture2DArray*>       m_allocatedTextureArrays;
        std::vector<VkDescriptorImageInfo>  m_imageInfos;

        VTexture m_dummyTexture;

        std::mutex m_createImageMutex;

        VkSampler                       m_sampler;
        std::vector<bool>               m_needToUpdateDescSet;
        std::vector<bool>               m_needToUpdateImgDescSet;
        std::vector<size_t>             m_descSetVersion;
        std::vector<size_t>             m_imgDescSetVersion;
        std::vector<VkDescriptorSet>    m_descriptorSets;
        std::vector<VkDescriptorSet>    m_imgDescriptorSets;
        VkDescriptorSetLayout           m_descriptorSetLayout;
        VkDescriptorPool                m_descriptorPool;

        size_t  m_lastFrameIndex;
        size_t  m_framesCount;

        std::list<std::pair<int, VRenderTarget*> >  m_cleaningList_renderTargets;
        std::list<std::pair<int, VkImageView> >     m_cleaningList_imageViews;

    public:
        static const size_t MAX_TEXTURES_ARRAY_SIZE;
        static const size_t MAX_LAYER_PER_TEXTURE;
};

}

#endif // VTEXTURESMANAGER_H
