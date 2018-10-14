#ifndef SCENERENDERINGDATA_H
#define SCENERENDERINGDATA_H

#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/scene/MeshEntity.h"
#include "Valag/scene/Light.h"
#include "Valag/renderers/RenderView.h"

namespace vlg
{

class SceneRenderer;
class SceneRenderingData;

struct AmbientLightingData
{
   //glm::vec4 viewPos;
    glm::vec4 ambientLight;
    bool      enableEnvMap;
    //glm::uvec2 envMap;
};

class SceneRenderingInstance
{
    friend class SceneRenderingData;

    public:
        SceneRenderingInstance(SceneRenderingData*, SceneRenderer*);

        void addToSpritesVbo(const IsoSpriteDatum &datum);
        void addToMeshesVbo(VMesh *mesh, const MeshDatum &datum);
        void addToLightsVbo(const LightDatum &datum);

        void setViewInfo(const ViewInfo &viewInfo, glm::vec3 camPos, float camZoom);

        SceneRenderingData *getRenderingData();

        size_t getSpritesVboSize();
        size_t getSpritesVboOffset();
        size_t getMeshesVboSize(VMesh *mesh);
        size_t getMeshesVboOffset(VMesh *mesh);
        size_t getLightsVboSize();
        size_t getLightsVboOffset();

        const ViewInfo &getViewInfo();
        glm::vec4       getCamPosAndZoom();

        void pushCamPosAndZoom(VkCommandBuffer cmb, VkPipelineLayout layout, VkFlags shaderStageBit = VK_SHADER_STAGE_VERTEX_BIT);

    protected:
        SceneRenderer       *m_renderer;
        SceneRenderingData  *m_renderingData;

        ViewInfo    m_viewInfo;
        glm::vec3   m_camPos;
        float       m_camZoom;
        size_t      m_spritesVboSize;
        size_t      m_spritesVboOffset;
        std::map<VMesh*, size_t>    m_meshesVboSize;
        std::map<VMesh*, size_t>    m_meshesVboOffset;
        size_t      m_lightsVboSize;
        size_t      m_lightsVboOffset;
};

class SceneRenderingData
{
    public:
        SceneRenderingData();
        virtual ~SceneRenderingData();

        bool init(SceneRenderer *renderer);
        void cleanup();

        bool isInitialized();

        void update();

       // void createNewInstance(SceneRenderingInstance *renderingInstance, SceneRenderer *renderer);

        /*void addToSpritesVbo(const IsoSpriteDatum &datum, size_t frameIndex);
        void addToMeshesVbo(VMesh *mesh, const MeshDatum &datum, size_t frameIndex);
        void addToLightsVbo(const LightDatum &datum, size_t frameIndex);*/

        //void setAmbientLightingData(const AmbientLightingData &);
        void setAmbientLight(Color color);
        void setEnvMap(VTexture src);

        /*size_t getSpritesVboSize(size_t frameIndex);
        size_t getMeshesVboSize(VMesh *mesh, size_t frameIndex);
        size_t getLightsVboSize(size_t frameIndex);*/

        VkDescriptorSet getAmbientLightingDescSet(size_t frameIndex);

        static VkDescriptorSetLayout ambientLightingDescSetLayout();
        static void cleanStatic();

    protected:
        bool createBuffers(size_t framesCount);
        static bool createDescriptorSetLayout();
        bool createDescriptorPool(size_t framesCount);
        bool createDescriptorSets(size_t framesCount);

        void updateAmbientLightingDescSet(size_t frameIndex);
        void updateAmbientLightingUbo(size_t frameIndex);

        void updateEnvMap();


    protected:
        bool m_isInitialized;
        bool m_needToUpdateEnvMap;

        VTexture                m_envMap;
        VFramebufferAttachment  m_filteredEnvMap;

        AmbientLightingData     m_ambientLightingData;
        std::vector<bool>       m_needToUpdateAmbientLightingUbos;
        std::vector<VBuffer>    m_ambientLightingUbos;


        VkDescriptorPool                m_descriptorPool;

        std::vector<bool>               m_needToUpdateAmbientLightingDescSets;
        std::vector<VkDescriptorSet>    m_ambientLightingDescSets;

        static VkDescriptorSetLayout    s_ambientLightingDescSetLayout;


        /*std::vector<DynamicVBO<IsoSpriteDatum> >                m_spritesVbos;
        std::vector<std::map<VMesh* ,DynamicVBO<MeshDatum> > >  m_meshesVbos;
        std::vector<DynamicVBO<LightDatum> >                    m_lightsVbos;*/
};

}

#endif // SCENERENDERINGDATA_H
