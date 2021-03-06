#ifndef CAMERAOBJECT_H
#define CAMERAOBJECT_H

#include "Valag/scene/SceneObject.h"

namespace vlg
{

///Add viewport, etc

class CameraObject : public SceneObject
{
    public:
        CameraObject();
        virtual ~CameraObject();

        void setViewport(glm::vec2 offset, glm::vec2 extent); //Normalized viewport

        void setZoom(float zoom);
        void zoom(float zoomFactor);

        float getZoom();
        glm::vec2 getViewportOffset();
        glm::vec2 getViewportExtent();

    protected:
        float m_zoom;
        glm::vec2 m_offset;
        glm::vec2 m_extent;

    private:
};

}

#endif // CAMERAOBJECT_H
