#pragma once

#include <string>
#include <unordered_map>

class PropertyRegistry
{
public:
	static PropertyRegistry& GetInstance();
	void Load(const std::string& assetsFolderPath);
	std::string GetPropertyName(const unsigned int hash) const;

private:
	PropertyRegistry() = default;
	PropertyRegistry(const PropertyRegistry& other) = delete;
	PropertyRegistry& operator=(const PropertyRegistry& other) = delete;

	std::unordered_map<unsigned int, std::string> properties;
};
