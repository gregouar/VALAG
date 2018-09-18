#ifndef DYNAMICVBO_H
#define DYNAMICVBO_H

#include "Valag/vulkanImpl/VBuffersAllocator.h"

namespace vlg
{

template<class T> class DynamicVBO
{
    public:
        DynamicVBO(size_t chunkSize);
        virtual ~DynamicVBO();

        void push_back(const T &datum);

        size_t uploadVBO(); //return buffer size

        size_t  getSize();
        VBuffer getBuffer();

    protected:
        void cleanup();
        bool expand();

    private:
        size_t m_chunkSize;

        size_t m_curSize;
        std::vector<T> m_content;
        VBuffer m_buffer;
};

}
#include "../src/vulkanImpl/DynamicVBO.inc"

#endif // DYNAMICVBO_H
