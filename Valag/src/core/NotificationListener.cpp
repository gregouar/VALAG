#include "Valag/core/NotificationListener.h"
#include "Valag/core/NotificationSender.h"


namespace vlg
{

NotificationListener::NotificationListener()
{
    //ctor
}

NotificationListener::~NotificationListener()
{
    /*std::list<NotificationSender*>::iterator senderIt;
    for(senderIt = m_senders.begin() ; senderIt != m_senders.end() ; ++senderIt)
        (*senderIt)->removeFromAllNotificationList(this);*/
    for(auto sender : m_senders)
        sender->removeFromAllNotificationList(this);
}

void NotificationListener::addSender(NotificationSender* sender)
{
    /*std::list<NotificationSender*>::iterator senderIt;
    senderIt = std::find(m_senders.begin(), m_senders.end(), sender);
    if(senderIt == m_senders.end())
        m_senders.push_back(sender);**/
    m_senders.insert(sender);
}

void NotificationListener::notifySenderDestruction(NotificationSender* sender)
{
    m_senders.erase(sender);
}

void NotificationListener::stopListeningTo(NotificationSender* sender)
{
    if(sender != nullptr)
    {
        /*std::list<NotificationSender*>::iterator senderIt;
        senderIt = std::find(m_senders.begin(), m_senders.end(), sender);
        if(senderIt != m_senders.end())
            m_senders.erase(senderIt);*/

        m_senders.erase(sender);
        sender->removeFromAllNotificationList(this);
    }
}

}
