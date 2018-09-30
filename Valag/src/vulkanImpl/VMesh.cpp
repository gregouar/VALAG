#include "Valag/vulkanImpl/VMesh.h"

#include "Valag/vulkanImpl/VBuffersAllocator.h"

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

std::array<VkVertexInputAttributeDescription, 5> MeshVertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

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
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, tangent);
    ++i;

    attributeDescriptions[i].binding = b;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(MeshVertex, bitangent);
    ++i;

    return attributeDescriptions;
}



VMesh::VMesh()
{
    //ctor
}

VMesh::~VMesh()
{
    VBuffersAllocator::freeBuffer(m_vertexBuffer);
    VBuffersAllocator::freeBuffer(m_indexBuffer);
}



bool VMesh::generateMesh(std::vector<MeshVertex> &vertexList,
                         std::vector<uint16_t> &indexList,
                         CommandPoolName poolName)
{
   /* std::vector<MeshVertex> meshVertexList;

    for(auto &vertex : vertexList)
    {
        meshVertexList.push_back({});
        meshVertexList.back().pos = std::get<0>(vertex);
        meshVertexList.back().uv = std::get<1>(vertex);
        meshVertexList.back().normal = std::get<2>(vertex);*/

        /*meshVertexList.back().albedo_color = glm::vec4(1.0,1.0,1.0,1.0);

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

        }*/
  //  }

    ///I could have some kind of VMesh heh
    VkDeviceSize vertexBufferSize   = sizeof(MeshVertex) * vertexList.size();
    VkDeviceSize indexBufferSize    = sizeof(uint16_t) * indexList.size();

    VBuffer vertexStaging,
            indexStaging;

    VBuffersAllocator::allocBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   vertexStaging);

    VBuffersAllocator::writeBuffer(vertexStaging, vertexList.data(), vertexBufferSize);

    VBuffersAllocator::allocBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   indexStaging);

    VBuffersAllocator::writeBuffer(indexStaging, indexList.data(), indexBufferSize);

    VBuffersAllocator::allocBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   m_vertexBuffer);

    VBuffersAllocator::copyBuffer(vertexStaging, m_vertexBuffer, vertexBufferSize, poolName);

    VBuffersAllocator::allocBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   m_indexBuffer);

    VBuffersAllocator::copyBuffer(indexStaging, m_indexBuffer, indexBufferSize, poolName);

    VBuffersAllocator::freeBuffer(vertexStaging);
    VBuffersAllocator::freeBuffer(indexStaging);

    m_indexCount = indexList.size();

    return (true);
}

VBuffer VMesh::getVertexBuffer()
{
    return m_vertexBuffer;
}

VBuffer VMesh::getIndexBuffer()
{
    return m_indexBuffer;
}

size_t VMesh::getIndexCount()
{
    return m_indexCount;
}

}
