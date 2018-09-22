#include "Valag/renderers/AbstractRenderer.h"

#include <array>

#include "Valag/utils/Profiler.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

AbstractRenderer::AbstractRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) :
    m_targetWindow(targetWindow),
    m_renderGraph(targetWindow->getSwapchainSize(),
                  targetWindow->getFramesCount()),
    m_descriptorPool(VK_NULL_HANDLE),
    m_curFrameIndex(0),
    m_order(order),
    m_name(name)
{
}

AbstractRenderer::~AbstractRenderer()
{
}


void AbstractRenderer::update(size_t frameIndex)
{
    m_renderView.update(frameIndex);
}

void AbstractRenderer::render(uint32_t imageIndex)
{
    Profiler::pushClock("Record primary buffer");
    this->recordPrimaryCmb(imageIndex);
    m_finalPasses = m_renderGraph.submitToGraphicsQueue(imageIndex, m_curFrameIndex);
    Profiler::popClock();

    m_curFrameIndex = (m_curFrameIndex + 1) % m_targetWindow->getFramesCount();
}

void AbstractRenderer::setView(glm::mat4 view)
{
    m_renderView.setView(view);
}

std::vector<FullRenderPass*> AbstractRenderer::getFinalPasses()
{
    return m_finalPasses;
}

RendererName AbstractRenderer::getName()
{
    return m_name;
}

bool AbstractRenderer::createRenderPass()
{
    m_defaultPass = m_renderGraph.addRenderPass(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    for(size_t i = 0 ; i < m_targetWindow->getSwapchainSize() ; ++i)
    {
        std::vector<VFramebufferAttachment> attachements = {m_targetWindow->getSwapchainAttachments()[i],
                                                            m_targetWindow->getSwapchainDepthAttachments()[i]};
        m_renderGraph.setAttachments(m_defaultPass, i, attachements);
    }

    return (true);
}

bool AbstractRenderer::initRenderGraph()
{
    m_renderGraph.setDefaultExtent(m_targetWindow->getSwapchainExtent());
    return m_renderGraph.init();
}

bool AbstractRenderer::createRenderView()
{
    m_renderView.setExtent({m_targetWindow->getSwapchainExtent().width,
                            m_targetWindow->getSwapchainExtent().height});

    return m_renderView.create(m_targetWindow->getFramesCount());
}

bool AbstractRenderer::init()
{
    if(!this->createRenderPass())
    {
        Logger::error("Cannot create default render pass");
        return (false);
    }

    if(!this->createDescriptorSetLayouts())
    {
        Logger::error("Cannot create default descriptor set layout");
        return (false);
    }

    if(!this->createRenderView())
    {
        Logger::error("Cannot create render view");
        return (false);
    }

    if(!this->initRenderGraph())
    {
        Logger::error("Cannot initialize render graph");
        return (false);
    }

    if(!this->createGraphicsPipeline())
    {
        Logger::error("Cannot create graphics pipeline");
        return (false);
    }

    if(!this->createDescriptorPool())
    {
        Logger::error("Cannot create descriptor pool");
        return (false);
    }

    if(!this->createDescriptorSets())
    {
        Logger::error("Cannot create descriptor sets");
        return (false);
    }

    return (true);
}

void AbstractRenderer::cleanup()
{
    VkDevice device = VInstance::device();

    m_renderView.destroy();

    if(m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device,m_descriptorPool,nullptr);
    m_descriptorPool = VK_NULL_HANDLE;

    m_renderGraph.destroy();
}


}
