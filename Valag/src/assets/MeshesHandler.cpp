#include "Valag/assets/MeshesHandler.h"

namespace vlg
{

MeshesHandler::MeshesHandler()
{
    //ctor
}

MeshesHandler::~MeshesHandler()
{
    //dtor
}


MeshAsset *MeshesHandler::makeQuad( glm::vec2 corner, glm::vec2 extent,
                                    MaterialAsset *material,
                                    glm::vec2 texCorner, glm::vec2 texExtent)
{
    std::vector<std::tuple<glm::vec3, glm::vec2, glm::vec3> > vertexList = {
            {{corner.x,corner.y+extent.y,0.0},  {texCorner.x, texCorner.y+texExtent.y}, {0.0,0.0,1.0}},
            {{corner.x,corner.y,0.0},           {texCorner.x, texCorner.y},             {0.0,0.0,1.0}},
            {{corner.x+extent.x,corner.y,0.0},  {texCorner.x+texExtent.x, texCorner.y}, {0.0,0.0,1.0}},
            {{corner.x+extent.x,corner.y+extent.y,0.0},  {texCorner.x+texExtent.x, texCorner.y+texExtent.y}, {0.0,0.0,1.0}}
        };
    std::vector<uint16_t> indexList = {0,1,2,2,3,0};

    return MeshesHandler::makeMesh(vertexList, indexList, material);
}

MeshAsset *MeshesHandler::makeMesh( std::vector<std::tuple<glm::vec3, glm::vec2, glm::vec3> > &vertexList,
                                    std::vector<uint16_t> &indexList,
                                    MaterialAsset *material)
{
    MeshAsset *asset = instance()->addAsset(instance()->generateId());
    asset->setMaterial(material);
    asset->generateModel(vertexList, indexList);
    //asset->forceLoaded(true);
    return asset;
}

}
