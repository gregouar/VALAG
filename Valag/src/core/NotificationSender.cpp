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
    for(auto listener : m_listenerToNotifyEverything)
    {
        listener->notify(this, Notification_SenderDestroyed);
        listener->notifySenderDestruction(this);
    }

    for(auto listenerSet : m_listenerToNotify)
        for(auto listener : listenerSet.second)
        {
            listener->notify(this, Notification_SenderDestroyed);
            listener->notifySenderDestruction(this);
        }
}



void NotificationSender::askForAllNotifications(NotificationListener *listener)
{
    auto ret = m_listenerToNotifyEverything.insert(listener);
    if(ret.second == true)
        listener->addSender(this);
}

void NotificationSender::askForNotification(NotificationListener *listener, NotificationType type)
{
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

    for(auto type : m_listenerToNotify)
        this->removeFromNotificationList(listener, type.first);
}


void NotificationSender::sendNotification(NotificationType type)
{
    for(auto listener : m_listenerToNotify[type])
        listener->notify(this, type);

    for(auto listener : m_listenerToNotifyEverything)
        listener->notify(this, type);
}

}
