#include "Valag/vulkanImpl/VTexturesManager.h"

#include "Valag/core/VApp.h"
//#include "Valag/core/AssetHandler.h"
//#include "Valag/gfx/TextureAsset.h"
#include "Valag/vulkanImpl/VTexture.h"

namespace vlg
{

const size_t VTexturesManager::TEXTURES_ARRAY_SIZE = 128; //Number of texture2DArray
const size_t VTexturesManager::MAX_LAYER_PER_TEXTURE = 128; //Number of layers in each texture2DArray

VTexturesManager::VTexturesManager()
{
    //this->init();
}

VTexturesManager::~VTexturesManager()
{
    this->cleanup();
}

/*bool VTexturesManager::bindTexture(AssetTypeID id, size_t frameIndex, int *texArrayID)
{
    return VTexturesManager::instance()->bindTextureImpl(id, frameIndex, texArrayID);
}*/

/*bool VTexturesManager::bindTextureImpl(AssetTypeID id, size_t frameIndex, int *texArrayID)
{
    auto imageIt = m_texturesArray.find(id);
    if(imageIt == m_texturesArray.end())
    {
        auto asset = TextureHandler::instance()->getAsset(id);

        if(!asset->isLoaded())
        {
            (*texArrayID) = 0;
            return (true);
        }

        imageIt = m_texturesToAdd.find(id);

        if(imageIt != m_texturesToAdd.end())
            return (false);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = nullptr;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = asset->getImageView();

        m_texturesToAdd[id] = m_availableImageInfos.front();

        m_imageInfos[m_availableImageInfos.front()] = imageInfo;
        m_availableImageInfos.pop_front();

        for(auto b : m_needToUpdateDescSet)
            b = true;
      //  m_needToUpdateDescSet[frameIndex] = true;
       // this->updateDescriptorSet(frameIndex);
       return (false);
    }

    (*texArrayID) = imageIt->second;

    return (true);
}*/


bool VTexturesManager::allocTexture(uint32_t width, uint32_t height, VBuffer source, CommandPoolName cmdPoolName, VTexture *texture)
{
    return VTexturesManager::instance()->allocTextureImpl(width, height, source, cmdPoolName, texture);
}


bool VTexturesManager::allocTextureImpl(uint32_t width, uint32_t height, VBuffer source, CommandPoolName cmdPoolName, VTexture *texture)
{
    auto foundedArrays = m_extentToArray.equal_range({width, height});
    size_t chosenArray = m_allocatedTextureArrays.size();

    for(auto ar = foundedArrays.first ; ar != foundedArrays.second ; ++ar)
    {
        if(!m_allocatedTextureArrays[ar->second].availableLayers.empty())
        {
            chosenArray = ar->second;
            break;
        }
    }

    if(chosenArray == m_allocatedTextureArrays.size())
        if(!this->createTextureArray(width, height))
            return (false);

    VTexture2DArray &texture2DArray = m_allocatedTextureArrays[chosenArray];
    size_t chosenLayer = *texture2DArray.availableLayers.begin();
    texture2DArray.availableLayers.pop_front();

    texture->m_textureId = chosenArray;
    texture->m_textureLayer = chosenLayer;

    VulkanHelpers::transitionImageLayout(texture2DArray.image,chosenLayer, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,cmdPoolName);
        VBuffersAllocator::copyBufferToImage(source, texture2DArray.image,
                                         width, height,chosenLayer,cmdPoolName);
    VulkanHelpers::transitionImageLayout(texture2DArray.image,chosenLayer, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmdPoolName);

    return (true);
}

bool VTexturesManager::createTextureArray(uint32_t width, uint32_t height)
{
    if(m_allocatedTextureArrays.size() >= TEXTURES_ARRAY_SIZE)
        return (false);

    m_extentToArray.insert({{width, height}, m_allocatedTextureArrays.size()});

    VTexture2DArray texture2DArray;

    texture2DArray.layerCount = MAX_LAYER_PER_TEXTURE;
    texture2DArray.extent.width = width;
    texture2DArray.extent.height = height;

    if(!VulkanHelpers::createImage(width, height,texture2DArray.layerCount, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               texture2DArray.image, texture2DArray.memory))
                               return (false);

    texture2DArray.view = VulkanHelpers::createImageView(texture2DArray.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,texture2DArray.layerCount);

    for(size_t i = 0 ; i < MAX_LAYER_PER_TEXTURE ; ++i)
        texture2DArray.availableLayers.push_back(i);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = nullptr;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture2DArray.view;

    m_imageInfos[m_allocatedTextureArrays.size()] = imageInfo;

    m_allocatedTextureArrays.push_back(texture2DArray);

    for(auto b : m_needToUpdateDescSet)
        b = true;

    return true;
}


void VTexturesManager::freeTexture(VTexture &texture)
{
    VTexturesManager::instance()->freeTextureImpl(texture);
}

void VTexturesManager::freeTextureImpl(VTexture &texture)
{
    if(!(texture.m_textureId == 0 && texture.m_textureLayer == 0))
    {
        m_allocatedTextureArrays[texture.m_textureId].availableLayers.push_back(texture.m_textureLayer);
        texture.m_textureId = 0;
        texture.m_textureLayer = 0;
    }
}


VkDescriptorSetLayout VTexturesManager::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet VTexturesManager::getDescriptorSet(size_t frameIndex)
{
    return m_descriptorSets/*[m_descriptorSetsNbr[frameIndex]]*/[frameIndex];
}

size_t VTexturesManager::getDescriptorSetVersion(size_t frameIndex)
{
    return m_descSetVersion[frameIndex];
}

void VTexturesManager::checkUpdateDescriptorSets(size_t frameIndex)
{
   // for(size_t i = 0 ; i < 2 ; ++i) {
        if(m_needToUpdateDescSet[frameIndex] == true)
        {
            /*for(auto tex : m_texturesToAdd)
                m_texturesArray[tex.first] = tex.second;
            m_texturesToAdd.clear();*/
            this->updateDescriptorSet(frameIndex);
        }
       // frameIndex = (frameIndex + 1) % VApp::MAX_FRAMES_IN_FLIGHT;
   // }
}

bool VTexturesManager::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding layoutBindings[2];
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = TEXTURES_ARRAY_SIZE;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = layoutBindings;

