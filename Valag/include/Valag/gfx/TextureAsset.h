#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

#include "Valag/core/Asset.h"

namespace vlg
{

class TextureAsset : public Asset
{
    public:
        TextureAsset();
        TextureAsset(const AssetTypeID&);
        ///TextureAsset(const sf::Image &img);
        ///TextureAsset(sf::Texture *);
        virtual ~TextureAsset();

        virtual bool loadNow();

        ///void generateMipmap();
        ///void setSmooth(bool = true);

        ///virtual sf::Texture* GetTexture();

    protected:
        ///sf::Texture* m_texture;

    private:
        bool m_createdTexture;
};

}

#endif // TEXTUREASSET_H
