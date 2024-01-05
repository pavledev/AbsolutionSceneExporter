#include <fstream>
#include <iostream>
#include <format>

#include "Registry/ResourceIDRegistry.h"

ResourceIDRegistry& ResourceIDRegistry::GetInstance()
{
	static ResourceIDRegistry instance;

	return instance;
}

void ResourceIDRegistry::Load(const std::string& assetsFolderPath)
{
    std::string textFilePath = std::format("{}\\assets\\HashMap.txt", assetsFolderPath);
    std::ifstream ifstream = std::ifstream(textFilePath);

    if (!ifstream.is_open())
    {
        std::cout << "Failed to open HashMap.txt!" << std::endl;

        return;
    }

    ifstream.seekg(0, ifstream.end);

    size_t fileSize = static_cast<size_t>(ifstream.tellg());

    ifstream.seekg(0, ifstream.beg);

    std::vector<char> hashListData = std::vector<char>(fileSize, 0);
    unsigned int position = 0, lastPosition = 0;

    ifstream.read(hashListData.data(), fileSize);

    while (true)
    {
        if (hashListData.data()[position] == 0xA)
        {
            hashListData.data()[position] = 0;

            std::string line = std::string(&hashListData.data()[lastPosition]);

            unsigned long long hash = std::stoull(line.substr(0, line.find(' ')), nullptr, 16);
            std::string resourceID = line.substr(line.find(' ') + 1);

            resourceIDs.insert(std::make_pair(hash, resourceID));

            lastPosition = position + 1;
        }

        position++;

        if (position > fileSize)
        {
            break;
        }
    }

    ifstream.close();
}

std::string ResourceIDRegistry::GetResourceID(const unsigned long long runtimeResourceID) const
{
    auto it = resourceIDs.find(runtimeResourceID);

    if (it != resourceIDs.end())
    {
        return it->second;
    }

    return "";
}
