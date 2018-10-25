#include "Valag/vulkanImpl/DynamicVBO.h"

namespace vlg
{

template<typename T>
DynamicVBO<T>::DynamicVBO(size_t chunkSize) :
    m_chunkSize(chunkSize),
    m_curSize(0),
    m_lastSize(0)
{
    //ctor
}

template<typename T>
DynamicVBO<T>::~DynamicVBO()
{
    this->cleanup();
}

template<typename T>
void DynamicVBO<T>::push_back(const T &datum)
{
    if(m_curSize == m_content.size())
        this->expand();

    m_content[m_curSize++] = datum;
}

template<typename T>
size_t DynamicVBO<T>::uploadVBO()
{
    if(m_curSize != 0)
        VBuffersAllocator::writeBuffer(m_buffer,m_content.data(),
                                       m_curSize*sizeof(T),false);

    m_lastSize = m_curSize;
    m_curSize = 0;
    return m_lastSize;
}


template<typename T>
size_t DynamicVBO<T>::getSize()
{
    return m_curSize;
}

template<typename T>
size_t DynamicVBO<T>::getUploadedSize()
{
    return m_lastSize;
}


template<typename T>
VBuffer DynamicVBO<T>::getBuffer()
{
    return m_buffer;
}


template<typename T>
bool DynamicVBO<T>::expand()
{
    VBuffer oldBuffer = m_buffer;
    m_content.resize(m_content.size() + m_chunkSize);

    VkDeviceSize bufferSize = m_content.size() * sizeof(T);
    if(!VBuffersAllocator::allocBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                          , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,m_buffer))
        return (false);

    if(m_curSize != 0)
    {
        VBuffersAllocator::copyBuffer(oldBuffer,m_buffer,m_curSize*sizeof(T));
        VBuffersAllocator::freeBuffer(oldBuffer);
    }

    return (true);
}

template<typename T>
void DynamicVBO<T>::cleanup()
{
    m_curSize = 0;
    m_content.clear();
    VBuffersAllocator::freeBuffer(m_buffer);
}

}
