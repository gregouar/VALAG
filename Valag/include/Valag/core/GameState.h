#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Valag/Types.h"
#include "Valag/core/EventsManager.h"
#include "Valag/gfx/DefaultRenderer.h"

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
        virtual void update(const Time &elapsedTime) = 0;
        virtual void draw(DefaultRenderer *renderer  /**sf::RenderTarget* **/) = 0;

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


