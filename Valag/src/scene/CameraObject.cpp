#include "Valag/scene/CameraObject.h"

namespace vlg
{

CameraObject::CameraObject() :
    m_zoom(1.0f),
    m_offset(0.0f, 0.0f),
    m_extent(1.0f, 1.0f)
{
    //ctor
}

CameraObject::~CameraObject()
{
    //dtor
}


void CameraObject::setViewport(glm::vec2 offset, glm::vec2 extent)
{
    m_offset = offset;
    m_extent = extent;
}

void CameraObject::setZoom(float zoom)
{
    if(zoom != 0)
        m_zoom = zoom;
}

void CameraObject::zoom(float zoom)
{
    if(zoom != 0)
        m_zoom *= zoom;
}

float CameraObject::getZoom()
{
    return m_zoom;
}

glm::vec2 CameraObject::getOffset()
{
    return m_offset;
}

glm::vec2 CameraObject::getExtent()
{
    return m_extent;
}

}
