#include "Valag/assets/MeshAsset.h"

#include <iostream>
#include "Valag/Types.h"
#include "Valag/utils/Logger.h"
#include "Valag/utils/Parser.h"

#include "Valag/assets/AssetHandler.h"
#include "Valag/assets/MaterialAsset.h"

namespace vlg
{


VkVertexInputBindingDescription MeshVertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(MeshVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 9> MeshVertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 9> attributeDescriptions = {};

    uint32_t i = 0;
    uint32_t b = 0;
    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, pos);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, uv);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, normal);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, albedo_color);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, rmt_color);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, albedo_texId);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, height_texId);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, normal_texId);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_UINT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, rmt_texId);
    ++i;


    return attributeDescriptions;
}


MeshAsset::MeshAsset() : MeshAsset(-1)
{
}

MeshAsset::MeshAsset(const AssetTypeId id) : Asset(id)
{
    m_allowLoadFromFile     = true;
    //m_allowLoadFromMemory   = true;
    m_material              = nullptr;

    m_scale = 1.0f;
}

MeshAsset::~MeshAsset()
{
    VBuffersAllocator::freeBuffer(m_vertexBuffer);
    VBuffersAllocator::freeBuffer(m_indexBuffer);
}

bool MeshAsset::loadFromFile(const std::string &filePath)
{
    TiXmlDocument file(filePath.c_str());

    if(!file.LoadFile())
    {
        Logger::error("Cannot load mesh from file: "+filePath);
        std::ostringstream errorReport;
        errorReport << "Because: "<<file.ErrorDesc();
        Logger::error(errorReport);
        return (false);
    }

    TiXmlHandle hdl(&file);
    hdl = hdl.FirstChildElement();

    return this->loadFromXML(&hdl);
}

void MeshAsset::notify(NotificationSender* sender, NotificationType type)
{

}

float MeshAsset::getScale()
{
    return m_scale;
}


/// Protected ///

bool MeshAsset::loadFromXML(TiXmlHandle *hdl)
{
    if(hdl == nullptr) return (false);

    if(hdl->FirstChildElement("name").Element() != nullptr)
        m_name = hdl->FirstChildElement("name").Element()->GetText();

    if(hdl->FirstChildElement("scale").Element() != nullptr)
        m_scale = Parser::parseFloat(hdl->FirstChildElement("scale").Element()->GetText());

    if(hdl->FirstChildElement("material").Element() != nullptr)
    {
        std::string materialPath = hdl->FirstChildElement("material").Element()->GetText();

        m_material = MaterialsHandler::instance()
                        ->loadAssetFromFile(m_fileDirectory+materialPath,m_loadType);
        m_material->askForAllNotifications(this);
    }

    if(hdl->FirstChildElement("model").Element() != nullptr)
    {
        std::string modelPath = hdl->FirstChildElement("model").Element()->GetText();
        this->loadModelFromObj(m_fileDirectory+modelPath);
    }

    Logger::write("Mesh loaded from file: "+m_filePath);

    return (true);
}

bool MeshAsset::loadModelFromObj(const std::string &filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::in);

    std::vector<glm::vec3> vertexList;
    std::vector<glm::vec2> uvList;
    std::vector<glm::vec3> normalList;
    std::vector<glm::vec3> indexList;

    if(file)
    {
        std::string buf;

        while(!file.eof())
        {
            file>>buf;

            if(buf == "#")
            {
                std::getline(file, buf);
            } else if(buf == "v") {
                vertexList.push_back({});
                file>>vertexList.back().x>>vertexList.back().y>>vertexList.back().z;
            } else if(buf == "vt") {
                uvList.push_back({});
                file>>uvList.back().x>>uvList.back().y;
                uvList.back().y = 1.0-uvList.back().y;
            } else if(buf == "vn") {
                normalList.push_back({});
                file>>normalList.back().x>>normalList.back().y>>normalList.back().z;
            } else if(buf == "f") {
                //f.push_back(std::vector<sf::Vector2i> ());
                std::string line;
                std::getline(file, line);

                std::vector<glm::vec3> indices;

                std::istringstream iss(line);
                std::string word;
                while(iss >> word)
                {
                    indices.push_back({-1,-1,-1});

                    std::istringstream issBis(word);
                    char c;
                    issBis >> indices.back().x;
                    if(!issBis.eof())
                        issBis >> c >> indices.back().y;
                    if(!issBis.eof())
                        issBis >> c >> indices.back().z;

                    indices.back().x -= 1;
                    indices.back().y -= 1;
                    indices.back().z -= 1;
                }

                indexList.push_back({indices[0].x, indices[0].y, indices[0].z});
                indexList.push_back({indices[1].x, indices[1].y, indices[1].z});
                indexList.push_back({indices[2].x, indices[2].y, indices[2].z});

                if(indices.size() == 4) //If quad we need to put 2 triangles
                {
                    indexList.push_back({indices[2].x, indices[2].y, indices[2].z});
                    indexList.push_back({indices[3].x, indices[3].y, indices[3].z});
                    indexList.push_back({indices[0].x, indices[0].y, indices[0].z});
                }
            }
        }
        this->generateModel(vertexList, uvList, normalList, indexList);
        file.close();
    }
    else
    {
        Logger::write("Cannot load mesh model from file: "+filePath);
        return (false);
    }

    return (true);
}


