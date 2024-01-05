#pragma once

#include <unordered_map>
#include <vector>

#include "Resources/Resource.h"

class EntityTemplate
{
public:
	struct TemplateSubEntity
	{
		int entityTypeResourceIndex;
		std::unordered_map<std::string, unsigned int> properties;
		std::unordered_map<std::string, unsigned int> postInitProperties;
	};

	EntityTemplate(Resource& resource);
	void Deserialize();
	const Resource& GetResource() const;
	Resource& GetResource();
	const int GetRootEntityIndex() const;
	const std::vector<TemplateSubEntity>& GetEntityTemplates() const;
	const unsigned long long GetEntityTemplateBlueprintRuntimeResourceID() const;

private:
	Resource& resource;
	int rootEntityIndex;
	std::vector<TemplateSubEntity> entityTemplates;
};
