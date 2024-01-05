#include "Resources/RenderMaterialInstance.h"
#include "Resources/Texture.h"
#include "Global.h"

RenderMaterialInstance::RenderMaterialInstance(Resource& resource) : resource(resource)
{
}

void RenderMaterialInstance::Deserialize()
{
	BinaryReader binaryReader = BinaryReader(resource.GetResourceData(), resource.GetResourceDataSize());
	unsigned int materialInfoOffset = binaryReader.Read<unsigned int>();

	binaryReader.Seek(materialInfoOffset, SeekOrigin::Begin);

	materialPropertyList = binaryReader.Read<SRMaterialPropertyList>();

	ReadProperty(instanceProperty, binaryReader, materialPropertyList.lPropertyList);
}

void RenderMaterialInstance::SerializeToJson()
{
	materialJsonBuffer.Clear();

	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(materialJsonBuffer);

	writer.StartObject();

	writer.String("materialPropertyList");
	materialPropertyList.SerializeToJson(writer);

	SerializeProperty(instanceProperty, writer);

	writer.EndObject();
}

void RenderMaterialInstance::SerializeProperty(Property& property, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
{
	static std::unordered_map<unsigned int, std::string> propertyNames;

	if (propertyNames.empty())
	{
		propertyNames.insert(std::make_pair('AREF', "Alpha Reference"));
		propertyNames.insert(std::make_pair('ATST', "Alpha Test Enabled"));
		propertyNames.insert(std::make_pair('BENA', "Blend Enabled"));
		propertyNames.insert(std::make_pair('BIND', "Binder"));
		propertyNames.insert(std::make_pair('BMOD', "Blend Mode"));
		propertyNames.insert(std::make_pair('COLO', "Color"));
		propertyNames.insert(std::make_pair('CULL', "Culling Mode"));
		propertyNames.insert(std::make_pair('DBDE', "Decal Blend Diffuse"));
		propertyNames.insert(std::make_pair('DBEE', "Decal Blend Emission"));
		propertyNames.insert(std::make_pair('DBNE', "Decal Blend Normal"));
		propertyNames.insert(std::make_pair('DBRE', "Decal Blend Roughness"));
		propertyNames.insert(std::make_pair('DBSE', "Decal Blend Specular"));
		propertyNames.insert(std::make_pair('ENAB', "Enabled"));
		propertyNames.insert(std::make_pair('FENA', "Fog Enabled"));
		propertyNames.insert(std::make_pair('FLTV', "Float Value"));
		propertyNames.insert(std::make_pair('INST', "Instance"));
		propertyNames.insert(std::make_pair('NAME', "Name"));
		propertyNames.insert(std::make_pair('OPAC', "Opacity"));
		propertyNames.insert(std::make_pair('RSTA', "Render State"));
		propertyNames.insert(std::make_pair('SSBW', "Subsurface Value"));
		propertyNames.insert(std::make_pair('SSVB', "Subsurface Blue"));
		propertyNames.insert(std::make_pair('SSVG', "Subsurface Green"));
		propertyNames.insert(std::make_pair('SSVR', "Subsurface Red"));
		propertyNames.insert(std::make_pair('TAGS', "Tags"));
		propertyNames.insert(std::make_pair('TEXT', "Texture"));
		propertyNames.insert(std::make_pair('TILU', "Tiling U"));
		propertyNames.insert(std::make_pair('TILV', "Tiling V"));
		propertyNames.insert(std::make_pair('TXID', "Texture Id"));
		propertyNames.insert(std::make_pair('TYPE', "Type"));
		propertyNames.insert(std::make_pair('VALU', "Value"));
		propertyNames.insert(std::make_pair('ZBIA', "Z Bias"));
		propertyNames.insert(std::make_pair('ZOFF', "Z Offset"));
	}

	writer.String(propertyNames[property.propertyInfo.lName].c_str());

	PROPERTY_TYPE propertyType = static_cast<PROPERTY_TYPE>(property.propertyInfo.lType);

	switch (propertyType)
	{
		case PROPERTY_TYPE::PT_FLOAT:
		{
			writer.Double(property.floatValue);

			break;
		}
		case PROPERTY_TYPE::PT_CHAR:
		{
			writer.String(property.stringValue.c_str());

			break;
		}
		case PROPERTY_TYPE::PT_UINT32:
		{
			writer.Uint(property.uint32Value);

			break;
		}
		case PROPERTY_TYPE::PT_LIST:
		{
			writer.StartArray();
			writer.StartObject();

			for (unsigned int i = 0; i < property.childProperties.size(); ++i)
			{
				SerializeProperty(property.childProperties[i], writer);
			}

			writer.EndObject();
			writer.EndArray();

			break;
		}
	}
}

void RenderMaterialInstance::ReadProperty(Property& property, BinaryReader& binaryReader, const unsigned int propertyOffset)
{
	binaryReader.Seek(propertyOffset, SeekOrigin::Begin);

	property.propertyInfo = binaryReader.Read<SProperty>();

	const PROPERTY_TYPE propertyType = static_cast<PROPERTY_TYPE>(property.propertyInfo.lType);

	switch (propertyType)
	{
		case PROPERTY_TYPE::PT_FLOAT:
		{
			if (property.propertyInfo.lSize == 1)
			{
				property.floatValue = *reinterpret_cast<float*>(&property.propertyInfo.lData);
			}
			else
			{
				binaryReader.Seek(property.propertyInfo.lData, SeekOrigin::Begin);

				property.floatValue = binaryReader.Read<float>();
			}

			break;
		}
		case PROPERTY_TYPE::PT_CHAR:
		{
			binaryReader.Seek(property.propertyInfo.lData, SeekOrigin::Begin);

			property.stringValue = binaryReader.ReadString();

			break;
		}
		case PROPERTY_TYPE::PT_UINT32:
		{
			if (property.propertyInfo.lSize == 1)
			{
				property.uint32Value = property.propertyInfo.lData;
			}
			else
			{
				binaryReader.Seek(property.propertyInfo.lData, SeekOrigin::Begin);

				property.uint32Value = binaryReader.Read<unsigned int>();
			}

			break;
		}
		case PROPERTY_TYPE::PT_LIST:
		{
			unsigned int childPropertyOffset = property.propertyInfo.lData;

			property.childProperties.reserve(property.propertyInfo.lSize);

			for (unsigned int i = 0; i < property.propertyInfo.lSize; ++i)
			{
				Property childProperty;

				ReadProperty(childProperty, binaryReader, childPropertyOffset);

				property.childProperties.push_back(childProperty);

				childPropertyOffset += sizeof(SProperty);
			}

			break;
		}
	}
}

void RenderMaterialInstance::GetTextures(Resource* matiResource, std::vector<RenderMaterialInstance::Texture>& textures)
{
	bool foundNormalTexture = false;
	bool foundDiffuseTexture = false;
	bool foundSpecularTexture = false;

	GetTextures(instanceProperty, matiResource, textures, foundNormalTexture, foundDiffuseTexture, foundSpecularTexture);

	std::vector<Resource*> matiReferences;
	Texture texture{};

	matiResource->GetReferences(matiReferences);

	if (!foundNormalTexture || !foundDiffuseTexture || !foundSpecularTexture)
	{
		for (size_t i = 0; i < matiReferences.size(); ++i)
		{
			Resource* reference = matiReferences[i];

			if (reference->GetResourceHeaderHeader().m_type == 'TEXT')
			{
				std::string resourceID = reference->GetResourceID();

				if (!foundNormalTexture && resourceID.contains("/normal"))
				{
					texture.type = Texture::Type::Normal;
					texture.textureReferenceIndex = i;
					foundNormalTexture = true;
				}
				else if (!foundDiffuseTexture && resourceID.contains("/diffuse"))
				{
					texture.type = Texture::Type::Diffuse;
					texture.textureReferenceIndex = i;
					foundDiffuseTexture = true;
				}
				else if (!foundDiffuseTexture && resourceID.contains("/specular"))
				{
					texture.type = Texture::Type::Specular;
					texture.textureReferenceIndex = i;
					foundSpecularTexture = true;
				}
			}
		}

		if (texture.type != Texture::Type::Unknown)
		{
			textures.push_back(texture);
		}
	}
}

void RenderMaterialInstance::GetTextures(const Property& property, Resource* matiResource, std::vector<Texture>& textures, bool& foundNormalTexture, bool& foundDiffuseTexture, bool& foundSpecularTexture)
{
	bool isTextureProperty = false;

	if (property.propertyInfo.lName == 'TEXT')
	{
		isTextureProperty = true;
	}

	Texture texture{};

	for (size_t i = 0; i < property.childProperties.size(); ++i)
	{
		if (isTextureProperty)
		{
			if (property.childProperties[i].propertyInfo.lName == 'NAME' &&
				property.childProperties[i].stringValue == "mapTextureNormal_01")
			{
				texture.type = Texture::Type::Normal;
				foundNormalTexture = true;
			}
			else if (property.childProperties[i].propertyInfo.lName == 'NAME' &&
				property.childProperties[i].stringValue == "mapDiffuse_01")
			{
				texture.type = Texture::Type::Diffuse;
				foundDiffuseTexture = true;
			}
			else if (property.childProperties[i].propertyInfo.lName == 'NAME' &&
				property.childProperties[i].stringValue == "mapSpecular_01")
			{
				texture.type = Texture::Type::Specular;
				foundSpecularTexture = true;
			}

			if (property.childProperties[i].propertyInfo.lName == 'TXID')
			{
				if (property.childProperties[i].uint32Value != -1)
				{
					texture.textureReferenceIndex = property.childProperties[i].uint32Value;
				}
				else
				{
					if (texture.type == Texture::Type::Normal)
					{
						foundNormalTexture = false;
					}
					else if (texture.type == Texture::Type::Diffuse)
					{
						foundDiffuseTexture = false;
					}
				}
			}
		}

		GetTextures(property.childProperties[i], matiResource, textures, foundNormalTexture, foundDiffuseTexture, foundSpecularTexture);
	}

	if (texture.type != Texture::Type::Unknown && texture.textureReferenceIndex != -1)
	{
		textures.push_back(texture);
	}
}