    return (vkCreateDescriptorSetLayout(VInstance::device(), &layoutInfo, nullptr, &m_descriptorSetLayout) == VK_SUCCESS);
}

bool VTexturesManager::createSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    return (vkCreateSampler(VInstance::device(), &samplerInfo, nullptr, &m_sampler) == VK_SUCCESS);
}

bool VTexturesManager::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize[2];
    poolSize[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSize[0].descriptorCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT/* *2 */);
    poolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT/* *2 */);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT /* *2*/ /* *2 */);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool VTexturesManager::createDescriptorSets()
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayout> layouts(VApp::MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_descSetVersion = std::vector<size_t> (VApp::MAX_FRAMES_IN_FLIGHT, 0);
    m_descriptorSets.resize(VApp::MAX_FRAMES_IN_FLIGHT);
   // m_descriptorSets[1].resize(VApp::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets.data()) != VK_SUCCESS)
        return (false);
   // if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets[1].data()) != VK_SUCCESS)
     //   return (false);

    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        this->updateDescriptorSet(i);

    return (true);
}

void VTexturesManager::updateDescriptorSet(size_t frameIndex)
{
    VkWriteDescriptorSet setWrites[2];

	VkDescriptorImageInfo samplerInfo = {};
	samplerInfo.sampler = m_sampler;

	setWrites[0] = {};
	setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[0].dstBinding = 0;
	setWrites[0].dstArrayElement = 0;
	setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	setWrites[0].descriptorCount = 1;
	setWrites[0].dstSet = m_descriptorSets/*[m_descriptorSetsNbr[frameIndex]]*/[frameIndex];
	setWrites[0].pBufferInfo = 0;
	setWrites[0].pImageInfo = &samplerInfo;

	setWrites[1] = {};
	setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[1].dstBinding = 1;
	setWrites[1].dstArrayElement = 0;
	setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	setWrites[1].descriptorCount = TEXTURES_ARRAY_SIZE; /// static_cast<uint32_t>(/*m_texturesArray.size()*/m_imageInfos[frameIndex].size());//TEXTURES_ARRAY_SIZE;
	setWrites[1].pBufferInfo = 0;
	setWrites[1].dstSet = m_descriptorSets/*[m_descriptorSetsNbr[frameIndex]]*/[frameIndex];
	setWrites[1].pImageInfo =  m_imageInfos.data();

    vkUpdateDescriptorSets(VInstance::device(), 2, setWrites, 0, nullptr);

    m_needToUpdateDescSet[frameIndex] = false;
    ++m_descSetVersion[frameIndex];
}

bool VTexturesManager::createDummyTexture()
{

    unsigned char dummyTexturePtr[4] = {255,255,255,255};
    //TextureHandler::instance()->enableDummyAsset();
    //TextureAsset* dummyTexture = TextureHandler::instance()->getDummyAsset();

    return m_dummyTexture.generateTexture(dummyTexturePtr,1,1);

   // this->createTextureArray(1,1);
}

bool VTexturesManager::init()
{
    m_imageInfos.resize(1);
    if(!this->createDummyTexture())
        return (false);

    //m_texturesArray.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    //m_texturesToAdd.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_needToUpdateDescSet = std::vector<bool> (VApp::MAX_FRAMES_IN_FLIGHT, true);

    //m_imageInfos.resize(VApp::MAX_FRAMES_IN_FLIGHT);

   // m_imageViews.resize(TEXTURES_ARRAY_SIZE)
    m_imageInfos.resize(TEXTURES_ARRAY_SIZE);

    size_t i = 0;
    for(auto &imageInfo : m_imageInfos)
    {
        imageInfo.sampler = nullptr;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_allocatedTextureArrays[0].view;//m_dummyTexture.view//TextureHandler::instance()->getDummyAsset()->getImageView();

        //if(i != 0)
          //  m_availableImageInfos.push_back(i);
        i++;
    }
    //m_texturesArray[0] = 0;

    if(!this->createDescriptorSetLayouts())
        return (false);
    if(!this->createSampler())
        return (false);
    if(!this->createDescriptorPool())
        return (false);
    if(!this->createDescriptorSets())
        return (false);

    return (true);
}

void VTexturesManager::cleanup()
{
    VkDevice device = VInstance::device();

    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyDescriptorPool(device,m_descriptorPool,nullptr);
    vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);

    for(auto vtexture : m_allocatedTextureArrays)
    {
        vkDestroyImageView(device, vtexture.view, nullptr);
        vkDestroyImage(device, vtexture.image, nullptr);
        vkFreeMemory(device, vtexture.memory, nullptr);
    }
}

}
