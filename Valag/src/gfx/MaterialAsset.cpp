#include "Valag/gfx/MaterialAsset.h"

#include "Valag/Types.h"
#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Parser.h"

namespace vlg
{

MaterialAsset::MaterialAsset()
{
    m_allowLoadFromFile     = true;
    m_allowLoadFromMemory   = false;

    m_heightFactor  = 1;
    m_rmtFactor     = glm::vec3(0.0,0.0,0.0);

    m_albedoMap = nullptr;
    m_normalMap = nullptr;
    m_heightMap = nullptr;
    m_rmtMap    = nullptr;
}

MaterialAsset::MaterialAsset(const AssetTypeID& id) : Asset(id)
{
    m_allowLoadFromFile     = true;
    m_allowLoadFromMemory   = false;

    m_heightFactor  = 1;
    m_rmtFactor     = glm::vec3(0.0,0.0,0.0);

    m_albedoMap = nullptr;
    m_normalMap = nullptr;
    m_heightMap = nullptr;
    m_rmtMap    = nullptr;
}

MaterialAsset::~MaterialAsset()
{
    //dtor
}

bool MaterialAsset::loadFromFile(const std::string &filePath)
{
    /*int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels)
    {
        Logger::error("Cannot load texture from file: "+m_filePath);
        return (false);
    }

    CommandPoolName commandPoolName;
    if(m_loadType == LoadType_Now)
        commandPoolName = COMMANDPOOL_SHORTLIVED;
    else
        commandPoolName = COMMANDPOOL_TEXTURESLOADING;

    m_vtexture.generateTexture(pixels, texWidth, texHeight,commandPoolName);

    stbi_image_free(pixels);

    Logger::write("Texture loaded from file: "+filePath);*/
    TiXmlDocument file(m_filePath.c_str());

    if(!file.LoadFile())
    {
        Logger::error("Cannot load material from file: "+m_filePath);
        std::ostringstream errorReport;
        errorReport << "Because: "<<file.ErrorDesc();
        Logger::error(errorReport);
        return (false);
    }

    TiXmlHandle hdl(&file);
    hdl = hdl.FirstChildElement();

    return this->loadFromXML(&hdl);
}


bool MaterialAsset::loadFromXML(TiXmlHandle *hdl)
{
    if(hdl == nullptr) return (false);

    if(hdl->FirstChildElement("name").Element() != nullptr)
        m_name = hdl->FirstChildElement("name").Element()->GetText();

    if(hdl->FirstChildElement("height").Element() != nullptr)
        m_heightFactor = Parser::parseFloat(hdl->FirstChildElement("height").Element()->GetText());

    if(hdl->FirstChildElement("roughness").Element() != nullptr)
        m_rmtFactor.r = Parser::parseFloat(hdl->FirstChildElement("roughness").Element()->GetText());

    if(hdl->FirstChildElement("metalness").Element() != nullptr)
        m_rmtFactor.g = Parser::parseFloat(hdl->FirstChildElement("metalness").Element()->GetText());

    if(hdl->FirstChildElement("translucency").Element() != nullptr)
        m_rmtFactor.b = Parser::parseFloat(hdl->FirstChildElement("translucency").Element()->GetText());

    /*TiXmlElement* textElem = hdl->FirstChildElement("material").Element();
    while(textElem != nullptr)
    {
        if(std::string(textElem->Attribute("type")).compare("roughness") == 0)
        {
            float r = Parser::ParseFloat(textElem->GetText());
            if(r >= 0 && r <= 1)
                m_roughness = r;
            else
                Logger::Error("Cannot read roughness in material"+m_filePath);
        } else if(std::string(textElem->Attribute("type")).compare("metalness") == 0)
        {
            float r = Parser::ParseFloat(textElem->GetText());
            if(r >= 0 && r <= 1)
                m_metalness = r;
            else
                Logger::Error("Cannot read metalness in material"+m_filePath);
        } else if(std::string(textElem->Attribute("type")).compare("translucency") == 0)
        {
            float r = Parser::ParseFloat(textElem->GetText());
            if(r >= 0 && r <= 1)
                m_translucency = r;
            else
                Logger::Error("Cannot read translucency in material"+m_filePath);
        }
        textElem = textElem->NextSiblingElement("material");
    }*/

    TiXmlElement* textElem = hdl->FirstChildElement("texture").Element();
    while(textElem != nullptr)
    {
        if(std::string(textElem->Attribute("type")).compare("albedo") == 0)
        {
            m_albedoMap = TexturesHandler::instance()
                            ->loadAssetFromFile(m_fileDirectory+textElem->GetText(),m_loadType);
            m_albedoMap->askForAllNotifications(this);
        }
        else if(std::string(textElem->Attribute("type")).compare("normal") == 0)
        {
            m_normalMap = TexturesHandler::instance()
                            ->loadAssetFromFile(m_fileDirectory+textElem->GetText(),m_loadType);
            m_normalMap->askForAllNotifications(this);

        }
        else if(std::string(textElem->Attribute("type")).compare("height") == 0)
        {
            m_heightMap = TexturesHandler::instance()
                            ->loadAssetFromFile(m_fileDirectory+textElem->GetText(),m_loadType);
            m_heightMap->askForAllNotifications(this);
        }
        else if(std::string(textElem->Attribute("type")).compare("rmt") == 0)
        {
            m_rmtMap = TexturesHandler::instance()
                            ->loadAssetFromFile(m_fileDirectory+textElem->GetText(),m_loadType);
            m_rmtMap->askForAllNotifications(this);
        }
        textElem = textElem->NextSiblingElement("texture");
    }

    Logger::write("Material loaded from file: "+m_filePath);

    return (true);
}


void MaterialAsset::notify(NotificationSender* sender, NotificationType notification)
{
    if(notification == Notification_AssetLoaded)
    if(sender == m_albedoMap || sender == m_heightMap
       || sender == m_normalMap || sender == m_rmtMap)
    {
        if(m_albedoMap == nullptr || m_albedoMap->isLoaded())
        if(m_heightMap == nullptr || m_heightMap->isLoaded())
        if(m_normalMap == nullptr || m_normalMap->isLoaded())
        if(m_rmtMap == nullptr || m_rmtMap->isLoaded())
        {
            m_loaded = true, Asset::loadNow();
            Logger::write("Material loaded from file: "+m_filePath);
        }
    }

    if(notification == Notification_SenderDestroyed)
    {
        if(sender == m_albedoMap)
            m_albedoMap = nullptr;
        else if(sender == m_heightMap)
            m_heightMap = nullptr;
        else if(sender == m_normalMap)
            m_normalMap = nullptr;
        else if(sender == m_rmtMap)
            m_rmtMap = nullptr;
    }
}

}
