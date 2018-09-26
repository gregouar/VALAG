
#ifndef   VALAG_TYPES
#define   VALAG_TYPES

#include <iostream>
#include <chrono>

#include <glm/glm.hpp>

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
class MaterialAsset;
class MeshAsset;

enum RendererName
{
    Renderer_Default,
    Renderer_Scene,
    Renderer_Instancing,
};

enum RenderereOrder
{
    Renderer_First,
    Renderer_Middle,
    Renderer_Last,
    Renderer_Unique,
};

enum BlendMode
{
    BlendMode_None,
    BlendMode_Add,
    BlendMode_Alpha,
    BlendMode_Custom,
};

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
    Notification_UpdateCmb,
    Notification_TextureChanged,
    Notification_TextureIsAboutToChange,
   // Notification_LightMoved,
};

enum LightType
{
    LightType_Omni,
    LightType_Directionnal,
    LightType_Spot,
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

enum CommandPoolName
{
    COMMANDPOOL_DEFAULT,
    COMMANDPOOL_SHORTLIVED, //fort short-lived command buffers
    COMMANDPOOL_TEXTURESLOADING, //for texture loading thread
    COMMANDPOOL_NBR_NAMES,
};

typedef std::chrono::duration<double, std::chrono::seconds::period> Time;

typedef unsigned int AssetTypeId;
typedef unsigned int NodeTypeId;
typedef unsigned int ObjectTypeId;

typedef glm::vec4 Color;

/**typedef ListIterator<SceneObject*> SceneObjectIterator;
typedef ListIterator<SceneEntity*> SceneEntityIterator;
typedef ListIterator<ShadowCaster*> ShadowCasterIterator;
typedef ListIterator<Light*> LightIterator;
typedef MapIterator<NodeTypeId, SceneNode*> SceneNodeIterator;**/

typedef AssetHandler<TextureAsset>  TexturesHandler;
typedef AssetHandler<MaterialAsset> MaterialsHandler;
//typedef AssetHandler<MeshAsset>     MeshesHandler;

const std::string emptyString;
///const sf::Texture emptyTexture;



}

#endif
