#ifndef ASSET_H
#define ASSET_H

#include "Valag/core/NotificationSender.h"
#include "Valag/Types.h"

#include <iostream>

namespace vlg
{

class Asset : public NotificationSender
{
    public:
        Asset();
        Asset(const AssetTypeID );
        virtual ~Asset();

        virtual bool loadFromFile(const std::string &);
        virtual bool loadFromMemory(void *data, std::size_t dataSize);

        bool prepareLoadFromFile(const std::string &, AssetLoadType = LoadType_Now);
        bool prepareLoadFromMemory(void *data, std::size_t dataSize, AssetLoadType = LoadType_Now);
        /// bool prepareLoadFromStream(sf::InputStream *stream, AssetLoadType = LoadType_Now);

        bool loadNow();

        bool isLoaded();
        const std::string& getFilePath();

        AssetTypeID getID();
        AssetLoadType getLoadType();
        AssetLoadSource getLoadSource();

        void forceLoadType(AssetLoadType);

    protected:

        bool m_allowLoadFromFile;
        std::string m_filePath;
        std::string m_fileDirectory;

        bool m_allowLoadFromMemory;
        void *m_loadData;
        std::size_t m_loadDataSize;

        ///bool m_allowLoadFromStream;
        ///sf::InputStream *m_loadStream;

        AssetLoadSource m_loadSource;
        AssetLoadType   m_loadType;

        bool m_loaded;

        std::string m_name;

    private:
        AssetTypeID m_id;

};


}

#endif // ASSET_H
