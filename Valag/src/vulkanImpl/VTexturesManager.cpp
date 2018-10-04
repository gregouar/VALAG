#include "Valag/vulkanImpl/VTexturesManager.h"

#include "Valag/core/VApp.h"
#include "Valag/vulkanImpl/VTexture.h"

#include "Valag/core/Config.h"

namespace vlg
{

const size_t VTexturesManager::MAX_TEXTURES_ARRAY_SIZE = 128; //Number of texture2DArray
const size_t VTexturesManager::MAX_LAYER_PER_TEXTURE = 64; //Number of layers in each texture2DArray

VTexturesManager::VTexturesManager()
{
}

VTexturesManager::~VTexturesManager()
{
    this->cleanup();
}

bool VTexturesManager::allocTexture(uint32_t width, uint32_t height, VBuffer source, CommandPoolName cmdPoolName, VTexture *texture)
{
    return VTexturesManager::instance()->allocTextureImpl(width, height, source, cmdPoolName, texture);
}


bool VTexturesManager::allocTextureImpl(uint32_t width, uint32_t height, VBuffer source, CommandPoolName cmdPoolName, VTexture *texture)
{
    m_createImageMutex.lock();
    auto foundedArrays = m_extentToArray.equal_range({width, height});
    size_t chosenArray = MAX_TEXTURES_ARRAY_SIZE;

    for(auto ar = foundedArrays.first ; ar != foundedArrays.second ; ++ar)
    {
        if(!m_allocatedTextureArrays[ar->second]->availableLayers.empty())
        {
            chosenArray = ar->second;
            break;
        }
    }

    if(chosenArray == MAX_TEXTURES_ARRAY_SIZE )
    {
        chosenArray = this->createTextureArray(width, height);
        if(chosenArray == MAX_TEXTURES_ARRAY_SIZE)
            return (false);
    }
    m_createImageMutex.unlock();

    VTexture2DArray *texture2DArray = m_allocatedTextureArrays[chosenArray];
    size_t chosenLayer = *texture2DArray->availableLayers.begin();
    texture2DArray->availableLayers.pop_front();

    texture->m_textureId = chosenArray;
    texture->m_textureLayer = chosenLayer;

    texture2DArray->mutex.lock();

    VulkanHelpers::transitionImageLayout(texture2DArray->image,chosenLayer, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,cmdPoolName);
        VBuffersAllocator::copyBufferToImage(source, texture2DArray->image,
                                         width, height,chosenLayer,cmdPoolName);
    VulkanHelpers::transitionImageLayout(texture2DArray->image,chosenLayer, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cmdPoolName);

    texture2DArray->mutex.unlock();

    return (true);
}

size_t VTexturesManager::createTextureArray(uint32_t width, uint32_t height)
{
   // std::lock_guard<std::mutex> lock(m_createImageMutex);

    //if(m_curTextArrayId >= TEXTURES_ARRAY_SIZE)
    if(m_allocatedTextureArrays.size() >= MAX_TEXTURES_ARRAY_SIZE)
        return (MAX_TEXTURES_ARRAY_SIZE);

    VTexture2DArray *texture2DArray = new VTexture2DArray();

    texture2DArray->layerCount = MAX_LAYER_PER_TEXTURE;
    texture2DArray->extent.width = width;
    texture2DArray->extent.height = height;

    if(!VulkanHelpers::createImage(width, height,texture2DArray->layerCount, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               texture2DArray->image, texture2DArray->memory))
                               return (false);

    texture2DArray->view = VulkanHelpers::createImageView(texture2DArray->image, VK_FORMAT_R8G8B8A8_UNORM,
                                                          VK_IMAGE_ASPECT_COLOR_BIT,texture2DArray->layerCount);

    for(size_t i = 0 ; i < MAX_LAYER_PER_TEXTURE ; ++i)
        texture2DArray->availableLayers.push_back(i);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = nullptr;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture2DArray->view;

    m_imageInfos[m_allocatedTextureArrays.size()] = imageInfo;

    m_extentToArray.insert({{width, height}, m_allocatedTextureArrays.size()});
    m_allocatedTextureArrays.push_back(texture2DArray);

    for(auto b : m_needToUpdateDescSet)
        b = true;
    for(auto b : m_needToUpdateImgDescSet)
        b = true;

    return m_allocatedTextureArrays.size()-1;
}


void VTexturesManager::freeTexture(VTexture &texture)
{
    VTexturesManager::instance()->freeTextureImpl(texture);
}

void VTexturesManager::freeTextureImpl(VTexture &texture)
{
    if(!(texture.m_textureId == 0 && texture.m_textureLayer == 0))
    {
        m_allocatedTextureArrays[texture.m_textureId]->availableLayers.push_back(texture.m_textureLayer);
        texture.m_textureId = 0;
        texture.m_textureLayer = 0;
    }
}


VkSampler VTexturesManager::sampler()
{
    return VTexturesManager::instance()->m_sampler;
}

VkDescriptorSetLayout VTexturesManager::descriptorSetLayout()
{
    return VTexturesManager::instance()->getDescriptorSetLayout();
}

VkDescriptorSet VTexturesManager::descriptorSet(size_t frameIndex)
{
    return VTexturesManager::instance()->getDescriptorSet(frameIndex);
}

VkDescriptorSet VTexturesManager::imgDescriptorSet(size_t imageIndex)
{
    return VTexturesManager::instance()->getImgDescriptorSet(imageIndex);
}

size_t VTexturesManager::descriptorSetVersion(size_t frameIndex)
{
    return VTexturesManager::instance()->getDescriptorSetVersion(frameIndex);
}

size_t VTexturesManager::imgDescriptorSetVersion(size_t imageIndex)
{
    return VTexturesManager::instance()->getImgDescriptorSetVersion(imageIndex);
}

VkDescriptorSetLayout VTexturesManager::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet VTexturesManager::getDescriptorSet(size_t frameIndex)
{
    return m_descriptorSets[frameIndex];
}


VkDescriptorSet VTexturesManager::getImgDescriptorSet(size_t imageIndex)
{
    return m_imgDescriptorSets[imageIndex];
}

size_t VTexturesManager::getDescriptorSetVersion(size_t frameIndex)
{
    return m_descSetVersion[frameIndex];
}

size_t VTexturesManager::getImgDescriptorSetVersion(size_t imageIndex)
{
    return m_imgDescSetVersion[imageIndex];
}

void VTexturesManager::checkUpdateDescriptorSets(size_t frameIndex, size_t imageIndex)
{
    if(m_needToUpdateDescSet[frameIndex] == true)
        this->updateDescriptorSet(frameIndex);
    if(m_needToUpdateImgDescSet[imageIndex] == true)
        this->updateImgDescriptorSet(imageIndex);
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
    layoutBindings[1].descriptorCount = MAX_TEXTURES_ARRAY_SIZE;
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
    samplerInfo.maxAnisotropy = Config::getInt("Graphics","AnisotropicFiltering",VApp::DEFAULT_ANISOTROPIC);
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

bool VTexturesManager::createDescriptorPool(size_t framesCount, size_t imagesCount)
{
    VkDescriptorPoolSize poolSize[2];
    poolSize[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSize[0].descriptorCount = static_cast<uint32_t>(framesCount+imagesCount);
    poolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(framesCount+imagesCount);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(framesCount+imagesCount);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool VTexturesManager::createDescriptorSets(size_t framesCount, size_t imagesCount)
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayout> layouts(imagesCount, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.pSetLayouts = layouts.data();

    allocInfo.descriptorSetCount = static_cast<uint32_t>(framesCount);
    m_descSetVersion = std::vector<size_t> (framesCount, 0);
    m_descriptorSets.resize(framesCount);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets.data()) != VK_SUCCESS)
        return (false);

    allocInfo.descriptorSetCount = static_cast<uint32_t>(imagesCount);
    m_imgDescSetVersion = std::vector<size_t> (imagesCount, 0);
    m_imgDescriptorSets.resize(imagesCount);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_imgDescriptorSets .data()) != VK_SUCCESS)
        return (false);

