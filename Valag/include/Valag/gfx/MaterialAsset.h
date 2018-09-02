#ifndef MATERIALASSET_H
#define MATERIALASSET_H

#include "Valag/core/Asset.h"
#include "Valag/core/NotificationListener.h"

#include "tinyxml/tinyxml.h"
#include "glm/glm.hpp"

namespace vlg
{

class MaterialAsset : public Asset, public NotificationListener
{
    public:
        MaterialAsset();
        MaterialAsset(const AssetTypeID&);
        virtual ~MaterialAsset();

        bool loadFromFile(const std::string &filePath);

        virtual void notify(NotificationSender* , NotificationType);

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
