#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED

#include <iostream>
#include <mutex>
#include <atomic>
#include "SingletonsCleaner.h"

template<typename T> class Singleton : public KillableSingleton
{
    public:

        static T* instance()
        {
            T* tmp = m_instance.load(std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_acquire);
            if (tmp == nullptr) {
                std::lock_guard<std::mutex> lock(m_mutex);
                tmp = m_instance.load(std::memory_order_relaxed);
                if (tmp == nullptr) {
                    tmp = new T;
                    std::atomic_thread_fence(std::memory_order_release);
                    m_instance.store(tmp, std::memory_order_relaxed);
                    SingletonsCleaner::addToList(tmp);
                }
            }
            return tmp;
        }

    protected:

        Singleton(){}
        virtual ~Singleton(){}

        static std::atomic<T*> m_instance;
        static std::mutex m_mutex;

    private:

        virtual void kill()
        {
            T* tmp = m_instance.load(std::memory_order_relaxed);
            if(tmp != nullptr)
            {
                SingletonsCleaner::removeFromList(tmp);
                delete tmp;
                m_instance.store(nullptr);
            }
        }
};

template <typename T> std::atomic<T*> Singleton<T>::m_instance;
template <typename T> std::mutex Singleton<T>::m_mutex;


#endif // SINGLETON_H_INCLUDED
