#include <unordered_map>
#include <tiny_obj_loader.h>
#include "mesh_loading.h"
#include "vkh_mesh.h"


#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


#include "config.h"

#include "tiny_obj_loader.h"

#if COMBINE_MESHES
static const int defaultFlags =  aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices | aiProcess_FlipWindingOrder | aiProcess_Triangulate;
#else
static const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate;
#endif









std::vector<vkh::MeshAsset> loadMesh(const char* filepath, bool combineSubMeshes, vkh::VkhContext& ctxt)
{
	using namespace vkh;
	std::vector<MeshAsset> outMeshes;

	const VertexRenderData* globalVertLayout = Mesh::vertexRenderData();

	Assimp::Importer aiImporter;
	const struct aiScene* scene = NULL;

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&stream);

	scene = aiImporter.ReadFile(filepath, defaultFlags);

	const aiVector3D ZeroVector(0.0, 0.0, 0.0);
	const aiColor4D ZeroColor(0.0, 0.0, 0.0, 0.0);

	if (scene)
	{
		uint32_t floatsPerVert = globalVertLayout->vertexSize / sizeof(float);
		std::vector<float> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		uint32_t numVerts = 0;
		uint32_t numFaces = 0;

		outMeshes.resize(combineSubMeshes ? 1 : scene->mNumMeshes);

		for (uint32_t mIdx = 0; mIdx < scene->mNumMeshes; mIdx++)
		{
			if (!combineSubMeshes)
			{
				vertexBuffer.clear();
				indexBuffer.clear();
				numVerts = 0;
				numFaces = 0;
			}

			const aiMesh* mesh = scene->mMeshes[mIdx];

			for (uint32_t vIdx = 0; vIdx < mesh->mNumVertices; ++vIdx)
			{

				const aiVector3D* pos = &(mesh->mVertices[vIdx]);
				const aiVector3D* nrm = &(mesh->mNormals[vIdx]);
				const aiVector3D* uv0 = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][vIdx]) : &ZeroVector;
				const aiVector3D* uv1 = mesh->HasTextureCoords(1) ? &(mesh->mTextureCoords[1][vIdx]) : &ZeroVector;
				const aiVector3D* tan = mesh->HasTangentsAndBitangents() ? &(mesh->mTangents[vIdx]) : &ZeroVector;
				const aiVector3D* biTan = mesh->HasTangentsAndBitangents() ? &(mesh->mBitangents[vIdx]) : &ZeroVector;
				const aiColor4D* col = mesh->HasVertexColors(0) ? &(mesh->mColors[0][vIdx]) : &ZeroColor;

				for (uint32_t lIdx = 0; lIdx < globalVertLayout->attrCount; ++lIdx)
				{
					EMeshVertexAttribute comp = globalVertLayout->attributes[lIdx];

					switch (comp)
					{
						case EMeshVertexAttribute::POSITION:
						{
							vertexBuffer.push_back(pos->x);
							vertexBuffer.push_back(pos->y);
							vertexBuffer.push_back(pos->z);
						}; break;
						case EMeshVertexAttribute::NORMAL:
						{
							vertexBuffer.push_back(nrm->x);
							vertexBuffer.push_back(nrm->y);
							vertexBuffer.push_back(nrm->z);
						}; break;
						case EMeshVertexAttribute::UV0:
						{
							vertexBuffer.push_back(uv0->x);
							vertexBuffer.push_back(uv0->y);
						}; break;
						case EMeshVertexAttribute::UV1:
						{
							vertexBuffer.push_back(uv1->x);
							vertexBuffer.push_back(uv1->y);
						}; break;
						case EMeshVertexAttribute::TANGENT:
						{
							vertexBuffer.push_back(tan->x);
							vertexBuffer.push_back(tan->y);
							vertexBuffer.push_back(tan->z);
						}; break;
						case EMeshVertexAttribute::BITANGENT:
						{
							vertexBuffer.push_back(biTan->x);
							vertexBuffer.push_back(biTan->y);
							vertexBuffer.push_back(biTan->z);
						}; break;

						case EMeshVertexAttribute::COLOR:
						{
							vertexBuffer.push_back(col->r);
							vertexBuffer.push_back(col->g);
							vertexBuffer.push_back(col->b);
							vertexBuffer.push_back(col->a);
						}; break;
					}

				}
			}

			for (unsigned int fIdx = 0; fIdx < mesh->mNumFaces; fIdx++)
			{
				const aiFace& face = mesh->mFaces[fIdx];
				checkf(face.mNumIndices == 3, "unsupported number of indices in mesh face");

				indexBuffer.push_back(numVerts + face.mIndices[0]);
				indexBuffer.push_back(numVerts + face.mIndices[1]);
				indexBuffer.push_back(numVerts + face.mIndices[2]);
			}

			numVerts += mesh->mNumVertices;
			numFaces += mesh->mNumFaces;

			if (!combineSubMeshes)
			{
				vkh::Mesh::make(outMeshes[mIdx], ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());
			}
		}

		if (combineSubMeshes)
		{
			vkh::Mesh::make(outMeshes[0], ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());
		}
	}

	aiDetachAllLogStreams();

	return outMeshes;
}















