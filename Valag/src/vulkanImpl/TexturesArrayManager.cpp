#include "Valag/vulkanImpl/TexturesArrayManager.h"

#include "Valag/core/VApp.h"
#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"

namespace vlg
{

const size_t TexturesArrayManager::TEXTURES_ARRAY_SIZE = 128;

TexturesArrayManager::TexturesArrayManager()
{
    this->init();
}

TexturesArrayManager::~TexturesArrayManager()
{
    this->cleanup();
}

bool TexturesArrayManager::bindTexture(AssetTypeID id, size_t frameIndex, int *texArrayID)
{
    auto imageIt = m_texturesArray.find(id);
    if(imageIt == m_texturesArray.end())
    {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = nullptr;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = TextureHandler::instance()->getAsset(id)->getImageView();

        m_texturesToAdd.push_back({id, m_availableImageInfos.front()});

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
}

VkDescriptorSetLayout TexturesArrayManager::getDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet TexturesArrayManager::getDescriptorSet(size_t frameIndex)
{
    return m_descriptorSets[m_descriptorSetsNbr[frameIndex]][frameIndex];
}

void TexturesArrayManager::checkUpdateDescriptorSets(size_t frameIndex)
{
    for(size_t i = 0 ; i < 2 ; ++i) {
        if(m_needToUpdateDescSet[frameIndex] == true)
        {
            m_descriptorSetsNbr[frameIndex] = !m_descriptorSetsNbr[frameIndex];
            for(auto tex : m_texturesToAdd)
                m_texturesArray[tex.first] = tex.second;
            m_texturesToAdd.clear();
            this->updateDescriptorSet(frameIndex);
        }
        frameIndex = (frameIndex + 1) % VApp::MAX_FRAMES_IN_FLIGHT;
    }
}

bool TexturesArrayManager::createDescriptorSetLayouts()
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

bool TexturesArrayManager::createSampler()
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

bool TexturesArrayManager::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize[2];
    poolSize[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSize[0].descriptorCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT*2);
    poolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT*2);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT*2 /* *2 */);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool TexturesArrayManager::createDescriptorSets()
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayout> layouts(VApp::MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSetsNbr.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_descriptorSets[0].resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_descriptorSets[1].resize(VApp::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets[0].data()) != VK_SUCCESS)
        return (false);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets[1].data()) != VK_SUCCESS)
        return (false);

    /*for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_viewBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ViewUBO);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_viewDescriptorSets[i];
        descriptorWrite.dstBinding = 0; //Bind number
        descriptorWrite.dstArrayElement = 0; //Set number
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }*/

    /*for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        this->updateDescriptorSet(i);*/

    return (true);
}

void TexturesArrayManager::updateDescriptorSet(size_t frameIndex)
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
	setWrites[0].dstSet = m_descriptorSets[m_descriptorSetsNbr[frameIndex]][frameIndex];
	setWrites[0].pBufferInfo = 0;
	setWrites[0].pImageInfo = &samplerInfo;

	setWrites[1] = {};
	setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[1].dstBinding = 1;
	setWrites[1].dstArrayElement = 0;
	setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	setWrites[1].descriptorCount = TEXTURES_ARRAY_SIZE; /// static_cast<uint32_t>(/*m_texturesArray.size()*/m_imageInfos[frameIndex].size());//TEXTURES_ARRAY_SIZE;
	setWrites[1].pBufferInfo = 0;
	setWrites[1].dstSet = m_descriptorSets[m_descriptorSetsNbr[frameIndex]][frameIndex];
	setWrites[1].pImageInfo =  m_imageInfos.data();

    vkUpdateDescriptorSets(VInstance::device(), 2, setWrites, 0, nullptr);

    m_needToUpdateDescSet[frameIndex] = false;
}

bool TexturesArrayManager::init()
{
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
        imageInfo.imageView = TextureHandler::instance()->getDummyAsset()->getImageView();

        m_availableImageInfos.push_back(i++);
    }

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

void TexturesArrayManager::cleanup()
{
    VkDevice device = VInstance::device();

    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyDescriptorPool(device,m_descriptorPool,nullptr);
    vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
}

}
