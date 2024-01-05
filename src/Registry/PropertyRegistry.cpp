#include <fstream>
#include <format>

#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"

#include "Registry/PropertyRegistry.h"

PropertyRegistry& PropertyRegistry::GetInstance()
{
	static PropertyRegistry instance;

	return instance;
}

void PropertyRegistry::Load(const std::string& assetsFolderPath)
{
	std::string jsonFilePath = std::format("{}\\assets\\Properties.json", assetsFolderPath);
	std::ifstream inputFileStream = std::ifstream(jsonFilePath);
	rapidjson::IStreamWrapper streamWrapper(inputFileStream);
	rapidjson::Document document;

	document.ParseStream(streamWrapper);

	const rapidjson::Value& properties2 = document["properties"];

	for (rapidjson::Value::ConstValueIterator it = properties2.Begin(); it != properties2.End(); ++it)
	{
		const rapidjson::Value& object = it->GetObj();
		std::string name = object["name"].GetString();
		unsigned int hash = object["hash"].GetUint();

		properties.insert(std::make_pair(hash, name));
	}
}

std::string PropertyRegistry::GetPropertyName(const unsigned int hash) const
{
	std::string result;
	auto it = properties.find(hash);

	if (it != properties.end())
	{
		result = it->second;
	}

	return result;
}
