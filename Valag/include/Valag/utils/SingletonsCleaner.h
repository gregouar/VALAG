#ifndef SINGLETONSCLEANER_H
#define SINGLETONSCLEANER_H

#include <list>

class KillableSingleton
{
    friend class SingletonsCleaner;

    protected:
        KillableSingleton(){}
        virtual ~KillableSingleton(){}

    private:
        virtual void kill() = 0;
};

class SingletonsCleaner
{
    public:
        static SingletonsCleaner* instance(void);
        static void addToList(KillableSingleton* singleton);
        static void cleanAll();
        static void removeFromList(KillableSingleton* singleton);

    protected:

        SingletonsCleaner(){}
        virtual ~SingletonsCleaner(){cleanAll();}

        std::list<KillableSingleton*> m_singletonsList;

        static SingletonsCleaner m_instance;
};


#endif // SINGLETONSCLEANER_H
