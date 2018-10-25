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
    glm::vec3 T = {1,0,0}, B = {0,1,0}, N = {0,0,1};
    std::vector<MeshVertex> vertexList(4);
    vertexList[0] = {{corner.x,corner.y+extent.y,0.0},  {texCorner.x, texCorner.y+texExtent.y}, N,T,B};
    vertexList[1] = {{corner.x,corner.y,0.0}         ,  {texCorner.x, texCorner.y}            , N,T,B};
    vertexList[2] = {{corner.x+extent.x,corner.y,0.0},  {texCorner.x+texExtent.x, texCorner.y}, N,T,B};
    vertexList[3] = {{corner.x+extent.x,corner.y+extent.y,0.0},  {texCorner.x+texExtent.x, texCorner.y+texExtent.y}, N,T,B};
    std::vector<uint16_t> indexList = {0,1,2,2,3,0};

    return MeshesHandler::makeMesh(vertexList, indexList, material);
}

MeshAsset *MeshesHandler::makeBox( glm::vec3 c, glm::vec3 e, MaterialAsset *material)
{
    glm::vec3 X = {1,0,0}, Y = {0,1,0}, Z = {0,0,1};
    std::vector<MeshVertex> vertexList(4*6);

    size_t d = 0;
    vertexList[d+0] = {{c.x,c.y+e.y,c.z+e.z}      ,  {0, 1}, Z,X,Y};
    vertexList[d+1] = {{c.x,c.y,c.z+e.z}          ,  {0, 0}, Z,X,Y};
    vertexList[d+2] = {{c.x+e.x,c.y,c.z+e.z}      ,  {1, 0}, Z,X,Y};
    vertexList[d+3] = {{c.x+e.x,c.y+e.y,c.z+e.z}  ,  {1, 1}, Z,X,Y};

    d += 4;
    vertexList[d+0] = {{c.x,c.y+e.y,c.z+e.z}      ,  {0, 1}, -Z,X,Y};
    vertexList[d+1] = {{c.x+e.x,c.y+e.y,c.z+e.z}  ,  {0, 0}, -Z,X,Y};
    vertexList[d+2] = {{c.x+e.x,c.y,c.z+e.z}      ,  {1, 0}, -Z,X,Y};
    vertexList[d+3] = {{c.x,c.y,c.z+e.z}          ,  {1, 0}, -Z,X,Y};

    d += 4;
    vertexList[d+0] = {{c.x,c.y+e.y,c.z}            ,  {0, 1}, Y,X,Z};
    vertexList[d+1] = {{c.x,c.y+e.y,c.z+e.z}        ,  {1, 1}, Y,X,Z};
    vertexList[d+2] = {{c.x+e.x,c.y+e.y,c.z+e.z}    ,  {1, 0}, Y,X,Z};
    vertexList[d+3] = {{c.x+e.x,c.y+e.y,c.z}        ,  {0, 0}, Y,X,Z};

    d += 4;
    vertexList[d+0] = {{c.x+e.x,c.y+e.y,c.z}    ,  {0, 1}, X,-Y,Z};
    vertexList[d+1] = {{c.x+e.x,c.y+e.y,c.z+e.z},  {1, 1}, X,-Y,Z};
    vertexList[d+2] = {{c.x+e.x,c.y,c.z+e.z}    ,  {1, 0}, X,-Y,Z};
    vertexList[d+3] = {{c.x+e.x,c.y,c.z}        ,  {0, 0}, X,-Y,Z};

    d += 4;
    vertexList[d+0] = {{c.x+e.x,c.y,c.z}    ,  {0, 1}, -Y,-X,Z};
    vertexList[d+1] = {{c.x+e.x,c.y,c.z+e.z},  {1, 1}, -Y,-X,Z};
    vertexList[d+2] = {{c.x,c.y,c.z+e.z}    ,  {1, 0}, -Y,-X,Z};
    vertexList[d+3] = {{c.x,c.y,c.z}        ,  {0, 0}, -Y,-X,Z};

    d += 4;
    vertexList[d+0] = {{c.x,c.y,c.z}        ,  {0, 1}, -X,Y,Z};
    vertexList[d+1] = {{c.x,c.y,c.z+e.z}    ,  {1, 1}, -X,Y,Z};
    vertexList[d+2] = {{c.x,c.y+e.y,c.z+e.z},  {1, 0}, -X,Y,Z};
    vertexList[d+3] = {{c.x,c.y+e.y,c.z}    ,  {0, 0}, -X,Y,Z};


    std::vector<uint16_t> indexList(6*6);// = {0,1,2,2,3,0};

    for(size_t i = 0 ; i < 6 ; ++i)
    {
        indexList[i*6] = i*4;
        indexList[i*6+1] = i*4+1;
        indexList[i*6+2] = i*4+2;
        indexList[i*6+3] = i*4+2;
        indexList[i*6+4] = i*4+3;
        indexList[i*6+5] = i*4;
    }

    return MeshesHandler::makeMesh(vertexList, indexList, material);
}

MeshAsset *MeshesHandler::makeMesh( std::vector<MeshVertex> &vertexList,
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
