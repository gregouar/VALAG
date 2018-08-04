#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Valag/core/EventsManager.h"

namespace vlg
{

class StatesManager;

class GameState
{
    public:
        GameState();
        virtual ~GameState();

        virtual void entered() = 0;
        virtual void leaving() = 0;
        virtual void revealed() = 0;
        virtual void obscuring() = 0;

        virtual void handleEvents(const EventsManager *eventsManager) = 0;
        virtual void update(/**sf::Time**/) = 0;
        virtual void draw(/**sf::RenderTarget* **/) = 0;

        void setManager(StatesManager *);

        void pause();
        void resume();

    protected:
        StatesManager *m_manager;

    private:
        bool m_running;
};

}

#endif // GAMESTATE_H


