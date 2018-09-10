#include "Valag/gfx/IsoSpriteEntity.h"

namespace vlg
{

IsoSpriteEntity::IsoSpriteEntity()
{
    //ctor
}

IsoSpriteEntity::~IsoSpriteEntity()
{
    //dtor
}


/** Static **/
DynamicUBODescriptor IsoSpriteEntity::s_entityUBO(sizeof(IsoSpriteEntityUBO), 4096);

bool IsoSpriteEntity::initRendering()
{
    return IsoSpriteEntity::s_entityUBO.init() & IsoSpriteModel::s_modelUBO.init();
}

void IsoSpriteEntity::updateRendering(size_t frameIndex)
{
    IsoSpriteModel::s_modelUBO.update(frameIndex);
    IsoSpriteEntity::s_entityUBO.update(frameIndex);
}

void IsoSpriteEntity::cleanupRendering()
{
    IsoSpriteModel::s_modelUBO.cleanup();
    IsoSpriteEntity::s_entityUBO.cleanup();
}

VkDescriptorSetLayout IsoSpriteEntity::getUBODescriptorSetLayout()
{
    return IsoSpriteEntity::s_entityUBO.getDescriptorSetLayout();
}


}
