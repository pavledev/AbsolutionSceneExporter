#include <iostream>

#include "Glacier/RenderPrimitive/SPrimObject.h"

#include "Resources/RenderPrimitive.h"
#include "Utility/StringUtility.h"
#include "Utility/ResourceUtility.h"
#include "IO/BinaryWriter.h"
#include "Resources/Texture.h"

#undef min
#undef max

RenderPrimitive::RenderPrimitive(Resource& resource) : resource(resource)
{
}

RenderPrimitive::~RenderPrimitive()
{
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		delete meshes[i];
	}

	meshes.clear();
}

const SPrimObjectHeader& RenderPrimitive::GetPrimObjectHeader() const
{
	return primObjectHeader;
}

const unsigned int RenderPrimitive::GetBoneRigResourceIndex() const
{
	return primObjectHeader.lBoneRigResourceIndex;
}

const std::vector<RenderPrimitive::Mesh*>& RenderPrimitive::GetMeshes() const
{
	return meshes;
}

void RenderPrimitive::Deserialize()
{
	BinaryReader binaryReader = BinaryReader(resource.GetResourceData(), resource.GetResourceDataSize());
	const unsigned int primaryOffset = binaryReader.Read<unsigned int>();

	binaryReader.Seek(primaryOffset, SeekOrigin::Begin);

	primObjectHeader = binaryReader.Read<SPrimObjectHeader>();
	const SPrimHeader::EPrimType primType = static_cast<SPrimHeader::EPrimType>(primObjectHeader.lType);
	const SPrimObjectHeader::PROPERTY_FLAGS propertyFlags = static_cast<SPrimObjectHeader::PROPERTY_FLAGS>(primObjectHeader.lPropertyFlags);
	const bool hasHighResolutionPositions = (propertyFlags & SPrimObjectHeader::PROPERTY_FLAGS::HAS_HIRES_POSITIONS) == SPrimObjectHeader::PROPERTY_FLAGS::HAS_HIRES_POSITIONS;

	if (primType != SPrimHeader::EPrimType::PTOBJECTHEADER)
	{
		std::cout << SPrimHeader::ConvertPrimTypeToString(primType) << " prim header type isn't supported" << std::endl;

		return;
	}

	std::vector<unsigned int> objectOffsets;

	objectOffsets.reserve(primObjectHeader.lNumObjects);
	meshes.reserve(primObjectHeader.lNumObjects);

	binaryReader.Seek(primObjectHeader.lObjectTable, SeekOrigin::Begin);

	for (unsigned int i = 0; i < primObjectHeader.lNumObjects; ++i)
	{
		const unsigned int objectOffset = binaryReader.Read<unsigned int>();

		objectOffsets.push_back(objectOffset);
	}

	for (unsigned int i = 0; i < primObjectHeader.lNumObjects; ++i)
	{
		binaryReader.Seek(objectOffsets[i] + 2, SeekOrigin::Begin);

		const SPrimHeader::EPrimType primType = static_cast<SPrimHeader::EPrimType>(binaryReader.Read<unsigned short>());

		if (primType != SPrimHeader::EPrimType::PTMESH)
		{
			continue;
		}

		const SPrimObject::SUBTYPE subType = static_cast<SPrimObject::SUBTYPE>(binaryReader.Read<unsigned char>());

		binaryReader.Seek(objectOffsets[i], SeekOrigin::Begin);

		switch (subType)
		{
			case SPrimObject::SUBTYPE::SUBTYPE_STANDARD:
			{
				StandardMesh* standardMesh = new StandardMesh();

				standardMesh->Deserialize(binaryReader, hasHighResolutionPositions);

				meshes.push_back(standardMesh);

				break;
			}
			case SPrimObject::SUBTYPE::SUBTYPE_LINKED:
			{
				LinkedMesh* linkedMesh = new LinkedMesh();

				linkedMesh->Deserialize(binaryReader, hasHighResolutionPositions);

				meshes.push_back(linkedMesh);

				break;
			}
			case SPrimObject::SUBTYPE::SUBTYPE_WEIGHTED:
			{
				WeightedMesh* weightedMesh = new WeightedMesh();

				weightedMesh->Deserialize(binaryReader, hasHighResolutionPositions);

				meshes.push_back(weightedMesh);

				break;
			}
			default:
				std::cout << SPrimObject::ConvertSubTypeToString(subType) << " prim object sub type isn't supported" << std::endl;
				break;
		}
	}
}

