#ifndef MESHESHANDLER_H
#define MESHESHANDLER_H

#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/MeshAsset.h"

namespace vlg
{

class MeshesHandler : public AssetHandler<MeshAsset>
{
    public:
        MeshesHandler();
        virtual ~MeshesHandler();

        static MeshAsset *makeQuad(glm::vec2 corner, glm::vec2 extent,
                                    AssetTypeId material,
                                    glm::vec2 texCorner = {0,0}, glm::vec2 texExtent = {1.0,1.0});

        static MeshAsset *makeMesh(std::vector<std::tuple<glm::vec3, glm::vec2, glm::vec3> > &vertexList,
                                    std::vector<uint16_t> &indexList,
                                    AssetTypeId material);

    protected:

    private:
};

}

#endif // MESHESHANDLER_H
