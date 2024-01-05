#include "Resources/EntityTemplate.h"
#include "Global.h"
#include "Utility/ResourceUtility.h"
#include "Registry/PropertyRegistry.h"

EntityTemplate::EntityTemplate(Resource& resource) : resource(resource)
{
}

void EntityTemplate::Deserialize()
{
	static unsigned int dataSectionOffset = 0x10;
	BinaryReader binaryReader = BinaryReader(resource.GetResourceData(), resource.GetResourceDataSize());

	binaryReader.Seek(dataSectionOffset + 4);

	rootEntityIndex = binaryReader.Read<unsigned int>();

	unsigned int entityTemplatesStartOffset = binaryReader.GetPosition();
	entityTemplatesStartOffset += binaryReader.Read<unsigned int>();
	unsigned int entityTemplatesEndOffset = binaryReader.GetPosition();
	entityTemplatesEndOffset += binaryReader.Read<unsigned int>();
	unsigned int entityTemplateCount = ResourceUtility::CalculateArrayElementsCount(entityTemplatesStartOffset, entityTemplatesEndOffset, 0x20); //0x20 is size of STemplateSubEntityBlueprint

	//Offset of data section + 0x14 (size of STemplateEntity) + 4 (skip count of array elements which is stored before element) + 4 (skip parent index)
	binaryReader.Seek(dataSectionOffset + 0x14 + 0x4 + 0x4);

	entityTemplates.resize(entityTemplateCount);

	for (unsigned int i = 0; i < entityTemplateCount; i++)
	{
		entityTemplates[i].entityTypeResourceIndex = binaryReader.Read<int>();

		binaryReader.Seek(0x1C, SeekOrigin::Current);
	}

	for (unsigned int i = 0; i < entityTemplateCount; i++)
	{
		TemplateSubEntity& templateSubEntity = entityTemplates[i];
		unsigned int entityTemplateOffset = entityTemplatesStartOffset + 0x20 * i; //0x20 is size of STemplateSubEntity

		binaryReader.Seek(entityTemplateOffset + 8);

		int propertyValuesStartOffset = binaryReader.GetPosition();
		propertyValuesStartOffset += binaryReader.Read<int>();
		int propertyValuesEndOffset = binaryReader.GetPosition();
		propertyValuesEndOffset += binaryReader.Read<int>();
		unsigned int propertyValueCount = ResourceUtility::CalculateArrayElementsCount(propertyValuesStartOffset, propertyValuesEndOffset, 0xC); //0xC is size of SEntityTemplateProperty

		binaryReader.Skip(4);

		unsigned int postInitPropertyValuesStartOffset = binaryReader.GetPosition();
		postInitPropertyValuesStartOffset += binaryReader.Read<unsigned int>();
		unsigned int postInitPropertyValuesEndOffset = binaryReader.GetPosition();
		postInitPropertyValuesEndOffset += binaryReader.Read<unsigned int>();
		unsigned int postInitPropertyValueCount = ResourceUtility::CalculateArrayElementsCount(postInitPropertyValuesStartOffset, postInitPropertyValuesEndOffset, 0xC); //0xC is size of SEntityTemplateProperty

		binaryReader.Seek(propertyValuesStartOffset);

		for (unsigned int j = 0; j < propertyValueCount; ++j)
		{
			unsigned int propertyID = binaryReader.Read<unsigned int>();
			unsigned int typeIDIndex = binaryReader.Read<unsigned int>();
			unsigned int dataOffset = binaryReader.GetPosition();
			dataOffset += binaryReader.Read<unsigned int>();
			std::string propertyName = PropertyRegistry::GetInstance().GetPropertyName(propertyID);

			if (templateSubEntity.properties.contains(propertyName))
			{
				throw std::invalid_argument("Property already exists!");
			}

			templateSubEntity.properties.insert(std::make_pair(propertyName, dataOffset));
		}

		if (postInitPropertyValueCount > 0)
		{
			binaryReader.Seek(postInitPropertyValuesStartOffset);

			for (unsigned int j = 0; j < postInitPropertyValueCount; ++j)
			{
				unsigned int propertyID = binaryReader.Read<unsigned int>();
				unsigned int typeIDIndex = binaryReader.Read<unsigned int>();
				unsigned int dataOffset = binaryReader.GetPosition();
				dataOffset += binaryReader.Read<unsigned int>();
				std::string propertyName = PropertyRegistry::GetInstance().GetPropertyName(propertyID);

				if (templateSubEntity.postInitProperties.contains(propertyName))
				{
					throw std::invalid_argument("Property already exists!");
				}

				templateSubEntity.postInitProperties.insert(std::make_pair(propertyName, dataOffset));
			}
		}
	}
}

const Resource& EntityTemplate::GetResource() const
{
	return resource;
}

Resource& EntityTemplate::GetResource()
{
	return resource;
}

const int EntityTemplate::GetRootEntityIndex() const
{
	return rootEntityIndex;
}

const std::vector<EntityTemplate::TemplateSubEntity>& EntityTemplate::GetEntityTemplates() const
{
	return entityTemplates;
}

const unsigned long long EntityTemplate::GetEntityTemplateBlueprintRuntimeResourceID() const
{
	std::vector<Resource*> references;

	resource.GetReferences(references);

	return references[references.size() - 1]->GetRuntimeResourceID();
}
