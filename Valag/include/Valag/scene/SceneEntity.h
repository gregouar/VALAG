#ifndef SCENEENTITY_H
#define SCENEENTITY_H

#include "Valag/scene/SceneObject.h"

namespace vlg
{

class SceneRenderer;


///Maybe this should be renamed in EntityObject
class SceneEntity : public SceneObject
{
    public:
        SceneEntity();
        virtual ~SceneEntity();

        virtual void draw(SceneRenderer *renderer) = 0;

    protected:

    private:
};

}

#endif // SCENEENTITY_H
