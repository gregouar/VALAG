#ifndef RENDERGRAPH_H
#define RENDERGRAPH_H

#include "Valag/renderers/FullRenderPass.h"

namespace vlg
{

///I should add option to force storeOp of attachment

class RenderGraph
{
    public:
        RenderGraph(size_t imagesCount, size_t framesCount);
        virtual ~RenderGraph();

        bool init();
        void destroy();

        void    setDefaultExtent(VkExtent2D extent);

        ///If VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT is set, then the cmb will use framesCount and frameIndex
        ///Otherwise it will use imagesCount and imageIndex
        size_t  addRenderPass(VkFlags usage = 0);
        void    connectRenderPasses(size_t src, size_t dst);

        ///If an attachment is of type VK_IMAGE_LAYOUT_PRESENT_SRC_KHR then the cmb will be returned by submitToGraphicsQueue in order to be
        ///rendered by RenderWindow
        void    setAttachments(size_t renderPassIndex, size_t bufferIndex, const std::vector<VFramebufferAttachment> &attachments);
        void    setClearValue(size_t renderPassIndex, size_t attachmentIndex ,glm::vec4 color, glm::vec2 depth);

        VkRenderPass getVkRenderPass(size_t renderPassIndex);


        VkCommandBuffer startRecording(size_t renderPassIndex, size_t imageIndex, size_t frameIndex,
                                       VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
        bool            endRecording(size_t renderPassIndex);

        ///Return final render passes
        std::vector<FullRenderPass*> submitToGraphicsQueue(size_t imageIndex, size_t frameIndex);

    protected:
        bool createSemaphores();
        bool initRenderPasses();

    protected:
        VkExtent2D  m_defaultExtent;
        size_t      m_imagesCount, m_framesCount;

        std::list<std::pair<size_t, size_t> > m_connexions; //Second one is waiting for first one

        std::vector<FullRenderPass*> m_renderPasses;
        std::list<VkSemaphore> m_semaphores;

    private:

};

}

#endif // RENDERGRAPH_H