void RenderPrimitive::Mesh::ReadIndices(BinaryReader& binaryReader)
{
	indices.reserve(primSubMesh.lNumIndices);

	for (unsigned int i = 0; i < primSubMesh.lNumIndices; ++i)
	{
		indices.push_back(binaryReader.Read<unsigned short>());
	}
}

void RenderPrimitive::Mesh::ReadVertexPosition(BinaryReader& binaryReader, const unsigned int vertexIndex, const bool hasHighResolutionPositions, const SVector4& scale, const SVector4& bias)
{
	if (hasHighResolutionPositions)
	{
		float x = binaryReader.Read<float>();
		float y = binaryReader.Read<float>();
		float z = binaryReader.Read<float>();

		vertices[vertexIndex].position = SVector4(x, y, z, 1.0f);
	}
	else
	{
		short x = binaryReader.Read<short>();
		short y = binaryReader.Read<short>();
		short z = binaryReader.Read<short>();
		short w = binaryReader.Read<short>();

		vertices[vertexIndex].position = SVector4(
			static_cast<float>(x * scale.x) / std::numeric_limits<short>::max() + bias.x,
			static_cast<float>(y * scale.y) / std::numeric_limits<short>::max() + bias.y,
			static_cast<float>(z * scale.z) / std::numeric_limits<short>::max() + bias.z,
			static_cast<float>(w * scale.w) / std::numeric_limits<short>::max() + bias.w);
	}
}

void RenderPrimitive::Mesh::ReadVertexWeighsAndBoneRemapValues(BinaryReader& binaryReader, const unsigned int vertexIndex)
{
	unsigned char weightA = binaryReader.Read<unsigned char>();
	unsigned char weightB = binaryReader.Read<unsigned char>();
	unsigned char weightC = binaryReader.Read<unsigned char>();
	unsigned char weightD = binaryReader.Read<unsigned char>();

	if (weightA == 1)
	{
		weightA = 0;
		weightD += 1;
	}

	vertices[vertexIndex].weights[0] = static_cast<float>(weightA) / 255;
	vertices[vertexIndex].weights[1] = static_cast<float>(weightB) / 255;
	vertices[vertexIndex].weights[2] = static_cast<float>(weightC) / 255;
	vertices[vertexIndex].weights[3] = static_cast<float>(weightD) / 255;

	for (unsigned char i = 0; i < 4; ++i)
	{
		vertices[vertexIndex].boneRemapValues[i] = binaryReader.Read<unsigned char>();
	}
}

void RenderPrimitive::Mesh::ReadVertexNormal(BinaryReader& binaryReader, const unsigned int vertexIndex)
{
	float x = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float y = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float z = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float w = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;

	vertices[vertexIndex].normal = SVector4(x, y, z, w);
}

void RenderPrimitive::Mesh::ReadVertexTangent(BinaryReader& binaryReader, const unsigned int vertexIndex)
{
	float x = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float y = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float z = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float w = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;

	vertices[vertexIndex].tangent = SVector4(x, y, z, w);
}

void RenderPrimitive::Mesh::ReadVertexBinormal(BinaryReader& binaryReader, const unsigned int vertexIndex)
{
	float x = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float y = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float z = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;
	float w = ((2.0f * static_cast<float>(binaryReader.Read<unsigned char>())) / 255.0f) - 1.0f;

	vertices[vertexIndex].binormal = SVector4(x, y, z, w);
}

