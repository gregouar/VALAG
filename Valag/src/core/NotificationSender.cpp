#include "Valag/core/NotificationSender.h"
#include "Valag/core/NotificationListener.h"

namespace vlg
{

NotificationSender::NotificationSender()
{
    //ctor
}

NotificationSender::~NotificationSender()
{
    /*std::list<NotificationListener*>::iterator listenerIt;
    for(listenerIt = m_listenerToNotifyEverything.begin() ;
        listenerIt != m_listenerToNotifyEverything.end() ; ++listenerIt)**/
    for(auto listener : m_listenerToNotifyEverything)
        {
            listener->notify(this, Notification_SenderDestroyed);
            listener->notifySenderDestruction(this);
        }

    /*std::map<NotificationType, std::list<NotificationListener*> >::iterator typeIt;
    for(typeIt = m_listenerToNotify.begin() ; typeIt != m_listenerToNotify.end() ; ++typeIt)
        for(listenerIt = typeIt->second.begin() ; listenerIt != typeIt->second.end() ; ++listenerIt)
        {
            (*listenerIt)->notify(this, Notification_SenderDestroyed);
            (*listenerIt)->notifySenderDestruction(this);
        }**/

    for(auto listenerSet : m_listenerToNotify)
        for(auto listener : listenerSet.second)
        {
            listener->notify(this, Notification_SenderDestroyed);
            listener->notifySenderDestruction(this);
        }
}



void NotificationSender::askForAllNotifications(NotificationListener *listener)
{
    /*std::list<NotificationListener*>::iterator listenerIt;
    listenerIt = std::find(m_listenerToNotifyEverything.begin(),
                            m_listenerToNotifyEverything.end(), listener);

    if(listenerIt == m_listenerToNotifyEverything.end())
    {
        m_listenerToNotifyEverything.push_back(listener);
        listener->addSender(this);
    }**/

    auto ret = m_listenerToNotifyEverything.insert(listener);
    if(ret.second == true)
        listener->addSender(this);
}

void NotificationSender::askForNotification(NotificationListener *listener, NotificationType type)
{
    /*std::list<NotificationListener*>::iterator listenerIt;
    listenerIt = std::find(m_listenerToNotify[type].begin(),
                            m_listenerToNotify[type].end(), listener);

    if( listenerIt == m_listenerToNotify[type].end())
    {
        m_listenerToNotify[type].push_back(listener);
        listener->addSender(this);
    }**/

    auto ret = m_listenerToNotify[type].insert(listener);
    if(ret.second == true)
        listener->addSender(this);
}

void NotificationSender::removeFromNotificationList(NotificationListener *listener, NotificationType type)
{
    m_listenerToNotify[type].erase(listener);
}

void NotificationSender::removeFromAllNotificationList(NotificationListener *listener)
{
    m_listenerToNotifyEverything.erase(listener);

    /*std::map<NotificationType, std::list<NotificationListener*> >::iterator typeIt;
    for(typeIt = m_listenerToNotify.begin() ; typeIt != m_listenerToNotify.end() ; ++typeIt)
        this->removeFromNotificationList(listener, typeIt->first);**/

    for(auto type : m_listenerToNotify)
        this->removeFromNotificationList(listener, type.first);
}


void NotificationSender::sendNotification(NotificationType type)
{
    /*std::list<NotificationListener*>::iterator listenerIt;
    for(listenerIt = m_listenerToNotify[type].begin() ;
        listenerIt != m_listenerToNotify[type].end() ; ++listenerIt)
        (*listenerIt)->notify(this, type);

    for(listenerIt = m_listenerToNotifyEverything.begin() ;
        listenerIt != m_listenerToNotifyEverything.end() ; ++listenerIt)
        (*listenerIt)->notify(this, type);**/

    for(auto listener : m_listenerToNotify[type])
        listener->notify(this, type);

    for(auto listener : m_listenerToNotifyEverything)
        listener->notify(this, type);
}

}
