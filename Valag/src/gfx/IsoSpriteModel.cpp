#include "Valag/gfx/IsoSpriteModel.h"

namespace vlg
{

IsoSpriteModel::IsoSpriteModel() :
    m_texture(0),
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

void IsoSpriteModel::setTexture(AssetTypeID textureID)
{
    m_texture = textureID;
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

AssetTypeID IsoSpriteModel::getTexture()
{
    return m_texture;
}

void IsoSpriteModel::updateModel(SceneRenderer *renderer, size_t frameIndex)
{

}

void IsoSpriteModel::cleanup()
{

}


/** Static **/
DynamicUBODescriptor IsoSpriteModel::s_modelUBO(sizeof(IsoSpriteModelUBO), 1024);

VkDescriptorSetLayout IsoSpriteModel::getUBODescriptorSetLayout()
{
    return IsoSpriteModel::s_modelUBO.getDescriptorSetLayout();
}


}