void RenderPrimitive::Mesh::ReadVertexUVs(BinaryReader& binaryReader, const unsigned int vertexIndex, const SVector2& scale, const SVector2& bias)
{
	float x = scale.x * static_cast<float>(binaryReader.Read<short>()) / std::numeric_limits<short>::max() + bias.x;
	float y = scale.y * static_cast<float>(binaryReader.Read<short>()) / std::numeric_limits<short>::max() + bias.y;

	vertices[vertexIndex].uv.push_back(SVector2(x, y));
}

void RenderPrimitive::Mesh::ReadVertexColor(BinaryReader& binaryReader, const unsigned int vertexIndex)
{
	unsigned char r = binaryReader.Read<unsigned char>();
	unsigned char g = binaryReader.Read<unsigned char>();
	unsigned char b = binaryReader.Read<unsigned char>();
	unsigned char a = binaryReader.Read<unsigned char>();

	vertices[vertexIndex].color = ColorRGBA(r, g, b, a);
}

const SPrimObject::SUBTYPE RenderPrimitive::Mesh::GetSubType() const
{
	return static_cast<SPrimObject::SUBTYPE>(primSubMesh.lSubType);
}

const unsigned int RenderPrimitive::Mesh::GetIndexCount() const
{
	return primSubMesh.lNumIndices;
}

const unsigned int RenderPrimitive::Mesh::GetVertexCount() const
{
	return primSubMesh.lNumVertices;
}

const std::vector<unsigned short>& RenderPrimitive::Mesh::GetIndices() const
{
	return indices;
}

const std::vector<RenderPrimitive::Vertex>& RenderPrimitive::Mesh::GetVertices() const
{
	return vertices;
}

std::vector<RenderPrimitive::Vertex>& RenderPrimitive::Mesh::GetVertices()
{
	return vertices;
}

void RenderPrimitive::StandardMesh::Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions)
{
	primMesh = binaryReader.Read<SPrimMesh>();

	binaryReader.Seek(primMesh.lSubMeshTable, SeekOrigin::Begin);

	const unsigned int subMeshOffset = binaryReader.Read<unsigned int>();

	binaryReader.Seek(subMeshOffset, SeekOrigin::Begin);

	primSubMesh = binaryReader.Read<SPrimSubMesh>();

	if (primSubMesh.lNumUVChannels == 0) //lNumUVChannels is 0 in some PRIMs that have UV coordinates
	{
		primSubMesh.lNumUVChannels = 1;
	}

	binaryReader.Seek(primSubMesh.lIndices, SeekOrigin::Begin);

	ReadIndices(binaryReader);
	ReadVertices(binaryReader, hasHighResolutionPositions);
}

void RenderPrimitive::StandardMesh::ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions)
{
	vertices.resize(primSubMesh.lNumVertices);

	binaryReader.Seek(primSubMesh.lVertices, SeekOrigin::Begin);

	for (unsigned int i = 0; i < primSubMesh.lNumVertices; ++i)
	{
		ReadVertexPosition(binaryReader, i, hasHighResolutionPositions, primMesh.vPosScale, primMesh.vPosBias);
		ReadVertexNormal(binaryReader, i);
		ReadVertexColor(binaryReader, i);
		ReadVertexTangent(binaryReader, i);
		ReadVertexBinormal(binaryReader, i);

		for (unsigned char uv = 0; uv < primSubMesh.lNumUVChannels; ++uv)
		{
			ReadVertexUVs(binaryReader, i, primMesh.vTexScale, primMesh.vTexBias);
		}
	}
}

const bool RenderPrimitive::StandardMesh::IsWeighted() const
{
	return false;
}

const unsigned short RenderPrimitive::StandardMesh::GetMaterialID() const
{
	return primMesh.lMaterialId;
}

const unsigned char RenderPrimitive::StandardMesh::GetLODMask() const
{
	return primMesh.lLODMask;
}

