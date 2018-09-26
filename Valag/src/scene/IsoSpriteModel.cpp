#include "Valag/scene/IsoSpriteModel.h"

namespace vlg
{

IsoSpriteModel::IsoSpriteModel() :
    m_material(0),
    m_size({1.0f,1.0f}),
    m_texturePosition({0.0f,0.0f}),
    m_textureExtent({1.0f,1.0f}),
    m_textureCenter({0.0f,0.0f})
{
    //ctor
}

IsoSpriteModel::~IsoSpriteModel()
{
    this->cleanup();
}

/*void IsoSpriteModel::setSize(glm::vec2 size)
{
    m_size = size;
}*/

void IsoSpriteModel::setMaterial(AssetTypeId materialId)
{
    m_material = materialId;
}

void IsoSpriteModel::setSize(glm::vec2 size)
{
    m_size = size;
}

void IsoSpriteModel::setTextureRect(glm::vec2 pos, glm::vec2 extent)
{
    m_texturePosition = pos;
    m_textureExtent = extent;
}

void IsoSpriteModel::setTextureCenter(glm::vec2 pos)
{
    m_textureCenter = pos;
}

void IsoSpriteModel::setColor(Color color)
{
    m_color = color;
}

void IsoSpriteModel::setRmt(Color rmt)
{
    m_rmt = rmt;
}


AssetTypeId IsoSpriteModel::getMaterial()
{
    return m_material;
}

glm::vec2 IsoSpriteModel::getSize()
{
    return m_size;
}

glm::vec2 IsoSpriteModel::getTextureExtent()
{
    return m_textureExtent;
}

glm::vec2 IsoSpriteModel::getTexturePosition()
{
    return m_texturePosition;
}

glm::vec2 IsoSpriteModel::getTextureCenter()
{
    return m_textureCenter;
}

/*void IsoSpriteModel::updateModel(SceneRenderer *renderer, size_t frameIndex)
{

}*/

void IsoSpriteModel::cleanup()
{

}


/** Static **/
/*DynamicUBODescriptor IsoSpriteModel::s_modelUBO(sizeof(IsoSpriteModelUBO), 1024);

VkDescriptorSetLayout IsoSpriteModel::getUBODescriptorSetLayout()
{
    return IsoSpriteModel::s_modelUBO.getDescriptorSetLayout();
}*/


}
