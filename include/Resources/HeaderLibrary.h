#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "Glacier/Resource/SResourceHeaderHeader.h"

#include "Resource.h"
#include "IO/BinaryReader.h"

class HeaderLibrary
{
public:
	void ParseHeaderLibrary(const std::string& runtimeFolderPath, std::string& resourceID);
	void ParseHeaderLibraryChunk(BinaryReader& headerLibraryBinaryReader, const std::string& runtimeFolderPath, std::string& chunkResourceID, const unsigned int chunkOffset, const int index);
	Resource& GetResource(const unsigned long long runtimeResourceID);

private:
	std::unordered_map<unsigned long long, Resource> resources;
};