//std::vector<vkh::MeshAsset> loadMesh(const char *filepath, bool combineSubMeshes, vkh::VkhContext &ctxt) {
//  using namespace vkh;
//  std::vector<MeshAsset> outMeshes;
//
//  const VertexRenderData *globalVertLayout = Mesh::vertexRenderData();
//
//  // Use TinyObjLoader to read in all the mesh file data
//  tinyobj::attrib_t attrib;
//  std::vector<tinyobj::shape_t> shapes;
//  std::vector<tinyobj::material_t> materials;
//  std::string err;
//  bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath);
//  checkf(success, "tinyObj::LoadObj(): %s", err.c_str());
//
//  if (success) {
//    uint32_t floatsPerVert = globalVertLayout->vertexSize / sizeof(float);
//    std::vector<float> vertexBuffer;
//    std::vector<uint32_t> indexBuffer;
//    uint32_t numVerts = 0;
//    uint32_t numFaces = 0;
//
//    outMeshes.resize(combineSubMeshes ? 1 : shapes.size());
//
//
//    for (const auto &shape : shapes) {
//
//      if (!combineSubMeshes) {
//        vertexBuffer.clear();
//        indexBuffer.clear();
//        numVerts = 0;
//        numFaces = 0;
//      }
//
//      for (const auto &index : shape.mesh.indices) {
//
//        const glm::vec3 pos = {
//            attrib.vertices[3 * index.vertex_index + 0],
//            attrib.vertices[3 * index.vertex_index + 1],
//            attrib.vertices[3 * index.vertex_index + 2]
//        };
//
//        const glm::vec3 nrm = {
//            attrib.normals[3 * index.normal_index + 0],
//            attrib.normals[3 * index.normal_index + 1],
//            attrib.normals[3 * index.normal_index + 2]
//        };
//
//        const glm::vec2 uv0 = {
//            attrib.texcoords[2 * index.texcoord_index + 0],
//            /*1.0f - */attrib.texcoords[2 * index.texcoord_index + 1]
//        };
//
//        for (uint32_t lIdx = 0; lIdx < globalVertLayout->attrCount; ++lIdx) {
//          EMeshVertexAttribute comp = globalVertLayout->attributes[lIdx];
//
//          switch (comp) {
//            case EMeshVertexAttribute::POSITION: {
//              vertexBuffer.push_back(pos.x);
//              vertexBuffer.push_back(pos.y);
//              vertexBuffer.push_back(pos.z);
//            }; break;
//            case EMeshVertexAttribute::NORMAL: {
//              vertexBuffer.push_back(nrm.x);
//              vertexBuffer.push_back(nrm.y);
//              vertexBuffer.push_back(nrm.z);
//            }; break;
//            case EMeshVertexAttribute::UV0: {
//              vertexBuffer.push_back(uv0.x);
//              vertexBuffer.push_back(uv0.y);
//            }; break;
//            case EMeshVertexAttribute::UV1: {
//              checkf(false, "Reading UV1 from obj not currently supported.\n");
//            }; break;
//            case EMeshVertexAttribute::TANGENT: {
//              checkf(false, "Reading tangents from obj not currently supported.\n");
//            }; break;
//            case EMeshVertexAttribute::BITANGENT: {
//              checkf(false, "Reading bitangents from obj not currently supported.\n");
//            }; break;
//            case EMeshVertexAttribute::COLOR: {
//              checkf(false, "Reading color from obj not currently supported.\n");
//            }; break;
//          }
//        }
//      }
//
//
//
//
//      for (auto &face : shape.mesh.num_face_vertices)
//
//
//
//
//      for (unsigned int fIdx = 0; fIdx < mesh->mNumFaces; fIdx++) {
//        const aiFace &face = mesh->mFaces[fIdx];
//        checkf(face.mNumIndices == 3, "unsupported number of indices in mesh face");
//
//        indexBuffer.push_back(numVerts + face.mIndices[0]);
//        indexBuffer.push_back(numVerts + face.mIndices[1]);
//        indexBuffer.push_back(numVerts + face.mIndices[2]);
//      }
//
//      numVerts += mesh->mNumVertices;
//      numFaces += mesh->mNumFaces;
//
//      if (!combineSubMeshes) {
//        vkh::Mesh::make(outMeshes[mIdx], ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());
//      }
//    }
//    if (combineSubMeshes) {
//      vkh::Mesh::make(outMeshes[0], ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());
//    }
//  }
//
//
//  return outMeshes;
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
////    for (uint32_t mIdx = 0; mIdx < shapes.size(); mIdx++) {
////      if (!combineSubMeshes) {
////        vertexBuffer.clear();
////        indexBuffer.clear();
////        numVerts = 0;
////        numFaces = 0;
////      }
////
//////			const aiMesh* mesh = scene->mMeshes[mIdx];
////      tinyobj::shape_t mesh = shapes.at(mIdx);
////
////      for (uint32_t vIdx = 0; vIdx < mesh->mNumVertices; ++vIdx) {
////
////        const aiVector3D *pos = &(mesh->mVertices[vIdx]);
////        const aiVector3D *nrm = &(mesh->mNormals[vIdx]);
////        const aiVector3D *uv0 = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][vIdx]) : &ZeroVector;
////        const aiVector3D *uv1 = mesh->HasTextureCoords(1) ? &(mesh->mTextureCoords[1][vIdx]) : &ZeroVector;
////        const aiVector3D *tan = mesh->HasTangentsAndBitangents() ? &(mesh->mTangents[vIdx]) : &ZeroVector;
////        const aiVector3D *biTan = mesh->HasTangentsAndBitangents() ? &(mesh->mBitangents[vIdx]) : &ZeroVector;
////        const aiColor4D *col = mesh->HasVertexColors(0) ? &(mesh->mColors[0][vIdx]) : &ZeroColor;
////
////        for (uint32_t lIdx = 0; lIdx < globalVertLayout->attrCount; ++lIdx) {
////          EMeshVertexAttribute comp = globalVertLayout->attributes[lIdx];
////
////          switch (comp) {
////            case EMeshVertexAttribute::POSITION: {
////              vertexBuffer.push_back(pos->x);
////              vertexBuffer.push_back(pos->y);
////              vertexBuffer.push_back(pos->z);
////            };
////              break;
////            case EMeshVertexAttribute::NORMAL: {
////              vertexBuffer.push_back(nrm->x);
////              vertexBuffer.push_back(nrm->y);
////              vertexBuffer.push_back(nrm->z);
////            };
////              break;
////            case EMeshVertexAttribute::UV0: {
////              vertexBuffer.push_back(uv0->x);
////              vertexBuffer.push_back(uv0->y);
////            };
////              break;
////            case EMeshVertexAttribute::UV1: {
////              vertexBuffer.push_back(uv1->x);
////              vertexBuffer.push_back(uv1->y);
////            };
////              break;
////            case EMeshVertexAttribute::TANGENT: {
////              vertexBuffer.push_back(tan->x);
////              vertexBuffer.push_back(tan->y);
////              vertexBuffer.push_back(tan->z);
////            };
////              break;
////            case EMeshVertexAttribute::BITANGENT: {
////              vertexBuffer.push_back(biTan->x);
////              vertexBuffer.push_back(biTan->y);
////              vertexBuffer.push_back(biTan->z);
////            };
////              break;
////
////            case EMeshVertexAttribute::COLOR: {
////              vertexBuffer.push_back(col->r);
////              vertexBuffer.push_back(col->g);
////              vertexBuffer.push_back(col->b);
////              vertexBuffer.push_back(col->a);
////            };
////              break;
////          }
////
////        }
////      }
////
////      for (unsigned int fIdx = 0; fIdx < mesh->mNumFaces; fIdx++) {
////        const aiFace &face = mesh->mFaces[fIdx];
////        checkf(face.mNumIndices == 3, "unsupported number of indices in mesh face");
////
////        indexBuffer.push_back(numVerts + face.mIndices[0]);
////        indexBuffer.push_back(numVerts + face.mIndices[1]);
////        indexBuffer.push_back(numVerts + face.mIndices[2]);
////      }
////
////      numVerts += mesh->mNumVertices;
////      numFaces += mesh->mNumFaces;
////
////      if (!combineSubMeshes) {
////        vkh::Mesh::make(outMeshes[mIdx], ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());
////      }
////    }
////
////    if (combineSubMeshes) {
////      vkh::Mesh::make(outMeshes[0], ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());
////    }
////  }
////
////
////  return outMeshes;
////}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//struct Vertex {
//  glm::vec3 pos;
//  glm::vec3 color;
//  glm::vec2 texCoord;
//
//  static VkVertexInputBindingDescription getBindingDescription();
//  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
//  bool operator==(const Vertex &other) const;
//};
//
//
//std::vector<vkh::MeshAsset> loadModel(const char *filepath, bool combineSubMeshes, vkh::VkhContext &ctxt) {
//
//  tinyobj::attrib_t attrib;
//  std::vector<tinyobj::shape_t> shapes;
//  std::vector<tinyobj::material_t> materials;
//  std::string err;
//
//  bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath);
//  checkf(success, "tinyObj::LoadObj(): %s", err.c_str());
//
//  std::unordered_map<Vertex, uint32_t> uniqueVertices;
//
//  for (const auto &shape : shapes) {
//    for (const auto &index : shape.mesh.indices) {
//      Vertex vertex = {};
//
//      vertex.pos = {
//          attrib.vertices[3 * index.vertex_index + 0],
//          attrib.vertices[3 * index.vertex_index + 1],
//          attrib.vertices[3 * index.vertex_index + 2]
//      };
//
//      vertex.texCoord = {
//          attrib.texcoords[2 * index.texcoord_index + 0],
//          1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//      };
//
//      vertex.color = {1.0f, 1.0f, 1.0f};
//
//      if (uniqueVertices.count(vertex) == 0) {
//        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
//        vertices.push_back(vertex);
//      }
//
//      indices.push_back(uniqueVertices[vertex]);
//    }
//  }
//}
//
//
//VkVertexInputBindingDescription Vertex::getBindingDescription() {
//  VkVertexInputBindingDescription bindingDescription = {};
//  bindingDescription.binding = 0;
//  bindingDescription.stride = sizeof(Vertex);
//  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return bindingDescription;
//}
//
//std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
//  std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
//
//  attributeDescriptions[0].binding = 0;
//  attributeDescriptions[0].location = 0;
//  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attributeDescriptions[0].offset = offsetof(Vertex, pos);
//
//  attributeDescriptions[1].binding = 0;
//  attributeDescriptions[1].location = 1;
//  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attributeDescriptions[1].offset = offsetof(Vertex, color);
//
//  attributeDescriptions[2].binding = 0;
//  attributeDescriptions[2].location = 2;
//  attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//  attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
//
//  return attributeDescriptions;
//}
//
//bool Vertex::operator==(const Vertex &other) const {
//  return pos == other.pos && color == other.color && texCoord == other.texCoord;
//}
