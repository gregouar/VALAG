#include "Valag/gfx/SceneRenderer.h"

namespace vlg
{

SceneRenderer::SceneRenderer(VInstance *vulkanInstance) :
    m_vulkanInstance(vulkanInstance)
{
    this->init();
}

SceneRenderer::~SceneRenderer()
{
    this->cleanup();
}

void SceneRenderer::init()
{
    //this->createRenderPass();
    //this->createGraphicsPipeline();
}

void SceneRenderer::cleanup()
{

}

}
