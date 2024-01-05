#pragma once

#include <string>
#include <unordered_map>

class ResourceIDRegistry
{
public:
	static ResourceIDRegistry& GetInstance();
	void Load(const std::string& assetsFolderPath);
	std::string GetResourceID(const unsigned long long runtimeResourceID) const;

private:
	ResourceIDRegistry() = default;
	ResourceIDRegistry(const ResourceIDRegistry& other) = delete;
	ResourceIDRegistry& operator=(const ResourceIDRegistry& other) = delete;

	std::unordered_map<unsigned long long, std::string> resourceIDs;
};
