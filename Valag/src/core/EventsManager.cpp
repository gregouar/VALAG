#include "Valag/core/EventsManager.h"

EventsManager::EventsManager()
{
    for(auto i = 0 ; i <= GLFW_KEY_LAST ; ++i)
    {
        m_keyIsPressed[i]   = false;
        m_keyPressed[i]     = false;
        m_keyReleased[i]    = false;
    }

    for(auto i = 0 ; i <= GLFW_MOUSE_BUTTON_LAST ; ++i)
    {
        m_mouseButtonPressed[i]     = false;
        m_mouseButtonIsPressed[i]   = false;
        m_mouseButtonReleased[i]    = false;
    }

    m_mousePosition = glm::vec2(0,0);
    m_mouseScroll   = glm::vec2(0,0);

    m_askingToClose = false;
}

EventsManager::~EventsManager()
{
    //dtor
}

void EventsManager::init(GLFWwindow *window)
{
    m_window = window;

    glfwSetWindowUserPointer(m_window, this);

    glfwSetKeyCallback          (m_window, EventsManager::key_callback);
    glfwSetMouseButtonCallback  (m_window, EventsManager::mouse_button_callback);
    glfwSetScrollCallback       (m_window, EventsManager::scroll_callback);
    glfwSetCursorPosCallback    (m_window, EventsManager::cursor_position_callback);
}

void EventsManager::update()
{
    while(!m_justPressedKeys.empty())
    {
        m_keyPressed[m_justPressedKeys.top()] = false;
        m_justPressedKeys.pop();
    }

    while(!m_justReleasedKeys.empty())
    {
        m_keyReleased[m_justReleasedKeys.top()] = false;
        m_justReleasedKeys.pop();
    }

    while(!m_justPressedMouseButtons.empty())
    {
        m_mouseButtonPressed[m_justPressedMouseButtons.top()] = false;
        m_justPressedMouseButtons.pop();
    }

    while(!m_justReleasedMouseButtons.empty())
    {
        m_mouseButtonReleased[m_justReleasedMouseButtons.top()] = false;
        m_justReleasedMouseButtons.pop();
    }

    m_mouseScroll = glm::vec2(0,0);

    m_askingToClose = glfwWindowShouldClose(m_window);

    glfwPollEvents();
}

bool EventsManager::keyPressed(int key) const
{
    if(key >= 0 && key <= GLFW_KEY_LAST)
        return m_keyPressed[key];
    return (false);
}

bool EventsManager::keyIsPressed(int key) const
{
    if(key >= 0 && key <= GLFW_KEY_LAST)
        return m_keyIsPressed[key] | m_keyPressed[key];
    return (false);
}

bool EventsManager::keyReleased(int key) const
{
    if(key >= 0 && key <= GLFW_KEY_LAST)
        return m_keyReleased[key];
    return (false);
}

bool EventsManager::mouseButtonPressed(int button) const
{
    if(button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
        return m_mouseButtonPressed[button];
    return (false);
}

bool EventsManager::mouseButtonIsPressed(int button) const
{
    if(button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
        return m_mouseButtonIsPressed[button] | m_mouseButtonPressed[button];
    return (false);
}
bool EventsManager::mouseButtonReleased(int button) const
{
    if(button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
        return m_mouseButtonReleased[button];
    return (false);
}

glm::vec2 EventsManager::mousePosition() const
{
    return m_mousePosition;
}

glm::vec2 EventsManager::mouseScroll() const
{
    return m_mouseScroll;
}

bool EventsManager::isAskingToClose() const
{
    return m_askingToClose;
}



void EventsManager::updateKey(int key, int action)
{
    if(key >= 0 && key <= GLFW_KEY_LAST)
    {
        if(action == GLFW_PRESS)
        {
            m_keyPressed[key]   = true;
            m_keyIsPressed[key] = true;
            m_justPressedKeys.push(key);
        }
        else if(action == GLFW_RELEASE)
        {
            m_keyReleased[key]  = true;
            m_keyIsPressed[key] = false;
            m_justReleasedKeys.push(key);
        }
    }
}

void EventsManager::updateMouseButton(int button, int action)
{
    if(button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
    {
        if(action == GLFW_PRESS)
        {
            m_mouseButtonPressed[button] = true;
            m_mouseButtonIsPressed[button] = true;
            m_justPressedMouseButtons.push(button);
        }
        else if(action == GLFW_RELEASE)
        {
            m_mouseButtonReleased[button]  = true;
            m_mouseButtonIsPressed[button] = false;
            m_justReleasedMouseButtons.push(button);
        }
    }
}

void EventsManager::updateMouseScroll(double xoffset, double yoffset)
{
    m_mouseScroll = glm::vec2(xoffset, yoffset);
}

void EventsManager::updateMousePosition(double xpos, double ypos)
{
    m_mousePosition = glm::vec2(xpos, ypos);
}



void EventsManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    EventsManager *eventsManager =
      static_cast<EventsManager*>(glfwGetWindowUserPointer(window));

    if(eventsManager != nullptr)
        eventsManager->updateKey(key, action);
}

void EventsManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    EventsManager *eventsManager =
      static_cast<EventsManager*>(glfwGetWindowUserPointer(window));

    if(eventsManager != nullptr)
        eventsManager->updateMouseButton(button, action);
}

void EventsManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    EventsManager *eventsManager =
      static_cast<EventsManager*>(glfwGetWindowUserPointer(window));

    if(eventsManager != nullptr)
        eventsManager->updateMouseScroll(xoffset, yoffset);
}

void EventsManager::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    EventsManager *eventsManager =
      static_cast<EventsManager*>(glfwGetWindowUserPointer(window));

    if(eventsManager != nullptr)
        eventsManager->updateMousePosition(xpos, ypos);
}

