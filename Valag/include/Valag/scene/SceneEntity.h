#ifndef SCENEENTITY_H
#define SCENEENTITY_H

#include "Valag/scene/SceneObject.h"

/** Any scene object that can be rendered on screen **/

namespace vlg
{

class SceneRenderingInstance;


///Maybe this should be renamed in EntityObject
class SceneEntity : public SceneObject
{
    public:
        SceneEntity();
        virtual ~SceneEntity();

        bool isVisible();
        void setVisible(bool = true);

        //virtual void draw(SceneRenderer *renderer) = 0;
        virtual void generateRenderingData(SceneRenderingInstance *renderingInstance) = 0;

    protected:
        bool m_isVisible;
};

}

#endif // SCENEENTITY_H