void RenderPrimitive::LinkedMesh::Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions)
{
	primMeshWeighted = binaryReader.Read<SPrimMeshWeighted>();

	binaryReader.Seek(primMeshWeighted.lSubMeshTable, SeekOrigin::Begin);

	const unsigned int subMeshOffset = binaryReader.Read<unsigned int>();

	binaryReader.Seek(subMeshOffset, SeekOrigin::Begin);

	primSubMesh = binaryReader.Read<SPrimSubMesh>();

	if (primSubMesh.lNumUVChannels == 0) //lNumUVChannels is 0 in some PRIMs that have UV coordinates
	{
		primSubMesh.lNumUVChannels = 1;
	}

	binaryReader.Seek(primSubMesh.lIndices, SeekOrigin::Begin);

	ReadIndices(binaryReader);
	ReadVertices(binaryReader, hasHighResolutionPositions);

	binaryReader.Seek(primMeshWeighted.lBoneInfo, SeekOrigin::Begin);

	boneInfo = binaryReader.Read<SBoneInfo>();
}

void RenderPrimitive::LinkedMesh::ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions)
{
	vertices.resize(primSubMesh.lNumVertices);

	binaryReader.Seek(primSubMesh.lVertices, SeekOrigin::Begin);

	for (unsigned int i = 0; i < primSubMesh.lNumVertices; ++i)
	{
		ReadVertexPosition(binaryReader, i, hasHighResolutionPositions, primMeshWeighted.vPosScale, primMeshWeighted.vPosBias);
	}

	for (unsigned int i = 0; i < primSubMesh.lNumVertices; ++i)
	{
		ReadVertexNormal(binaryReader, i);
		ReadVertexTangent(binaryReader, i);
		ReadVertexBinormal(binaryReader, i);

		for (unsigned char uv = 0; uv < primSubMesh.lNumUVChannels; ++uv)
		{
			ReadVertexUVs(binaryReader, i, primMeshWeighted.vTexScale, primMeshWeighted.vTexBias);
		}

		ReadVertexColor(binaryReader, i);
	}
}

const bool RenderPrimitive::LinkedMesh::IsWeighted() const
{
	return false;
}

const unsigned short RenderPrimitive::LinkedMesh::GetMaterialID() const
{
	return primMeshWeighted.lMaterialId;
}

const unsigned char RenderPrimitive::LinkedMesh::GetLODMask() const
{
	return primMeshWeighted.lLODMask;
}

void RenderPrimitive::WeightedMesh::Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions)
{
	primMeshWeighted = binaryReader.Read<SPrimMeshWeighted>();

	binaryReader.Seek(primMeshWeighted.lSubMeshTable, SeekOrigin::Begin);

	const unsigned int subMeshOffset = binaryReader.Read<unsigned int>();

	binaryReader.Seek(subMeshOffset, SeekOrigin::Begin);

	primSubMesh = binaryReader.Read<SPrimSubMesh>();

	if (primSubMesh.lNumUVChannels == 0) //lNumUVChannels is 0 in some PRIMs that have UV coordinates
	{
		primSubMesh.lNumUVChannels = 1;
	}

	binaryReader.Seek(primSubMesh.lIndices, SeekOrigin::Begin);

	ReadIndices(binaryReader);
	ReadVertices(binaryReader, hasHighResolutionPositions);

	binaryReader.Seek(primMeshWeighted.lBoneInfo, SeekOrigin::Begin);

	boneInfo = binaryReader.Read<SBoneInfo>();
}

void RenderPrimitive::WeightedMesh::ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions)
{
	vertices.resize(primSubMesh.lNumVertices);

	binaryReader.Seek(primSubMesh.lVertices, SeekOrigin::Begin);

	for (unsigned int i = 0; i < primSubMesh.lNumVertices; ++i)
	{
		ReadVertexPosition(binaryReader, i, hasHighResolutionPositions, primMeshWeighted.vPosScale, primMeshWeighted.vPosBias);
	}

	for (unsigned int i = 0; i < primSubMesh.lNumVertices; ++i)
	{
		ReadVertexWeighsAndBoneRemapValues(binaryReader, i);
	}

	for (unsigned int i = 0; i < primSubMesh.lNumVertices; ++i)
	{
		ReadVertexNormal(binaryReader, i);
		ReadVertexTangent(binaryReader, i);
		ReadVertexBinormal(binaryReader, i);

		for (unsigned char uv = 0; uv < primSubMesh.lNumUVChannels; ++uv)
		{
			ReadVertexUVs(binaryReader, i, primMeshWeighted.vTexScale, primMeshWeighted.vTexBias);
		}

		ReadVertexColor(binaryReader, i);
	}
}

