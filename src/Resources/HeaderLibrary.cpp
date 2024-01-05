#include <vector>

#include "Resources/HeaderLibrary.h"
#include "Utility/ResourceUtility.h"
#include "Glacier/Resource/ZResourceHeaderReader.h"
#include "Registry/ResourceIDRegistry.h"

void HeaderLibrary::ParseHeaderLibrary(const std::string& runtimeFolderPath, std::string& resourceID)
{
	std::string filePath = ResourceUtility::ConvertResourceIDToFilePath(runtimeFolderPath, resourceID);

	BinaryReader binaryReader = BinaryReader(filePath);
	SResourceHeaderHeader resourceHeaderHeader = binaryReader.Read<SResourceHeaderHeader>();

	binaryReader.Skip(resourceHeaderHeader.m_nReferencesChunkSize);
	binaryReader.Skip(0x30); //Size of BIN2 header + size of SHeaderLibrary

	unsigned int headerLibraryChunkCount = binaryReader.Read<unsigned int>();

	for (unsigned int i = 0; i < headerLibraryChunkCount; ++i)
	{
		const unsigned int chunkOffset = binaryReader.GetPosition();

		binaryReader.Skip(4);

		unsigned int resourceIDOffset = static_cast<unsigned int>(binaryReader.GetPosition());
		resourceIDOffset += binaryReader.Read<unsigned int>();
		size_t currentPosition = binaryReader.GetPosition();

		binaryReader.Seek(resourceIDOffset - 4, SeekOrigin::Begin);

		unsigned int resourceIDLength = binaryReader.Read<unsigned int>();
		std::string chunkResourceID = binaryReader.ReadString((size_t)resourceIDLength - 1);

		ParseHeaderLibraryChunk(binaryReader, runtimeFolderPath, chunkResourceID, chunkOffset, i);

		binaryReader.Seek(currentPosition, SeekOrigin::Begin);
		binaryReader.Seek(0x48, SeekOrigin::Current);
	}
}

void HeaderLibrary::ParseHeaderLibraryChunk(BinaryReader& headerLibraryBinaryReader, const std::string& runtimeFolderPath, std::string& chunkResourceID, const unsigned int chunkOffset, const int index)
{
	std::string resourceLibraryFilePath = ResourceUtility::ConvertResourceIDToFilePath(runtimeFolderPath, chunkResourceID);

	headerLibraryBinaryReader.Seek(chunkOffset + 0x44);

	unsigned int ridMappingIDsStartOffset = static_cast<unsigned int>(headerLibraryBinaryReader.GetPosition());
	ridMappingIDsStartOffset += headerLibraryBinaryReader.Read<unsigned int>();

	headerLibraryBinaryReader.Seek(ridMappingIDsStartOffset - 4);

	unsigned int ridMappingIDsCount = headerLibraryBinaryReader.Read<unsigned int>();
	std::unordered_map<unsigned int, unsigned long long> resourceIndicesToRuntimeResourceIDs;

	resourceIndicesToRuntimeResourceIDs.reserve(ridMappingIDsCount);

	for (unsigned int i = 0; i < ridMappingIDsCount; ++i)
	{
		unsigned long long runtimeResourceID = headerLibraryBinaryReader.Read<unsigned long long>();

		resources.insert(std::make_pair(runtimeResourceID, Resource()));
		resourceIndicesToRuntimeResourceIDs.insert(std::make_pair(i, runtimeResourceID));
	}

	headerLibraryBinaryReader.Seek(chunkOffset + 0x2C);

	unsigned int resourceHeadersStartOffset = headerLibraryBinaryReader.GetPosition();
	resourceHeadersStartOffset += headerLibraryBinaryReader.Read<unsigned int>();

	headerLibraryBinaryReader.Seek(resourceHeadersStartOffset - 4);

	unsigned int resourceCount = headerLibraryBinaryReader.Read<unsigned int>();
	unsigned int offsetInResourceLibrary = 0x18;

	for (unsigned int i = 0; i < resourceCount; ++i)
	{
		headerLibraryBinaryReader.Seek(resourceHeadersStartOffset + i * 0xC, SeekOrigin::Begin);

		unsigned int resourceHeaderOffset = headerLibraryBinaryReader.GetPosition();
		resourceHeaderOffset += headerLibraryBinaryReader.Read<unsigned int>();

		const unsigned long long runtimeResourceID = resourceIndicesToRuntimeResourceIDs[i];
		Resource& resource = resources[runtimeResourceID];

		resource.LoadResource(headerLibraryBinaryReader, resourceHeaderOffset);

		const SResourceHeaderHeader& resourceHeaderHeader = resource.GetResourceHeaderHeader();
		unsigned int dataSize = resourceHeaderHeader.m_nDataSize;
		std::string resourceID = ResourceIDRegistry::GetInstance().GetResourceID(runtimeResourceID);

		if (resourceHeaderHeader.m_type == 'FSBM' || resourceHeaderHeader.m_type == 'FSBS')
		{
			dataSize += 24;
		}

		resource.SetOffsetInResourceLibrary(offsetInResourceLibrary);
		resource.SetResourceLibraryFilePath(resourceLibraryFilePath);
		resource.SetResourceDataSize(dataSize);
		resource.SetRuntimeResourceID(runtimeResourceID);
		resource.SetResourceID(resourceID);
		resource.SetName(ResourceUtility::GetResourceName(resourceID));

		offsetInResourceLibrary += dataSize;
	}
}

Resource& HeaderLibrary::GetResource(const unsigned long long runtimeResourceID)
{
	if (ZRuntimeResourceID(runtimeResourceID).IsLibraryResource())
	{
		return resources[runtimeResourceID];
	}

	return globalHeaderLibrary.resources[runtimeResourceID];
}
