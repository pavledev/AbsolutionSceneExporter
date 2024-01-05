#pragma once

#include "Resources/Resource.h"

class EntityTemplateBlueprint
{
public:
	EntityTemplateBlueprint(Resource* resource);
	void Deserialize();
	const Resource* GetResource() const;
	Resource* GetResource();
	const std::vector<std::string>& GetEntityNames() const;

private:
	Resource* resource;
	std::vector<std::string> entityNames;
};
