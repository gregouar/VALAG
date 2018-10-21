#ifndef DYNAMICUBODESCRIPTOR_H
#define DYNAMICUBODESCRIPTOR_H

#include "Valag/vulkanImpl/DynamicUBO.h"

///Maybe this class is to specific to be in vulkanImpl

namespace vlg
{

class DynamicUBODescriptor
{
    public:
        DynamicUBODescriptor(size_t objectSize, size_t chunkSize);
        virtual ~DynamicUBODescriptor();

        DynamicUBODescriptor( const DynamicUBODescriptor& ) = delete;
        DynamicUBODescriptor& operator=( const DynamicUBODescriptor& ) = delete;

        bool init(size_t framesCount);
        void update(size_t frameIndex); //Expands UBOs
        void cleanup();

        bool allocObject(size_t frameIndex, size_t &index);
        bool freeObject(size_t frameIndex, size_t index);
        bool updateObject(size_t frameIndex, size_t index, void *data);

        size_t getBufferVersion(size_t frameIndex);

        VkDescriptorSetLayout   getDescriptorSetLayout();
        VkDescriptorSet         getDescriptorSet(size_t frameIndex);
        uint32_t                getDynamicOffset(size_t frameIndex, size_t index);

    protected:
        bool    createDescriptorSetLayouts();
        bool    createDescriptorPool(size_t framesCount);
        bool    createDescriptorSets(size_t framesCount);

        void    updateDescriptorSets(size_t frameIndex);

    private:
        std::vector<bool>               m_needToExpandBuffers;
        std::vector<DynamicUBO*>        m_buffers;
        VkDescriptorSetLayout           m_descriptorSetLayout;
        VkDescriptorPool                m_descriptorPool;
        std::vector<VkDescriptorSet>    m_descriptorSets;

        size_t m_objectSize;
        size_t m_chunkSize;

};

}

#endif // DYNAMICUBODESCRIPTOR_H