const bool RenderPrimitive::WeightedMesh::IsWeighted() const
{
	return static_cast<SPrimObject::SUBTYPE>(primMeshWeighted.lSubType) == SPrimObject::SUBTYPE::SUBTYPE_WEIGHTED;
}

const unsigned short RenderPrimitive::WeightedMesh::GetMaterialID() const
{
	return primMeshWeighted.lMaterialId;
}

const unsigned char RenderPrimitive::WeightedMesh::GetLODMask() const
{
	return primMeshWeighted.lLODMask;
}

const unsigned char RenderPrimitive::WeightedMesh::GetBoneIndex(unsigned char boneRemapValue) const
{
	for (unsigned char i = 0; i < 255; ++i)
	{
		if (boneInfo.aBoneRemap[i] == boneRemapValue)
		{
			return i;
		}
	}

	return -1;
}

void RenderPrimitive::ConvertToOBJ(const std::string& outputFolderPath)
{
	std::ofstream objFile = std::ofstream(std::format("{}Models\\{}_{}.obj", outputFolderPath, resource.GetName(), resource.GetRuntimeResourceID().GetID()));
	unsigned int vertexCount = 1;

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		unsigned char lodMask = meshes[i]->GetLODMask();

		if ((lodMask & 1) != 1)
		{
			continue;
		}

		std::vector<Vertex>& vertices = meshes[i]->GetVertices();

		//objFile << "o Mesh " << i << std::endl;

		for (size_t j = 0; j < vertices.size(); ++j)
		{
			objFile << std::format("v {} {} {}\n", vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
		}

		if (vertices[0].uv.size() > 0)
		{
			for (size_t j = 0; j < vertices.size(); ++j)
			{
				std::vector<SVector2>& uv = vertices[j].uv;

				objFile << std::format("vt {} {}\n", uv[0].x, -1 * uv[0].y + 1);
			}

			ExportMaterialAndTextures(meshes[i]->GetMaterialID(), objFile, outputFolderPath);

			const std::vector<unsigned short>& indices = meshes[i]->GetIndices();

			for (size_t j = 0; j < indices.size() / 3; ++j)
			{
				unsigned int vertexIndex = indices[j * 3] + vertexCount;
				unsigned int vertexIndex2 = indices[j * 3 + 1] + vertexCount;
				unsigned int vertexIndex3 = indices[j * 3 + 2] + vertexCount;

				objFile << std::format("f {}/{} {}/{} {}/{}\n", vertexIndex, vertexIndex, vertexIndex2, vertexIndex2, vertexIndex3, vertexIndex3);
			}
		}
		else
		{
			objFile << "s 1" << std::endl;

			const std::vector<unsigned short>& indices = meshes[i]->GetIndices();

			for (size_t j = 0; j < indices.size() / 3; ++j)
			{
				unsigned int vertexIndex = indices[j * 3] + vertexCount;
				unsigned int vertexIndex2 = indices[j * 3 + 1] + vertexCount;
				unsigned int vertexIndex3 = indices[j * 3 + 2] + vertexCount;

				objFile << std::format("f {} {} {}\n", vertexIndex, vertexIndex2, vertexIndex3);
			}
		}

		vertexCount += meshes[i]->GetVertices().size();
	}

	objFile.close();
}

