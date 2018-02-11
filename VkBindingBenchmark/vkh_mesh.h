#pragma once
#include "vkh.h"

#define GLM_FORCE_RADIANS
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

namespace vkh
{
	enum class EMeshVertexAttribute : uint8_t
	{
		POSITION,
		UV0,
		UV1,
		NORMAL,
		TANGENT,
		BITANGENT,
		COLOR
	};

	struct VertexRenderData
	{
		VkVertexInputAttributeDescription* attrDescriptions;
		EMeshVertexAttribute* attributes;
		uint32_t attrCount;
		uint32_t vertexSize;
	};

	struct MeshAsset
	{
		VkBuffer vBuffer;
		VkBuffer iBuffer;

		Allocation vBufferMemory;
		Allocation iBufferMemory;

		uint32_t vCount;
		uint32_t iCount;

		glm::vec3 min;
		glm::vec3 max;
	};
}

namespace vkh::Mesh
{
	void setGlobalVertexLayout(std::vector<EMeshVertexAttribute> layout);

	const VertexRenderData* vertexRenderData();
	void make(MeshAsset& outAsset, VkhContext& ctxt, float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);
	void quad(MeshAsset& outAsset, VkhContext& ctxt, float width = 2.0f, float height = 2.0f, float xOffset = 0.0f, float yOffset = 0.0f);
}