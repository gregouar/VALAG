#include "Valag/utils/SingletonsCleaner.h"


SingletonsCleaner SingletonsCleaner::m_instance=SingletonsCleaner();

SingletonsCleaner* SingletonsCleaner::instance(void)
{
    return &m_instance;
}

void SingletonsCleaner::addToList(KillableSingleton* singleton)
{
    SingletonsCleaner::instance()->m_singletonsList.push_back(singleton);
    SingletonsCleaner::instance()->m_singletonsList.unique();
}

void SingletonsCleaner::cleanAll()
{
    while(!SingletonsCleaner::instance()->m_singletonsList.empty())
        SingletonsCleaner::instance()->m_singletonsList.back()->kill();
}

void SingletonsCleaner::removeFromList(KillableSingleton* singleton)
{
    SingletonsCleaner *sc = SingletonsCleaner::instance();

    std::list<KillableSingleton*>::iterator singletonIterator;
    singletonIterator = sc->m_singletonsList.begin();
    while(singletonIterator != sc->m_singletonsList.end())
    {
        if(*singletonIterator == singleton)
        {
            sc->m_singletonsList.erase(singletonIterator);
            singletonIterator = sc->m_singletonsList.end();
        } else
            ++singletonIterator;
    }
}