void RenderPrimitive::ExportMaterialAndTextures(const unsigned short materialID, std::ofstream& objFile, const std::string& outputFolderPath)
{
	static std::unordered_map<unsigned long long, std::string> extractedMaterialFilePaths;
	static std::unordered_map<unsigned long long, std::string> extractedTextureFilePaths;
	std::vector<Resource*> primReferences;

	resource.GetReferences(primReferences);

	Resource& matiReference = *primReferences[materialID];
	std::string materialResourceName = ResourceUtility::GetResourceName(matiReference.GetResourceID());
	auto iterator = extractedMaterialFilePaths.find(matiReference.GetRuntimeResourceID());

	if (iterator != extractedMaterialFilePaths.end())
	{
		objFile << "mtllib " << iterator->second << std::endl;
	}
	else
	{
		RenderMaterialInstance renderMaterialInstance = RenderMaterialInstance(matiReference);
		std::vector<RenderMaterialInstance::Texture> textures;

		matiReference.LoadResourceData();
		renderMaterialInstance.Deserialize();
		renderMaterialInstance.GetTextures(&matiReference, textures);

		std::vector<Resource*> matiReferences;

		matiReference.GetReferences(matiReferences);

		for (size_t j = 0; j < textures.size(); ++j)
		{
			unsigned int textureReferenceIndex = textures[j].textureReferenceIndex;
			std::string textureResourceID = matiReferences[textureReferenceIndex]->GetResourceID();
			std::string textureResourceName = ResourceUtility::GetResourceName(textureResourceID);
			Resource* textReference = matiReferences[textures[j].textureReferenceIndex];
			Texture texture;

			auto iterator = extractedTextureFilePaths.find(textReference->GetRuntimeResourceID());

			if (iterator != extractedTextureFilePaths.end())
			{
				textures[j].filePath = iterator->second;
			}
			else
			{
				std::string filePath = std::format("{}Textures\\{}_{}.tga", outputFolderPath, textureResourceName, textReference->GetRuntimeResourceID().GetID());

				textures[j].filePath = std::format("../Textures/{}_{}.tga", textureResourceName, textReference->GetRuntimeResourceID().GetID());

				textReference->LoadResourceData();
				texture.Deserialize(textReference->GetResourceData(), textReference->GetResourceDataSize());
				texture.ConvertTextureToTGA(filePath);
				textReference->DeleteResourceData();

				extractedTextureFilePaths.insert(std::make_pair(textReference->GetRuntimeResourceID(), textures[j].filePath));
			}
		}

		matiReference.DeleteResourceData();

		std::string filePath = std::format("../Materials/{}_{}.mtl", materialResourceName, matiReference.GetRuntimeResourceID().GetID());
		std::string mtlFilePath = std::format("{}Materials\\{}_{}.mtl", outputFolderPath, materialResourceName, matiReference.GetRuntimeResourceID().GetID());
		std::ofstream mtlFile = std::ofstream(mtlFilePath);

		objFile << "mtllib " << filePath << std::endl;

		mtlFile << "newmtl " << materialResourceName << std::endl;
		mtlFile << "Ka 1.000 1.000 1.000\nKd 1.000 1.000 1.000\nKs 0.000 0.000 0.000\nNs 1.0\nd 1.0\nillum 2\n";

		for (size_t j = 0; j < textures.size(); ++j)
		{
			if (textures[j].type == RenderMaterialInstance::Texture::Type::Diffuse)
			{
				mtlFile << "map_Kd " << textures[j].filePath << std::endl;
			}
			else if (textures[j].type == RenderMaterialInstance::Texture::Type::Normal)
			{
				mtlFile << "map_Bump " << textures[j].filePath << std::endl;
			}
			else if (textures[j].type == RenderMaterialInstance::Texture::Type::Specular)
			{
				mtlFile << "map_Ks " << textures[j].filePath << std::endl;
			}
		}

		mtlFile.close();

		extractedMaterialFilePaths.insert(std::make_pair(matiReference.GetRuntimeResourceID(), filePath));
	}

	objFile << "usemtl " << materialResourceName << std::endl;
	objFile << "s 1" << std::endl;
}
