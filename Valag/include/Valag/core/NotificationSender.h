#ifndef NOTIFICATIONSENDER_H
#define NOTIFICATIONSENDER_H

#include <set>
#include <map>

#include "Valag/Types.h"

namespace vlg
{

class NotificationListener;

class NotificationSender
{
    public:
        NotificationSender();
        virtual ~NotificationSender();

        void askForAllNotifications(NotificationListener *);
        void askForNotification(NotificationListener *, NotificationType);

        void removeFromNotificationList(NotificationListener *, NotificationType);
        void removeFromAllNotificationList(NotificationListener *);

    protected:
        void sendNotification(NotificationType);

    private:
        std::set<NotificationListener*> m_listenerToNotifyEverything;
        std::map<NotificationType, std::set<NotificationListener*> > m_listenerToNotify;
};

}

#endif // NOTIFICATIONSENDER_H
