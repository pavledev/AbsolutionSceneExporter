#include <sstream>
#include <iomanip>

#include "Glacier/Resource/ZResourceHeaderReader.h"

#include "Resources/Resource.h"
#include "Utility/ResourceUtility.h"
#include "Global.h"

Resource::Resource()
{
	resourceID = "";
	offsetInResourceLibrary = 0;
	resourceData = nullptr;
	resourceDataSize = 0;
}

Resource::~Resource()
{
	if (resourceData)
	{
		delete[] resourceData;
	}
}

const std::string Resource::GetResourceLibraryFilePath() const
{
	return resourceLibraryFilePath;
}

const std::string Resource::GetName() const
{
	return name;
}

const SResourceHeaderHeader& Resource::GetResourceHeaderHeader() const
{
	return resourceHeaderHeader;
}

const ZRuntimeResourceID& Resource::GetRuntimeResourceID() const
{
	return runtimeResourceID;
}

std::string Resource::GetResourceID() const
{
	return resourceID;
}

const unsigned int Resource::GetOffsetInResourceLibrary() const
{
	return offsetInResourceLibrary;
}

const void* Resource::GetResourceData() const
{
	return resourceData;
}

void* Resource::GetResourceData()
{
	return resourceData;
}

const unsigned int Resource::GetResourceDataSize() const
{
	return resourceDataSize;
}

void Resource::GetReferences(std::vector<Resource*>& references) const
{
	references.reserve(this->references.size());

	for (size_t i = 0; i < this->references.size(); ++i)
	{
		Resource& reference = headerLibrary.GetResource(this->references[i]);

		references.push_back(&reference);
	}
}

void Resource::GetReferences(std::vector<Resource*>& references)
{
	references.reserve(this->references.size());

	for (size_t i = 0; i < this->references.size(); ++i)
	{
		Resource& reference = headerLibrary.GetResource(this->references[i]);

		references.push_back(&reference);
	}
}

void Resource::SetResourceLibraryFilePath(std::string resourceLibraryFilePath)
{
	this->resourceLibraryFilePath = resourceLibraryFilePath;
}

void Resource::SetName(std::string name)
{
	this->name = name;
}

void Resource::SetResourceHeaderHeader(const SResourceHeaderHeader& resourceHeaderHeader)
{
	this->resourceHeaderHeader = resourceHeaderHeader;
}

void Resource::SetRuntimeResourceID(const ZRuntimeResourceID& runtimeResourceID)
{
	this->runtimeResourceID = runtimeResourceID;
}

void Resource::SetResourceID(std::string resourceID)
{
	this->resourceID = resourceID;
}

void Resource::SetResourceDataSize(const unsigned int resourceDataSize)
{
	this->resourceDataSize = resourceDataSize;
}

void Resource::SetOffsetInResourceLibrary(const unsigned int offsetInResourceLibrary)
{
	this->offsetInResourceLibrary = offsetInResourceLibrary;
}

void Resource::LoadResourceData()
{
	BinaryReader resourceLibraryBinaryReader = BinaryReader(resourceLibraryFilePath);

	resourceLibraryBinaryReader.Seek(offsetInResourceLibrary);

	resourceData = resourceLibraryBinaryReader.Read<void>(resourceDataSize);
}

void Resource::DeleteResourceData()
{
	delete[] resourceData;

	resourceData = nullptr;
}

void Resource::LoadResource(BinaryReader& headerLibraryBinaryReader, const unsigned int resourceHeaderOffset)
{
	headerLibraryBinaryReader.Seek(resourceHeaderOffset + 4, SeekOrigin::Begin);

	unsigned int referencesChunkSize = headerLibraryBinaryReader.Read<unsigned int>();
	unsigned int headerDataSize = sizeof(SResourceHeaderHeader) + referencesChunkSize;

	headerLibraryBinaryReader.Seek(resourceHeaderOffset, SeekOrigin::Begin);

	void* headerData = headerLibraryBinaryReader.Read<void>(headerDataSize);
	BinaryReader headerDataBinaryReader = BinaryReader(headerData, headerDataSize);

	resourceHeaderHeader = headerDataBinaryReader.Read<SResourceHeaderHeader>();

	if (resourceHeaderHeader.m_nReferencesChunkSize > 0)
	{
		LoadReferences(headerDataBinaryReader);
	}

	delete[] headerData;
}

void Resource::LoadReferences(const BinaryReader& headerDataBinaryReader)
{
	unsigned char* referencesChunk = headerDataBinaryReader.Read<unsigned char>(resourceHeaderHeader.m_nReferencesChunkSize);
	const ZResourceHeaderReader resourceHeaderReader = ZResourceHeaderReader(&resourceHeaderHeader, referencesChunk);
	const unsigned int numberOfReferences = resourceHeaderReader.GetNumResourceIdentifiers();

	for (unsigned int i = 0; i < numberOfReferences; ++i)
	{
		const ZRuntimeResourceID runtimeResourceID = resourceHeaderReader.GetResourceIdentifier(i);

		references.push_back(runtimeResourceID.GetID());
	}

	delete[] referencesChunk;
}
