#ifndef ASSETHANDLER_H
#define ASSETHANDLER_H

#include "Valag/Types.h"
#include "Valag/core/Asset.h"
#include "Valag/utils/Singleton.h"

#include <map>
#include <vector>
#include <mutex>
#include <thread>


namespace vlg
{

template<class AssetType> class AssetHandler : public Singleton<AssetHandler<AssetType> >
{
    public:
        friend class Singleton<AssetHandler<AssetType> >;


        AssetType* getAsset(const AssetTypeID &assetID);

        AssetType* loadAssetFromFile(const std::string &, AssetLoadType = LoadType_Now);
        AssetType* loadAssetFromMemory(void *data, std::size_t dataSize, AssetLoadType = LoadType_Now);
        ///AssetType* loadAssetFromStream(sf::InputStream *stream, AssetLoadType = LoadType_Now);

        AssetType* loadAssetFromFile(const AssetTypeID& id,const std::string &, AssetLoadType = LoadType_Now);
        AssetType* loadAssetFromMemory(const AssetTypeID& id, void *data, std::size_t dataSize, AssetLoadType = LoadType_Now);
       /// AssetType* loadAssetFromStream(const AssetTypeID& id,sf::InputStream *stream, AssetLoadType = LoadType_Now);

        AssetType* addAsset(const AssetTypeID &assetID, bool plannedObsolescence=false, int lifeSpan=1);

        void addToLoadingThread(AssetType*);
        void removeFromLoadingThread(AssetType*);

        AssetTypeID generateID();

        void addToObsolescenceList(const AssetTypeID &assetID,int lifeSpan = 1);
        void removeFromObsolescenceList(const AssetTypeID &assetID);

        void descreaseObsolescenceLife();
        void deleteAsset(AssetType* );
        void deleteAsset(const AssetTypeID &assetID);
        void cleanAll();

        void enableDummyAsset(bool enable = true);
        void setDummyAsset(AssetType);
        AssetType* getDummyAsset();

    protected:
        AssetHandler();
        virtual ~AssetHandler();

        void loadInThread();

        void lockLoadMutex();
        void unlockLoadMutex();
        void waitForLoadingThread(AssetType *assetToWaitFor);

    private:
        std::map<AssetTypeID, AssetType*> m_assets;
        std::map<AssetTypeID, int> m_obsolescenceList;
        std::map<std::string, AssetTypeID> m_filesList;

        std::thread m_loadThread;
        std::mutex m_loadMutex;
        std::mutex m_loadingCurAssetMutex;

        std::list<AssetType*> m_assetsToLoadInThread;
        AssetType* m_assetLoadingInThread;

        int m_curNewId;
        bool m_enableDummyAsset;
        AssetType m_dummyAsset;
};

}


#include "../src/core/AssetHandler.inc"


#endif // ASSETHANDLER_H
