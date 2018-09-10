#ifndef NOTIFICATIONLISTENER_H
#define NOTIFICATIONLISTENER_H

#include <set>

#include "Valag/Types.h"

namespace vlg
{

class NotificationSender;

class NotificationListener
{
    friend class NotificationSender;

    public:
        NotificationListener();
        virtual ~NotificationListener();

        void stopListeningTo(NotificationSender*);

    protected:
        virtual void notify(NotificationSender*, NotificationType) = 0;

    private:
        void addSender(NotificationSender*);
        void notifySenderDestruction(NotificationSender*);

        std::set<NotificationSender*> m_senders;
};

}

#endif // NOTIFICATIONLISTENER_H
