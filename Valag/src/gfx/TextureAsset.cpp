#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

TextureAsset::TextureAsset()
{
    ///m_texture = new sf::Texture();
    m_createdTexture = true;

    m_allowLoadFromFile = true;
    m_allowLoadFromMemory = true;
    ///m_allowLoadFromStream = true;

    ///m_texture->setRepeated(true);
    ///m_texture->setSmooth(true);
}

TextureAsset::TextureAsset(const AssetTypeID& id) : Asset(id)
{
    ///m_texture = new sf::Texture();
    m_createdTexture = true;

    m_allowLoadFromFile = true;
    m_allowLoadFromMemory = true;
    ///m_allowLoadFromStream = true;

   /// m_texture->setRepeated(true);
   /// m_texture->setSmooth(true);
}


/**TextureAsset::TextureAsset(const sf::Image &img) : TextureAsset()
{
    m_texture->loadFromImage(img);
    m_loaded = true;
}

TextureAsset::TextureAsset(sf::Texture *texture)
{
    m_texture = texture;
    if(m_texture != nullptr)
        m_loaded = true;

    m_createdTexture = false;
}**/


TextureAsset::~TextureAsset()
{
    ///if(m_createdTexture && m_texture != nullptr)
       /// delete m_texture;
}

bool TextureAsset::loadNow()
{
    bool loaded = true;

    if(!m_loaded) {
        if(m_loadSource == LoadSource_File)
        {
           /** if(!m_texture->loadFromFile(m_filePath))
            {
                Logger::Error("Cannot load texture from file: "+m_filePath);
                loaded = false;
            } else**/
                Logger::write("Texture loaded from file: "+m_filePath);
        } else if(m_loadSource == LoadSource_Memory) {
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

    return Asset::loadNow();
}

/**void TextureAsset::generateMipmap()
{
    if(m_texture != nullptr)
        m_texture->generateMipmap();
}

void TextureAsset::SetSmooth(bool smooth)
{
    if(m_texture != nullptr)
        m_texture->setSmooth(smooth);
}

sf::Texture *TextureAsset::GetTexture()
{
    if(m_loaded)
        return m_texture;

    if(AssetHandler<TextureAsset>::Instance()->GetDummyAsset() != nullptr)
        return AssetHandler<TextureAsset>::Instance()->GetDummyAsset()->GetTexture();
    else
        return nullptr;
}**/

}
