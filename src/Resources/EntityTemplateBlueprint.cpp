#include "Resources/EntityTemplateBlueprint.h"
#include "Utility/ResourceUtility.h"

EntityTemplateBlueprint::EntityTemplateBlueprint(Resource* resource) : resource(resource)
{
}

void EntityTemplateBlueprint::Deserialize()
{
	static unsigned int dataSectionOffset = 0x10;
	BinaryReader binaryReader = BinaryReader(resource->GetResourceData(), resource->GetResourceDataSize());

	binaryReader.Seek(dataSectionOffset + 4);

	unsigned int entityTemplatesStartOffset = binaryReader.GetPosition();
	entityTemplatesStartOffset += binaryReader.Read<unsigned int>();
	unsigned int entityTemplatesEndOffset = binaryReader.GetPosition();
	entityTemplatesEndOffset += binaryReader.Read<unsigned int>();
	unsigned int entityTemplateCount = ResourceUtility::CalculateArrayElementsCount(entityTemplatesStartOffset, entityTemplatesEndOffset, 0x40); //0x40 is size of STemplateSubEntityBlueprint

	/*
	* Offset of data section + 0x34 (size of STemplateEntityBlueprint) + 4 (skip count of array elements which is stored before element) +
	* 0xC (skip parentIndex, entityTypeResourceIndex and m_length)
	*/
	binaryReader.Seek(dataSectionOffset + 0x34 + 0x4 + 0xC);

	std::vector<unsigned long long> offsets = std::vector<unsigned long long>(entityTemplateCount);

	for (unsigned int i = 0; i < entityTemplateCount; i++)
	{
		offsets[i] = binaryReader.GetPosition();
		offsets[i] += binaryReader.Read<unsigned int>() - 4;

		binaryReader.Seek(0x3C, SeekOrigin::Current);
	}

	entityNames.resize(entityTemplateCount);

	for (unsigned int i = 0; i < entityTemplateCount; i++)
	{
		binaryReader.Seek(offsets[i]);

		int lenghtOfString = binaryReader.Read<int>();

		entityNames[i] = binaryReader.ReadString(static_cast<size_t>(lenghtOfString - 1));
	}
}

const Resource* EntityTemplateBlueprint::GetResource() const
{
	return resource;
}

Resource* EntityTemplateBlueprint::GetResource()
{
	return resource;
}

const std::vector<std::string>& EntityTemplateBlueprint::GetEntityNames() const
{
	return entityNames;
}
