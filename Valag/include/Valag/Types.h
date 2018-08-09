
#ifndef   VALAG_TYPES
#define   VALAG_TYPES

#include <iostream>
#include <chrono>

///#include "AlAGE/utils/MapIterator.h"
///#include "AlAGE/utils/ListIterator.h"

namespace vlg
{

class SceneObject;
class SceneNode;
/**class SceneEntity;
class Light;
class ShadowCaster;**/

template<class AssetType> class AssetHandler;
class TextureAsset;

enum AssetLoadType
{
    LoadType_Now = 0,
    LoadType_InThread = 1,
};

enum AssetLoadSource
{
    LoadSource_None,
    LoadSource_File,
    LoadSource_Memory,
    LoadSource_Stream,
};

enum NotificationType
{
    Notification_SenderDestroyed,
    Notification_AssetLoaded,
    Notification_SceneNodeDetroyed,
    Notification_SceneNodeMoved,
   // Notification_LightMoved,
};

enum LightType
{
    OmniLight,
    DirectionnalLight,
    SpotLight,
};

enum ShadowCastingType
{
    NoShadow,
    DirectionnalShadow,
    DynamicShadow,
    AllShadows,
};

enum ShadowVolumeType
{
    OneSidedShadow,
    TwoSidedShadow,
    MirroredTwoSidedShadow,
};

enum PBRScreenType
{
    PBRAlbedoScreen = 0,
    PBRNormalScreen,
    PBRDepthScreen,
    PBRMaterialScreen,
    PBRExtraScreen0,
};

enum CommandPoolName
{
    MAIN_COMMANDPOOL,
    TEXTURESLOADING_COMMANDPOOL,
};

typedef std::chrono::duration<double, std::chrono::seconds::period> Time;

typedef unsigned int AssetTypeID;
typedef unsigned int NodeTypeID;
typedef unsigned int ObjectTypeID;
//typedef unsigned int RenderTargetTypeID;
typedef std::string AnimationTypeID;

/**typedef ListIterator<SceneObject*> SceneObjectIterator;
typedef ListIterator<SceneEntity*> SceneEntityIterator;
typedef ListIterator<ShadowCaster*> ShadowCasterIterator;
typedef ListIterator<Light*> LightIterator;
typedef MapIterator<NodeTypeID, SceneNode*> SceneNodeIterator;**/

typedef AssetHandler<TextureAsset> TextureHandler;

const std::string emptyString;
///const sf::Texture emptyTexture;



}

#endif
