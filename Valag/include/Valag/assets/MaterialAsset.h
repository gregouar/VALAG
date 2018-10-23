#ifndef MATERIALASSET_H
#define MATERIALASSET_H

#include "Valag/assets/Asset.h"
#include "Valag/core/NotificationListener.h"
#include "Valag/vulkanImpl/VTexture.h"

#include "tinyxml/tinyxml.h"
#include "glm/glm.hpp"

namespace vlg
{

class MaterialAsset : public Asset, public NotificationListener
{
    public:
        MaterialAsset();
        MaterialAsset(const AssetTypeId);
        virtual ~MaterialAsset();

        bool loadFromFile(const std::string &filePath);

        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

        VTexture getAlbedoMap();
        VTexture getNormalMap();
        VTexture getHeightMap();
        VTexture getRmtMap();

        float getHeightFactor();
        glm::vec3 getRmtFactor();

    protected:
        bool loadFromXML(TiXmlHandle *);

    private:
        TextureAsset* m_albedoMap; //Color
        TextureAsset* m_normalMap;
        TextureAsset* m_heightMap;
        TextureAsset* m_rmtMap; // Roughness-Metalness-Translucency

        float       m_heightFactor;
        glm::vec3   m_rmtFactor;
};

}

#endif // MATERIALASSET_H
