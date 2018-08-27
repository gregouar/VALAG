
#include "Valag/utils/Logger.h"

namespace vlg
{

template<typename AssetType>
AssetHandler<AssetType>::AssetHandler()
{
    ///m_loadThread = new sf::Thread(&AssetHandler<AssetType>::LoadInThread, this);
    ///m_loadThread = new std::thread(&AssetHandler<AssetType>::LoadInThread, this);
    m_curNewId = 1;
    m_assetLoadingInThread = nullptr;
}


template<typename AssetType>
AssetHandler<AssetType>::~AssetHandler()
{
    this->cleanAll();
    //m_loadThread->terminate();
    ///delete m_loadThread;
}



template<typename AssetType>
AssetType* AssetHandler<AssetType>::getAsset(const AssetTypeID& assetID)
{
    if(assetID == 0)
        return this->getDummyAsset();

    typename std::map<AssetTypeID, AssetType*>::iterator assetIter;

    assetIter = m_assets.find(assetID);

    if(assetIter == m_assets.end())
    {
        Logger::error("Cannot find asset");
        return this->getDummyAsset(); //(nullptr);
    }

    if(!assetIter->second->isLoaded())
    return this->getDummyAsset();

    return assetIter->second;
}


template<typename AssetType>
AssetType* AssetHandler<AssetType>::loadAssetFromFile(
                                        const std::string &filePath, AssetLoadType loadType)
{
    return loadAssetFromFile(this->generateID(), filePath, loadType);
}

template<typename AssetType>
AssetType* AssetHandler<AssetType>::loadAssetFromFile(const AssetTypeID& id,
                                        const std::string &filePath, AssetLoadType loadType)
{
	std::map<std::string, AssetTypeID>::iterator filesIter;
	filesIter = m_filesList.find(filePath);
	if(filesIter != m_filesList.end())
	{
        AssetType* foundedAsset = this->getAsset(filesIter->second);
        if(loadType == LoadType_Now && foundedAsset != nullptr)
        {
            foundedAsset->forceLoadType(loadType);

            this->removeFromLoadingThread(foundedAsset);
            this->waitForLoadingThread(foundedAsset);

            foundedAsset->loadNow();
        }
		return foundedAsset;
    }

    AssetType* newAsset = this->addAsset(id);
    newAsset->prepareLoadFromFile(filePath, loadType);
    if(loadType == LoadType_Now) {
        newAsset->loadNow();
    } else {
        this->addToLoadingThread(newAsset);
    }

	m_filesList[filePath] = id;

    return newAsset;
}


template<typename AssetType>
AssetType* AssetHandler<AssetType>::loadAssetFromMemory(
                                        void *data, std::size_t dataSize, AssetLoadType loadType)
{
    return this->loadAssetFromMemory(this->generateID(), data, dataSize, loadType);
}

template<typename AssetType>
AssetType* AssetHandler<AssetType>::loadAssetFromMemory(const AssetTypeID& id,
                                        void *data, std::size_t dataSize, AssetLoadType loadType)
{
    Asset* newAsset = this->addAsset(id);
    newAsset->prepareLoadFromMemory(data, dataSize, loadType);
    if(loadType == LoadType_Now)
        newAsset->loadNow();
    else
        this->addToLoadingThread(newAsset);

    return newAsset;
}


/**template<typename AssetType>
AssetType* AssetHandler<AssetType>::loadAssetFromStream(
                                        sf::InputStream *stream, AssetLoadType loadType)
{
    return this->loadAssetFromStream(GenerateID(), stream, loadType);
}**/

/**template<typename AssetType>
AssetType* AssetHandler<AssetType>::loadAssetFromStream(const AssetTypeID& id,
                                        sf::InputStream *stream, AssetLoadType loadType)
{
    Asset* newAsset = AddAsset(id);
    newAsset->prepareLoadFromStream(stream, loadType);
    if(loadType == LoadType_Now)
        newAsset->LoadNow();
    else
        AddToLoadingThread(newAsset);

    return newAsset;
}**/


template<typename AssetType>
AssetType* AssetHandler<AssetType>::addAsset(const AssetTypeID &assetID, bool plannedObsolescence, int lifeSpan)
{
    ///sf::Lock lockLoadMutex(m_loadMutex);
    std::lock_guard<std::mutex> lock(m_loadMutex);

    AssetType* newAsset = nullptr;
    typename std::map<AssetTypeID, AssetType*>::iterator assetIter;

    assetIter = m_assets.find(assetID);

    if(assetIter == m_assets.end())
    {
        newAsset = new AssetType(assetID);
        m_assets[assetID] = newAsset;
    } else {
        if(!plannedObsolescence)
            this->removeFromObsolescenceList(assetID);
        newAsset = assetIter->second;
    }

    if(plannedObsolescence)
        this->addToObsolescenceList(assetID, lifeSpan);

    return newAsset;
}

template<class AssetType>
AssetTypeID AssetHandler<AssetType>::generateID()
{
    return m_curNewId++;
}



template<typename AssetType>
void AssetHandler<AssetType>::addToLoadingThread(AssetType* asset)
{
    ///sf::Lock lockLoadMutex(m_loadMutex);
    std::lock_guard<std::mutex> lock(m_loadMutex);

    if(asset != nullptr && !asset->isLoaded())
    {

        m_assetsToLoadInThread.push_back(asset);
        if(m_assetsToLoadInThread.size() == 1)
        {
            if(m_loadThread.joinable())
                m_loadThread.join();
            m_loadThread = std::thread(&AssetHandler<AssetType>::loadInThread, this);
            ///m_loadThread->Launch();
        }
    }
}

template<typename AssetType>
void AssetHandler<AssetType>::removeFromLoadingThread(AssetType* asset)
{
    ///sf::Lock lockLoadMutex(m_loadMutex);
    std::lock_guard<std::mutex> lock(m_loadMutex);

    if(asset != nullptr && asset != m_assetLoadingInThread)
    {
        typename std::list<AssetType*>::iterator loadingListIt;
        loadingListIt = m_assetsToLoadInThread.begin();
        while(loadingListIt != m_assetsToLoadInThread.end())
        {
            if(*loadingListIt == asset)
            {
                m_assetsToLoadInThread.erase(loadingListIt);
                loadingListIt = m_assetsToLoadInThread.end();
            } else
                ++loadingListIt;
        }
    }
}

template<typename AssetType>
void AssetHandler<AssetType>:: addToObsolescenceList(const AssetTypeID &assetID,int lifeSpan)
{
    if(this->getAsset(assetID) != nullptr)
        m_obsolescenceList[assetID] = lifeSpan;
}

template<typename AssetType>
void AssetHandler<AssetType>:: removeFromObsolescenceList(const AssetTypeID &assetID)
{
    m_obsolescenceList.erase(assetID);
}

template<typename AssetType>
void AssetHandler<AssetType>:: descreaseObsolescenceLife()
{
    std::map<AssetTypeID, int>::iterator iter;
    for(iter = m_obsolescenceList.begin() ; iter != m_obsolescenceList.end() ; ++iter)
    {
        iter->second--;
        if(iter->second-- <= 0)
        {
            this->deleteAsset(iter->first);
            m_obsolescenceList.erase(iter);
        }
    }

}

template<typename AssetType>
void AssetHandler<AssetType>::deleteAsset(const AssetTypeID &assetID)
{
    ///sf::Lock lockLoadMutex(m_loadMutex);
    std::lock_guard<std::mutex> lock(m_loadMutex);


    typename std::map<AssetTypeID, AssetType*>::iterator iter;
    iter = m_assets.find(assetID);

    if(iter != m_assets.end())
    {
        if(iter->second != nullptr)
        {
            this->waitForLoadingThread(iter->second);

            if(!iter->second->getFilePath().empty())
                m_filesList.erase(m_filesList.find(iter->second->getFilePath()));

            if(!iter->second->isLoaded() && iter->second->getLoadType() == LoadType_InThread)
                this->removeFromLoadingThread(iter->second);

            delete iter->second;
            m_assets.erase(iter);
        }
    } else
        Logger::error("Trying to delete unfindable asset");
}


template<typename AssetType>
void AssetHandler<AssetType>::deleteAsset(AssetType *asset)
{
    if(asset != nullptr)
        this->deleteAsset(asset->getID());
}


template<typename AssetType>
void AssetHandler<AssetType>::cleanAll()
{
    this->lockLoadMutex();

    this->waitForLoadingThread(m_assetLoadingInThread);

    typename std::map<AssetTypeID, AssetType*>::iterator assetIter;
    for(assetIter = m_assets.begin() ; assetIter != m_assets.end() ; ++assetIter)
        delete assetIter->second;

    m_assets.clear();
    m_obsolescenceList.clear();
    m_filesList.clear();
    m_assetsToLoadInThread.clear();

    this->unlockLoadMutex();

    if(m_loadThread.joinable())
        m_loadThread.join();
}

template<typename AssetType>
AssetType* AssetHandler<AssetType>::getDummyAsset()
{
    if(!m_enableDummyAsset)
        return nullptr;
    return &m_dummyAsset;
}

template<typename AssetType>
void AssetHandler<AssetType>::enableDummyAsset(bool enable)
{
    m_enableDummyAsset = enable;
}


template<typename AssetType>
void AssetHandler<AssetType>::setDummyAsset(AssetType dummy)
{
    m_dummyAsset = dummy;
    m_enableDummyAsset = true;
}


template<typename AssetType>
void AssetHandler<AssetType>::loadInThread()
{
    this->lockLoadMutex();
    while(!m_assetsToLoadInThread.empty())
    {
        if(m_assetsToLoadInThread.front() != nullptr)
        {
            if(!m_assetsToLoadInThread.front()->isLoaded())
            {
                AssetType* curAsset = m_assetsToLoadInThread.front();

                m_assetLoadingInThread = curAsset;

                m_loadingCurAssetMutex.lock();
                this->unlockLoadMutex();
                curAsset->loadNow();
                m_assetLoadingInThread = nullptr;
                m_loadingCurAssetMutex.unlock();
                this->lockLoadMutex();
            }
            if(!m_assetsToLoadInThread.empty())
                m_assetsToLoadInThread.erase(m_assetsToLoadInThread.begin());
        } else
            Logger::error("Tried to load null asset in thread");
    }
    this->unlockLoadMutex();
}


template<typename AssetType>
void AssetHandler<AssetType>::lockLoadMutex()
{
    m_loadMutex.lock();
}

template<typename AssetType>
void AssetHandler<AssetType>::unlockLoadMutex()
{
    m_loadMutex.unlock();
}


template<typename AssetType>
void AssetHandler<AssetType>::waitForLoadingThread(AssetType *asset)
{
    //if(asset != nullptr)
    //while(asset == m_assetLoadingInThread){std::cout<<m_assetLoadingInThread<<std::endl;};
    if(asset != nullptr && asset == m_assetLoadingInThread)
    {
        m_loadingCurAssetMutex.lock();
        m_loadingCurAssetMutex.unlock();
    }
}


}
