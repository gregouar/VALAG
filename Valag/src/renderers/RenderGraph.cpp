#include "Valag/renderers/RenderGraph.h"

#include "Valag/utils/Logger.h"

namespace vlg
{

RenderGraph::RenderGraph(size_t imagesCount, size_t framesCount) :
    m_imagesCount(imagesCount),
    m_framesCount(framesCount)
{
    //ctor
}

RenderGraph::~RenderGraph()
{
    this->destroy();
}

bool RenderGraph::init()
{
    if(!this->createSemaphores())
        return (false);

    if(!this->initRenderPasses())
        return (false);

    return (true);
}

void RenderGraph::destroy()
{
    for(auto renderPass : m_renderPasses)
        delete renderPass;
    m_renderPasses.clear();

    m_connexions.clear();

    for(auto semaphore : m_semaphores)
        vkDestroySemaphore(VInstance::device(), semaphore, nullptr);
    m_semaphores.clear();
}

void RenderGraph::setDefaultExtent(VkExtent2D extent)
{
    m_defaultExtent = extent;
}

size_t RenderGraph::addRenderPass(VkFlags usage)
{
    m_renderPasses.push_back(new FullRenderPass(m_imagesCount, m_framesCount));
    m_renderPasses.back()->setCmbUsage(usage);
    return m_renderPasses.size()-1;
}

void RenderGraph::connectRenderPasses(size_t src, size_t dst)
{
    if(src >= m_renderPasses.size()
    || dst >= m_renderPasses.size())
    {
        std::ostringstream errorReport;
        errorReport<<"Can not connect render pass "<<src<<" with "<<dst;
        Logger::error(errorReport);
        return;
    }

    m_connexions.push_back({src, dst});
}


void RenderGraph::setAttachments(size_t renderPassIndex, size_t bufferIndex, const std::vector<VFramebufferAttachment> &attachments)
{
    m_renderPasses[renderPassIndex]->setAttachments(bufferIndex, attachments);
}

void RenderGraph::setClearValue(size_t renderPassIndex, size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    m_renderPasses[renderPassIndex]->setClearValues(attachmentIndex, color, depth);
}

VkRenderPass RenderGraph::getVkRenderPass(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->getVkRenderPass();
}

VkCommandBuffer RenderGraph::startRecording(size_t renderPassIndex, size_t imageIndex, size_t frameIndex, VkSubpassContents contents)
{
    return m_renderPasses[renderPassIndex]->startRecording(imageIndex, frameIndex, contents);
}

bool RenderGraph::endRecording(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->endRecording();
}

std::vector<FullRenderPass*> RenderGraph::submitToGraphicsQueue(size_t imageIndex, size_t frameIndex)
{
    std::vector<FullRenderPass*> finalRenderPasses;
    std::vector<VkSubmitInfo> submitInfos;

    for(auto renderPass : m_renderPasses)
    {
        if(renderPass->isFinalPass())
        {
            finalRenderPasses.push_back(renderPass);
        } else {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(renderPass->getWaitSemaphores(frameIndex).size());
            submitInfo.pWaitDstStageMask    = renderPass->getWaitSemaphoresStages().data();
            submitInfo.pWaitSemaphores      = renderPass->getWaitSemaphores(frameIndex).data();
            submitInfo.signalSemaphoreCount = 1; //static_cast<uint32_t>(renderPass->getSignalSemaphores(frameIndex).size()));
            submitInfo.pSignalSemaphores    = renderPass->getSignalSemaphore(frameIndex);//renderPass->getSignalSemaphores(frameIndex).data();
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = renderPass->getPrimaryCmb(imageIndex, frameIndex);

            submitInfos.push_back(submitInfo);
        }
    }

    VInstance::submitToGraphicsQueue(submitInfos, 0);

    return finalRenderPasses;
}



///Protected ///

bool RenderGraph::initRenderPasses()
{
    for(auto renderPass : m_renderPasses)
    {
        size_t bufferCount =
           (renderPass->getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? m_framesCount : m_imagesCount;
        renderPass->setCmbCount(bufferCount);

        if(renderPass->getExtent().width == 0)
            renderPass->setExtent(m_defaultExtent);

        if(!renderPass->init())
            return (false);
    }

    return (true);
}

bool RenderGraph::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


    for(auto connexion : m_connexions)
    {
        std::vector<VkSemaphore> waitSemaphores(m_framesCount, VkSemaphore{});

        for(size_t i = 0 ; i < m_framesCount ; ++i)
        {
            //bug here
            VkSemaphore *signalSem = m_renderPasses[connexion.first]->getSignalSemaphore(i);

            if(signalSem == nullptr)
            {
                m_semaphores.push_back(VkSemaphore{});
                if(vkCreateSemaphore(VInstance::device(), &semaphoreInfo, nullptr, &m_semaphores.back()) != VK_SUCCESS)
                    return (false);
                m_renderPasses[connexion.first]->setSignalSemaphores(i,&m_semaphores.back());
                signalSem = &m_semaphores.back();
            }
            waitSemaphores[i] = *signalSem;
        }
        m_renderPasses[connexion.second]->addWaitSemaphores(waitSemaphores, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    return (true);
}



}