void MeshAsset::setMaterial(AssetTypeId material)
{
    if(m_material != nullptr)
        m_material->stopListeningTo(this);

    m_material = MaterialsHandler::instance()->getAsset(material);
    m_material->askForAllNotifications(this);
}

bool MeshAsset::generateModel(const std::vector<glm::vec3> &vertexList,
                            const std::vector<glm::vec2> &uvList,
                            const std::vector<glm::vec3> &normalList,
                            const std::vector<glm::vec3> &indexList)
{
    std::vector<std::tuple<glm::vec3, glm::vec2, glm::vec3> > trueVertexList;
    std::vector<uint16_t> trueIndexList;

   ///Omg this will be so inefficient, why am I doing this
    for(auto &index : indexList)
    {
        glm::vec3 vertex = vertexList[index.x];
        glm::vec2 uv{0,0};
        glm::vec3 normal{0,0,1.0};

        if(index.y != -1)
            uv = uvList[index.y];

        if(index.z != -1)
            normal = normalList[index.z];

        size_t foundedVertex = 0;

        while(foundedVertex < trueVertexList.size())
        {
            if(std::get<0>(trueVertexList[foundedVertex]) == vertex
            && std::get<1>(trueVertexList[foundedVertex]) == uv
            && std::get<2>(trueVertexList[foundedVertex]) == normal)
                break;
            else
                foundedVertex++;
        }

        trueIndexList.push_back(foundedVertex);

        if(foundedVertex == trueVertexList.size())
            trueVertexList.push_back({vertex, uv, normal});
    }

    return this->generateModel(trueVertexList, trueIndexList);
}


bool MeshAsset::generateModel(std::vector<std::tuple<glm::vec3, glm::vec2, glm::vec3> > &vertexList,
                              std::vector<uint16_t> &indexList)
{
    std::vector<MeshVertex> meshVertexList;

    for(auto &vertex : vertexList)
    {
        meshVertexList.push_back({});
        meshVertexList.back().pos = std::get<0>(vertex);
        meshVertexList.back().uv = std::get<1>(vertex);
        meshVertexList.back().normal = std::get<2>(vertex);

        meshVertexList.back().albedo_color = glm::vec4(1.0,1.0,1.0,1.0);

        if(m_material != nullptr)
        {
            meshVertexList.back().rmt_color = m_material->getRmtFactor();

            meshVertexList.back().albedo_texId = {m_material->getAlbedoMap().m_textureId,
                                                  m_material->getAlbedoMap().m_textureLayer};

            meshVertexList.back().height_texId = {m_material->getHeightMap().m_textureId,
                                                  m_material->getHeightMap().m_textureLayer};

            meshVertexList.back().normal_texId = {m_material->getNormalMap().m_textureId,
                                                  m_material->getNormalMap().m_textureLayer};

            meshVertexList.back().rmt_texId    = {m_material->getRmtMap().m_textureId,
                                                  m_material->getRmtMap().m_textureLayer};

        }
    }

    VkDeviceSize vertexBufferSize   = sizeof(MeshVertex) * meshVertexList.size();
    VkDeviceSize indexBufferSize    = sizeof(uint16_t) * indexList.size();

    VBuffer vertexStaging,
            indexStaging;

    VBuffersAllocator::allocBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   vertexStaging);

    VBuffersAllocator::writeBuffer(vertexStaging, meshVertexList.data(), vertexBufferSize);

    VBuffersAllocator::allocBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   indexStaging);

    VBuffersAllocator::writeBuffer(indexStaging, indexList.data(), indexBufferSize);

    VBuffersAllocator::allocBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   m_vertexBuffer);

    VBuffersAllocator::copyBuffer(vertexStaging, m_vertexBuffer, vertexBufferSize);

    VBuffersAllocator::allocBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   m_indexBuffer);

    VBuffersAllocator::copyBuffer(indexStaging, m_indexBuffer, indexBufferSize);

    VBuffersAllocator::freeBuffer(vertexStaging);
    VBuffersAllocator::freeBuffer(indexStaging);

    m_indexCount = indexList.size();

    return (true);
}

VBuffer MeshAsset::getVertexBuffer()
{
    return m_vertexBuffer;
}

VBuffer MeshAsset::getIndexBuffer()
{
    return m_indexBuffer;
}

size_t MeshAsset::getIndexCount()
{
    return m_indexCount;
}

}
