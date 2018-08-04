#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include <GLFW/glfw3.h>

#include <stack>
#include <glm/vec2.hpp>

class EventsManager
{
    public:
        EventsManager();
        virtual ~EventsManager();

        void init(GLFWwindow *window);

        void update();

        bool keyPressed(int key)    const;
        bool keyIsPressed(int key)  const;
        bool keyReleased(int key)   const;

        bool mouseButtonPressed(int button)     const;
        bool mouseButtonIsPressed(int button)   const;
        bool mouseButtonReleased(int button)    const;

        glm::vec2 mousePosition()   const;
        glm::vec2 mouseScroll()     const;

        bool isAskingToClose() const;

    protected:
        void updateKey(int key, int action);
        void updateMouseButton(int button, int action);
        void updateMouseScroll(double xoffset, double yoffset);
        void updateMousePosition(double xpos, double ypos);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

    private:
        GLFWwindow *m_window;

        bool m_keyPressed[GLFW_KEY_LAST+1];
        bool m_keyIsPressed[GLFW_KEY_LAST+1];
        bool m_keyReleased[GLFW_KEY_LAST+1];

        std::stack<int> m_justPressedKeys, m_justReleasedKeys,
                        m_justPressedMouseButtons, m_justReleasedMouseButtons;

        bool m_mouseButtonPressed[GLFW_MOUSE_BUTTON_LAST+1];
        bool m_mouseButtonIsPressed[GLFW_MOUSE_BUTTON_LAST+1];
        bool m_mouseButtonReleased[GLFW_MOUSE_BUTTON_LAST+1];

        glm::vec2 m_mousePosition;
        glm::vec2 m_mouseScroll;

        bool m_askingToClose;
};

#endif // EVENTSMANAGER_H
