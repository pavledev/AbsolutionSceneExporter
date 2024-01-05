#pragma once

#include <vector>
#include <string>

#include "Glacier/Material/SProperty.h"
#include "Glacier/Material/SRMaterialPropertyList.h"
#include "Glacier/Material/PROPERTY_TYPE.h"

#include "IO/BinaryReader.h"
#include "Resource.h"

class RenderMaterialInstance
{
public:
	struct Property
	{
		SProperty propertyInfo;
		std::vector<Property> childProperties;
		unsigned int uint32Value;
		float floatValue;
		std::string stringValue;
	};

	struct Texture
	{
		enum class Type
		{
			Unknown,
			Diffuse,
			Normal,
			Specular,
			Emissive
		};

		Type type;
		int textureReferenceIndex = -1;
		std::string filePath;
	};

	RenderMaterialInstance(Resource& resource);
	void Deserialize();
	void SerializeToJson();
	void SerializeProperty(Property& property, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	void ReadProperty(Property& property, BinaryReader& binaryReader, const unsigned int propertyOffset);
	void GetTextures(Resource* matiResource, std::vector<RenderMaterialInstance::Texture>& textures);
	void GetTextures(const Property& property, Resource* matiResource, std::vector<Texture>& textures, bool& foundNormalTexture, bool& foundDiffuseTexture, bool& foundSpecularTexture);

private:
	Resource& resource;
	SRMaterialPropertyList materialPropertyList;
	Property instanceProperty;
};
