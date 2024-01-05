#pragma once

#include <vector>

#include "Glacier/RenderPrimitive/SPrimObjectHeader.h"
#include "Glacier/RenderPrimitive/SPrimMesh.h"
#include "Glacier/RenderPrimitive/SPrimSubMesh.h"
#include "Glacier/RenderPrimitive/SPrimMeshWeighted.h"
#include "Glacier/Math/SVector2.h"

#include "IO/BinaryReader.h"
#include "Resources/RenderMaterialInstance.h"
#include "Glacier/RenderPrimitive/SBoneInfo.h"

class RenderPrimitive
{
public:
	struct ColorRGBA
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};

	struct Vertex
	{
		SVector4 position;
		SVector4 normal;
		SVector4 tangent;
		SVector4 binormal;
		std::vector<SVector2> uv;
		ColorRGBA color;
		float weights[4];
		unsigned char boneRemapValues[4];
	};

	class Mesh
	{
	public:
		virtual void Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions) = 0;
		void ReadIndices(BinaryReader& binaryReader);
		virtual void ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions) = 0;
		void ReadVertexPosition(BinaryReader& binaryReader, const unsigned int vertexIndex, const bool hasHighResolutionPositions, const SVector4& scale, const SVector4& bias);
		void ReadVertexWeighsAndBoneRemapValues(BinaryReader& binaryReader, const unsigned int vertexIndex);
		void ReadVertexNormal(BinaryReader& binaryReader, const unsigned int vertexIndex);
		void ReadVertexTangent(BinaryReader& binaryReader, const unsigned int vertexIndex);
		void ReadVertexBinormal(BinaryReader& binaryReader, const unsigned int vertexIndex);
		void ReadVertexUVs(BinaryReader& binaryReader, const unsigned int vertexIndex, const SVector2& scale, const SVector2& bias);
		void ReadVertexColor(BinaryReader& binaryReader, const unsigned int vertexIndex);
		const SPrimObject::SUBTYPE GetSubType() const;
		const unsigned int GetIndexCount() const;
		const unsigned int GetVertexCount() const;
		const std::vector<unsigned short>& GetIndices() const;
		const std::vector<Vertex>& GetVertices() const;
		std::vector<Vertex>& GetVertices();
		virtual const bool IsWeighted() const = 0;
		virtual const unsigned short GetMaterialID() const = 0;
		virtual const unsigned char GetLODMask() const = 0;

	protected:
		SPrimSubMesh primSubMesh;
		std::vector<unsigned short> indices;
		std::vector<Vertex> vertices;
	};

	class StandardMesh : public Mesh
	{
	public:
		void Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions) override;
		void ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions) override;
		const bool IsWeighted() const override;
		const unsigned short GetMaterialID() const override;
		const unsigned char GetLODMask() const override;

	private:
		SPrimMesh primMesh;
	};

	class LinkedMesh : public Mesh
	{
	public:
		void Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions) override;
		void ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions) override;
		const bool IsWeighted() const override;
		const unsigned short GetMaterialID() const override;
		const unsigned char GetLODMask() const override;

	private:
		SPrimMeshWeighted primMeshWeighted;
		SBoneInfo boneInfo;
	};

	class WeightedMesh : public Mesh
	{
	public:
		void Deserialize(BinaryReader& binaryReader, const bool hasHighResolutionPositions) override;
		void ReadVertices(BinaryReader& binaryReader, const bool hasHighResolutionPositions) override;
		const bool IsWeighted() const override;
		const unsigned short GetMaterialID() const override;
		const unsigned char GetLODMask() const override;
		const unsigned char GetBoneIndex(unsigned char boneRemapValue) const;

	private:
		SPrimMeshWeighted primMeshWeighted;
		SBoneInfo boneInfo;
	};

	RenderPrimitive(Resource& resource);
	~RenderPrimitive();
	const SPrimObjectHeader& GetPrimObjectHeader() const;
	const unsigned int GetBoneRigResourceIndex() const;
	const std::vector<Mesh*>& GetMeshes() const;
	void Deserialize();
	void ConvertToOBJ(const std::string& outputFolderPath);
	void ExportMaterialAndTextures(const unsigned short materialID, std::ofstream& objFile, const std::string& outputFolderPath);

private:
	Resource& resource;
	SPrimObjectHeader primObjectHeader;
	std::vector<Mesh*> meshes;
};
