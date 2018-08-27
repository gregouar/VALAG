#include "Valag/core/Asset.h"
#include "Valag/utils/Parser.h"
#include "Valag/utils/Logger.h"
///#include "Valag/gfx/SceneEntity.h"

namespace vlg
{

Asset::Asset()
{
    m_allowLoadFromFile = false;
    m_allowLoadFromMemory = false;
   /// m_allowLoadFromStream = false;
    m_loadSource = LoadSource_None;
    m_loaded = false;
    m_loadType = LoadType_Now;
}

Asset::Asset(const AssetTypeID &id) : Asset()
{
    m_id=id;
}

Asset::~Asset()
{
    //dtor
}


bool Asset::loadFromFile(const std::string &)
{
    return (false);
}

bool Asset::loadFromMemory(void *data, std::size_t size)
{
    return (false);
}

bool Asset::prepareLoadFromFile(const std::string &filePath, AssetLoadType loadType)
{
    if(!m_allowLoadFromFile)
    {
        Logger::error("Asset is not allowed to load from file");
        return (false);
    }

    m_loadSource = LoadSource_File;
    m_filePath = filePath;
    m_fileDirectory = Parser::findFileDirectory(m_filePath);
    m_loadType = loadType;

    return (true);
}

bool Asset::prepareLoadFromMemory(void *data, std::size_t dataSize, AssetLoadType loadType)
{
    if(!m_allowLoadFromMemory)
    {
        Logger::error("Asset is not allowed to load from memory");
        return (false);
    }

    m_loadSource = LoadSource_Memory;
    m_loadData = data;
    m_loadDataSize = dataSize;
    m_loadType = loadType;

    return (true);
}

/**bool Asset::prepareLoadFromStream(sf::InputStream *stream, AssetLoadType loadType)
{
    if(!m_allowLoadFromStream)
    {
        Logger::Error("Asset is not allowed to load from stream");
        return (false);
    }

    m_loadSource = LoadSource_Stream;
    m_loadStream = stream;
    m_loadType = loadType;

    return (true);
}**/

bool Asset::loadNow()
{
    bool loaded = true;

    if(!m_loaded)
    {
        if(m_loadSource == LoadSource_File)
        {
            this->loadFromFile(m_filePath);

            /**if(!m_texture->loadFromFile(m_filePath))
            {
                Logger::Error("Cannot load texture from file: "+m_filePath);
                loaded = false;
            } else
                Logger::write("Texture loaded from file: "+m_filePath);**/
        } else if(m_loadSource == LoadSource_Memory) {
            this->loadFromMemory(m_loadData, m_loadDataSize);

            /**if(!m_texture->loadFromMemory(m_loadData,m_loadDataSize))
            {
                Logger::Error("Cannot load texture from memory");
                loaded = false;
            }**/
        } else if(m_loadSource == LoadSource_Stream) {
            /**if(!m_texture->loadFromStream(*m_loadStream))
            {
                Logger::Error("Cannot load texture from stream");
                loaded = false;
            }**/
        } else {
            Logger::error("Cannot load asset");
            m_loaded = false;
        }

        m_loaded = loaded;
    }

    if(!m_loaded)
        return (false);

    this->sendNotification(Notification_AssetLoaded);
    return (true);
}

bool Asset::isLoaded()
{
    return m_loaded;
}

const std::string &Asset::getFilePath()
{
    if(m_loadSource == LoadSource_File)
        return m_filePath;
    return emptyString;
}


const AssetLoadType Asset::getLoadType()
{
    return m_loadType;
}

const AssetLoadSource Asset::getLoadSource()
{
    return m_loadSource;
}

const AssetTypeID &Asset::getID()
{
    return m_id;
}

void Asset::forceLoadType(AssetLoadType type)
{
    m_loadType = type;
}

}
