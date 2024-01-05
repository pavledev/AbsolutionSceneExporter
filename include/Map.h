#pragma once

#include <string>

#include "Glacier/Math/SMatrix43.h"

#include "Resources/HeaderLibrary.h"
#include "Resources/RenderPrimitive.h"
#include "Resources/EntityTemplate.h"
#include "Resources/EntityTemplateBlueprint.h"

class Map
{
public:
	struct Node
	{
		unsigned int index = -1;
		unsigned int parentIndex = -1;
		unsigned int entityIndex = -1;
		bool isVolumeBox = false;
		bool isVolumeSphere = false;
		bool hasPrimRuntimeResourceID = false;
		unsigned long long primRuntimeResourceID = -1;
		bool hasTransform = false;
		SMatrix43 transform;
		bool hasGlobalSize = false;
		SVector3 globalSize;
		bool hasEIDParent = false;
		unsigned int eIDParent;
		bool hasVisible = false;
		bool isVisible = false;
		bool hasGeomEntity = false;
		unsigned int geomEntityIndex = -1;
		std::string name;
		std::vector<Node*> children;
		unsigned long long tempRuntimeResourceID;
		std::unordered_map<unsigned long long, unsigned int> tempChildren;
	};

	~Map();
	void ExportMap(const unsigned long long sceneTempRuntimeResourceID, const std::string& outputFolderPath);
	void AddMapNodes(const unsigned long long tempRuntimeResourceID, Node* parentMapNode = nullptr);
	void LoadEntityTemplatesAndEntityTemplateBlueprints(Resource& tempResource);
	void GenerateJson(const unsigned long long sceneTempRuntimeResourceID, const std::string& outputFolderPath);
	static unsigned long long FindPRIMRuntimeResourceID(Resource& tempResource, const std::vector<EntityTemplate::TemplateSubEntity>& entityTemplates, const std::unordered_map<std::string, unsigned int>& properties);
	static void FindProperties(Node& mapNode, Resource& tempResource, const std::vector<EntityTemplate::TemplateSubEntity>& entityTemplates, const std::unordered_map<std::string, unsigned int>& properties);
	static void ScaleTransform(SMatrix43& transform, const SVector3& scale);

private:
	std::unordered_map<unsigned long long, EntityTemplate*> entityTemplates;
	std::unordered_map<unsigned long long, EntityTemplateBlueprint*> entityTemplateBlueprints;
	unsigned int entityCount = 0;
	std::vector<Node*> nodes;
	std::unordered_map<unsigned long long, std::unordered_map<unsigned int, std::vector<unsigned int>>> entityIDParentIndexToMapNodeIndices;
	std::unordered_map<unsigned long long, std::unordered_map<unsigned int, unsigned int>> entityIDParentIndexToMapNodeIndex;
};