    for(size_t i = 0 ; i < framesCount ; ++i)
        this->updateDescriptorSet(i);

    for(size_t i = 0 ; i < imagesCount ; ++i)
        this->updateImgDescriptorSet(i);

    return (true);
}

void VTexturesManager::updateDescriptorSet(size_t frameIndex)
{
    this->writeDescriptorSet(m_descriptorSets[frameIndex]);
    m_needToUpdateDescSet[frameIndex] = false;
    ++m_descSetVersion[frameIndex];
}

void VTexturesManager::updateImgDescriptorSet(size_t imageIndex)
{
    this->writeDescriptorSet(m_imgDescriptorSets[imageIndex]);
    m_needToUpdateImgDescSet[imageIndex] = false;
    ++m_imgDescSetVersion[imageIndex];
}

void VTexturesManager::writeDescriptorSet(VkDescriptorSet &descSet)
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
	setWrites[0].dstSet = descSet;
	setWrites[0].pBufferInfo = 0;
	setWrites[0].pImageInfo = &samplerInfo;

	setWrites[1] = {};
	setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[1].dstBinding = 1;
	setWrites[1].dstArrayElement = 0;
	setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	setWrites[1].descriptorCount = MAX_TEXTURES_ARRAY_SIZE;
	setWrites[1].pBufferInfo = 0;
	setWrites[1].dstSet = descSet;
	setWrites[1].pImageInfo =  m_imageInfos.data();

    vkUpdateDescriptorSets(VInstance::device(), 2, setWrites, 0, nullptr);
}

bool VTexturesManager::createDummyTexture()
{
    unsigned char dummyTexturePtr[4] = {255,255,255,255};
    return m_dummyTexture.generateTexture(dummyTexturePtr,1,1);
}

bool VTexturesManager::init(size_t framesCount, size_t imagesCount)
{
    m_imageInfos.resize(MAX_TEXTURES_ARRAY_SIZE);

    if(!this->createDummyTexture())
        return (false);

    m_needToUpdateDescSet = std::vector<bool> (framesCount, true);
    m_needToUpdateImgDescSet = std::vector<bool> (imagesCount, true);

    size_t i = 0;
    for(auto &imageInfo : m_imageInfos)
    {
        imageInfo.sampler = nullptr;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_allocatedTextureArrays[0]->view;
        i++;
    }

    if(!this->createDescriptorSetLayouts())
        return (false);
    if(!this->createSampler())
        return (false);
    if(!this->createDescriptorPool(framesCount,imagesCount))
        return (false);
    if(!this->createDescriptorSets(framesCount,imagesCount))
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
        vkDestroyImageView(device, vtexture->view, nullptr);
        vkDestroyImage(device, vtexture->image, nullptr);
        vkFreeMemory(device, vtexture->memory, nullptr);
        delete vtexture;
    }
    m_allocatedTextureArrays.clear();
    m_extentToArray.clear();
}

}
