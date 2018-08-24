#include "Valag/gfx/SceneRenderer.h"

namespace vlg
{

SceneRenderer::SceneRenderer(RenderWindow *targetWindow) :
    m_targetWindow(targetWindow)
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
