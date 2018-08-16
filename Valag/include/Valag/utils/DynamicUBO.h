#ifndef DYNAMICUBO_H
#define DYNAMICUBO_H

#include "Valag/gfx/VInstance.h"

namespace vlg
{

class DynamicUBO
{
    public:
        DynamicUBO(size_t objectSize, size_t chunkSize );
        virtual ~DynamicUBO();

        bool  allocObject(size_t &index);
        bool    freeObject(size_t index);

        bool    updateObject(size_t index, void *data);

        uint32_t getDynamicOffset(size_t index);

        VkBuffer        getBuffer();
        VkDeviceSize    getBufferRange();

    protected:
        void createBuffers(size_t objectSize, size_t chunkSize);
        void expandBuffers();
        void cleanup();

    private:
        size_t m_chunkSize;
        size_t m_objectSize;
        size_t m_dynamicAlignment;

        VkBuffer        m_buffer;
        VkDeviceSize    m_bufferRange;
        VkDeviceMemory  m_bufferMemory;
        size_t          m_totalSize;


        void*   m_localData;

        size_t m_currentIndex; ///Replace by smart thing
};

}

#endif // DYNAMICUBO_H
