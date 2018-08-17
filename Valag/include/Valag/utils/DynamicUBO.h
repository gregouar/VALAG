#ifndef DYNAMICUBO_H
#define DYNAMICUBO_H

#include <list>

#include "Valag/gfx/VInstance.h"

namespace vlg
{

class DynamicUBO
{
    public:
        DynamicUBO(size_t objectSize, size_t chunkSize );
        virtual ~DynamicUBO();

        bool    allocObject(size_t &index);
        bool    freeObject(size_t index);
        bool    isFull();

        bool    updateObject(size_t index, void *data);

        void expandBuffers(bool destroyOldBUffers = true);

        uint32_t getDynamicOffset(size_t index);

        VkBuffer        getBuffer();
        VkDeviceMemory  getBufferMemory();
        size_t          getBufferVersion();

    protected:
        void computeDynamicAlignment();
        void createBuffers();
        void cleanup();

    private:
        size_t m_objectSize;
        size_t m_chunkSize;
        size_t m_dynamicAlignment;

        VkBuffer        m_buffer;
        VkDeviceSize    m_bufferSize;
        VkDeviceMemory  m_bufferMemory;
        size_t          m_totalSize;

        std::list<size_t> m_availableIndices;

        size_t m_bufferVersion;
        bool m_firstTime;
};

}

#endif // DYNAMICUBO_H
