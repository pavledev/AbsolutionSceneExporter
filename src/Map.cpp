#include <iostream>

#include "Map.h"
#include "Registry/PropertyRegistry.h"
#include "Global.h"
#include "Utility/StringUtility.h"
#include "Utility/ResourceUtility.h"

Map::~Map()
{
	for (auto it = entityTemplates.begin(); it != entityTemplates.end(); ++it)
	{
		it->second->GetResource().DeleteResourceData();

		delete it->second;
	}

	for (auto it = entityTemplateBlueprints.begin(); it != entityTemplateBlueprints.end(); ++it)
	{
		it->second->GetResource()->DeleteResourceData();

		delete it->second;
	}

	for (unsigned int i = 0; i < nodes.size(); ++i)
	{
		delete nodes[i];
	}

	entityTemplates.clear();
	entityTemplateBlueprints.clear();
	nodes.clear();
}

void Map::ExportMap(const unsigned long long sceneTempRuntimeResourceID, const std::string& outputFolderPath)
{
	Resource& sceneTempResource = headerLibrary.GetResource(sceneTempRuntimeResourceID);
	std::vector<Resource*> references;

	sceneTempResource.GetReferences(references);
	LoadEntityTemplatesAndEntityTemplateBlueprints(sceneTempResource);

	nodes.reserve(entityCount);

	AddMapNodes(sceneTempRuntimeResourceID);
	GenerateJson(sceneTempRuntimeResourceID, outputFolderPath);
}

void Map::AddMapNodes(const unsigned long long tempRuntimeResourceID, Node* parentMapNode)
{
	EntityTemplate* entityTemplate = entityTemplates[tempRuntimeResourceID];
	EntityTemplateBlueprint* entityTemplateBlueprint = entityTemplateBlueprints[entityTemplate->GetEntityTemplateBlueprintRuntimeResourceID()];
	Resource& tempResource = entityTemplate->GetResource();
	std::vector<Resource*> references;

	entityTemplate->GetResource().GetReferences(references);

	const std::vector<EntityTemplate::TemplateSubEntity>& entityTemplates = entityTemplate->GetEntityTemplates();
	const std::vector<std::string>& entityNames = entityTemplateBlueprint->GetEntityNames();
	bool hasGeomEntity = false;
	bool hasPrimRuntimeResourceID = false;
	std::unordered_map<unsigned int, unsigned int> entityIndicesToNodeIndices;
	std::set<unsigned int> eidParents;

	for (unsigned int i = 0; i < entityTemplates.size(); ++i)
	{
		const EntityTemplate::TemplateSubEntity& templateSubEntity = entityTemplates[i];
		const unsigned long long runtimeResourceID = references[templateSubEntity.entityTypeResourceIndex]->GetRuntimeResourceID().GetID();
		Node* childMapNode = new Node();

		childMapNode->index = nodes.size();
		childMapNode->entityIndex = i;
		childMapNode->name = entityNames[i];
		childMapNode->tempRuntimeResourceID = tempRuntimeResourceID;

		//0x8000001CDE626D76 is runtime resource id for volume box only in scenes
		//0x800005CEA45DBE71 is runtime resource id for volume sphere only in scenes
		if (runtimeResourceID == 0x8000001CDE626D76)
		{
			childMapNode->isVolumeBox = true;
		}
		else if (runtimeResourceID == 0x800005CEA45DBE71)
		{
			childMapNode->isVolumeSphere = true;
		}

		std::string entityName = StringUtility::ToLower(childMapNode->name);

		if (entityName.contains("collision") || entityName.contains("volume"))
		{
			if (entityName.contains("box") || entityName.contains("sphere"))
			{
				childMapNode->isVolumeBox = true;
			}
			else if (entityName.contains("sphere"))
			{
				childMapNode->isVolumeSphere = true;
			}
		}

		FindProperties(*childMapNode, tempResource, entityTemplates, templateSubEntity.properties);
		FindProperties(*childMapNode, tempResource, entityTemplates, templateSubEntity.postInitProperties);

		if (parentMapNode)
		{
			if (i == entityTemplate->GetRootEntityIndex())
			{
				entityIndicesToNodeIndices.insert(std::make_pair(i, parentMapNode->index));
			}
			else
			{
				childMapNode->parentIndex = parentMapNode->index;

				//if (childMapNode->hasPrimRuntimeResourceID)
				//{
				//	childMapNode->parentIndex = parentMapNode->index;
				//}
				//else
				//{
				//	//Assign index of Scene entity in this case
				//}

				nodes.push_back(childMapNode);
				entityIndicesToNodeIndices.insert(std::make_pair(i, childMapNode->index));
			}
		}
		else
		{
			nodes.push_back(childMapNode);
			entityIndicesToNodeIndices.insert(std::make_pair(i, childMapNode->index));
		}

		Resource& resource = headerLibrary.GetResource(runtimeResourceID);

		if (resource.GetResourceHeaderHeader().m_type == 'ASET')
		{
			std::vector<Resource*> references;

			resource.GetReferences(references);

			for (unsigned j = 0; j < references.size(); ++j)
			{
				if (references[j]->GetResourceHeaderHeader().m_type != 'TEMP')
				{
					continue;
				}

				AddMapNodes(references[j]->GetRuntimeResourceID(), childMapNode);
			}
		}
		else if (resource.GetResourceHeaderHeader().m_type == 'TEMP')
		{
			AddMapNodes(resource.GetRuntimeResourceID(), childMapNode);
		}

		if (childMapNode->hasEIDParent)
		{
			entityIDParentIndexToMapNodeIndices[tempRuntimeResourceID][childMapNode->eIDParent].push_back(childMapNode->index);
			eidParents.insert(childMapNode->eIDParent);
		}

		if (parentMapNode && i == entityTemplate->GetRootEntityIndex()) //Root entity of TEMP resource referenced by entity is assigned to that entity
		{
			if (!parentMapNode->hasPrimRuntimeResourceID)
			{
				parentMapNode->hasPrimRuntimeResourceID = childMapNode->hasPrimRuntimeResourceID;
				parentMapNode->primRuntimeResourceID = childMapNode->primRuntimeResourceID;
			}

			if (!parentMapNode->hasTransform)
			{
				parentMapNode->hasTransform = childMapNode->hasTransform;
				parentMapNode->transform = childMapNode->transform;
			}

			if (!parentMapNode->hasGlobalSize)
			{
				parentMapNode->hasGlobalSize = childMapNode->hasGlobalSize;
				parentMapNode->globalSize = childMapNode->globalSize;
			}

			if (!parentMapNode->hasEIDParent)
			{
				parentMapNode->hasEIDParent = childMapNode->hasEIDParent;
				parentMapNode->eIDParent = childMapNode->eIDParent;
			}

			if (!parentMapNode->hasVisible)
			{
				parentMapNode->hasVisible = childMapNode->hasVisible;
				parentMapNode->isVisible = childMapNode->isVisible;
			}

			parentMapNode->tempChildren.insert(std::make_pair(tempRuntimeResourceID, i));
		}

		if (childMapNode->hasGeomEntity)
		{
			hasGeomEntity = true;
		}

		if (childMapNode->hasPrimRuntimeResourceID)
		{
			hasPrimRuntimeResourceID = true;
		}
	}

	for (auto it = entityIndicesToNodeIndices.begin(); it != entityIndicesToNodeIndices.end(); ++it)
	{
		if (eidParents.contains(it->first))
		{
			entityIDParentIndexToMapNodeIndex[tempRuntimeResourceID][it->first] = it->second;
		}
	}

	if (hasGeomEntity && !hasPrimRuntimeResourceID)
	{
		bool hasResourceID = false;
		bool hasGizmoGeomRID = false;

		for (unsigned int i = 0; i < entityTemplates.size(); ++i)
		{
			const EntityTemplate::TemplateSubEntity& templateSubEntity = entityTemplates[i];

			auto iterator = templateSubEntity.properties.find("m_ResourceID");

			if (iterator != templateSubEntity.properties.end())
			{
				const unsigned long long runtimeResourceID = ResourceUtility::GetRuntimeResourceID(tempResource, iterator->second);

				if (runtimeResourceID != -1)
				{
					hasResourceID = true;

					break;
				}
			}

			iterator = templateSubEntity.postInitProperties.find("m_ResourceID");

			if (iterator != templateSubEntity.postInitProperties.end())
			{
				const unsigned long long runtimeResourceID = ResourceUtility::GetRuntimeResourceID(tempResource, iterator->second);

				if (runtimeResourceID != -1)
				{
					hasResourceID = true;

					break;
				}
			}

			iterator = templateSubEntity.properties.find("m_GizmoGeomRID");

			if (iterator != templateSubEntity.properties.end())
			{
				const unsigned long long runtimeResourceID = ResourceUtility::GetRuntimeResourceID(tempResource, iterator->second);

				if (runtimeResourceID != -1)
				{
					hasGizmoGeomRID = true;

					break;
				}
			}

			iterator = templateSubEntity.postInitProperties.find("m_GizmoGeomRID");

			if (iterator != templateSubEntity.postInitProperties.end())
			{
				const unsigned long long runtimeResourceID = ResourceUtility::GetRuntimeResourceID(tempResource, iterator->second);

				if (runtimeResourceID != -1)
				{
					hasGizmoGeomRID = true;

					break;
				}
			}
		}

		if (hasResourceID || hasGizmoGeomRID)
		{
			throw std::invalid_argument("Has Geom Entity but doesn't have PRIM Runtime Resource ID!");
		}
	}
}

void Map::LoadEntityTemplatesAndEntityTemplateBlueprints(Resource& tempResource)
{
	if (entityTemplates.contains(tempResource.GetRuntimeResourceID()))
	{
		return;
	}

	EntityTemplate* entityTemplate = new EntityTemplate(tempResource);
	std::vector<Resource*> references;

	tempResource.GetReferences(references);
	tempResource.LoadResourceData();
	entityTemplate->Deserialize();

	entityCount += entityTemplate->GetEntityTemplates().size();

	entityTemplates.insert(std::make_pair(tempResource.GetRuntimeResourceID(), entityTemplate));

	Resource* tbluResource = references[references.size() - 1];

	if (!entityTemplateBlueprints.contains(tbluResource->GetRuntimeResourceID()))
	{
		EntityTemplateBlueprint* entityTemplateBlueprint = new EntityTemplateBlueprint(tbluResource);

		tbluResource->LoadResourceData();
		entityTemplateBlueprint->Deserialize();

		entityTemplateBlueprints.insert(std::make_pair(tbluResource->GetRuntimeResourceID(), entityTemplateBlueprint));
	}

	for (size_t i = 0; i < references.size(); ++i)
	{
		unsigned int resourceType = references[i]->GetResourceHeaderHeader().m_type;

		if (resourceType == 'TEMP')
		{
			LoadEntityTemplatesAndEntityTemplateBlueprints(*references[i]);
		}
		else if (resourceType == 'ASET')
		{
			std::vector<Resource*> references2;

			references[i]->GetReferences(references2);

			for (size_t j = 0; j < references2.size(); ++j)
			{
				resourceType = references2[j]->GetResourceHeaderHeader().m_type;

				if (resourceType == 'TEMP')
				{
					LoadEntityTemplatesAndEntityTemplateBlueprints(*references2[j]);
				}
			}
		}
	}
}

void Map::GenerateJson(const unsigned long long sceneTempRuntimeResourceID, const std::string& outputFolderPath)
{
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sceneJsonBuffer);
	unsigned int rootEntityIndex = -1;

	writer.StartObject();

	writer.String("Scene");
	writer.StartArray();

	unsigned int nodeIndex = 0;

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		Node& node = *nodes[i];

		if (rootEntityIndex == -1 &&
			node.tempRuntimeResourceID == sceneTempRuntimeResourceID &&
			node.name == "Scene")
		{
			rootEntityIndex = node.index;
		}

		writer.StartObject();

		writer.String("Index");
		writer.Uint(node.index);

		writer.String("Name");
		writer.String(node.name.c_str());

		if (node.hasEIDParent)
		{
			unsigned int parentNodeIndex = entityIDParentIndexToMapNodeIndex[node.tempRuntimeResourceID][node.eIDParent];

			writer.String("Parent");
			writer.String(nodes[parentNodeIndex]->name.c_str());

			writer.String("Parent Index");
			writer.Uint(parentNodeIndex);
		}
		else if (node.parentIndex != -1)
		{
			writer.String("Parent");
			writer.String(nodes[node.parentIndex]->name.c_str());

			writer.String("Parent Index");
			writer.Uint(node.parentIndex);
		}
		else
		{
			writer.String("Parent");
			writer.Null();

			writer.String("Parent Index");
			writer.Null();
		}

		writer.String("Children");
		writer.StartArray();

		if (entityIDParentIndexToMapNodeIndices.contains(node.tempRuntimeResourceID))
		{
			if (entityIDParentIndexToMapNodeIndices[node.tempRuntimeResourceID].contains(node.entityIndex))
			{
				const std::vector<unsigned int>& mapNodeIndices = entityIDParentIndexToMapNodeIndices[node.tempRuntimeResourceID][node.entityIndex];

				for (size_t j = 0; j < mapNodeIndices.size(); ++j)
				{
					const unsigned int mappingNodeIndex = mapNodeIndices[j];

					writer.StartObject();

					writer.String("Name");
					writer.String(nodes[mappingNodeIndex]->name.c_str());

					writer.String("Index");
					writer.Uint(nodes[mappingNodeIndex]->index);

					writer.EndObject();
				}
			}
		}

		for (auto it = node.tempChildren.begin(); it != node.tempChildren.end(); ++it)
		{
			const std::vector<unsigned int>& mapNodeIndices = entityIDParentIndexToMapNodeIndices[it->first][it->second];

			for (size_t j = 0; j < mapNodeIndices.size(); ++j)
			{
				const unsigned int mappingNodeIndex = mapNodeIndices[j];

				writer.StartObject();

				writer.String("Name");
				writer.String(nodes[mappingNodeIndex]->name.c_str());

				writer.String("Index");
				writer.Uint(nodes[mappingNodeIndex]->index);

				writer.EndObject();
			}
		}

		writer.EndArray();

		if (node.hasGlobalSize)
		{
			ScaleTransform(node.transform, node.globalSize);
		}

		writer.String("Transform");
		node.transform.SerializeToJson(writer);

		writer.String("IsVisible");
		writer.Bool(node.isVisible);

		writer.String("IsVolumeBox");
		writer.Bool(node.isVolumeBox);

		writer.String("IsVolumeSphere");
		writer.Bool(node.isVolumeSphere);

		writer.String("PRIMRuntimeResourceID");
		writer.Uint64(node.primRuntimeResourceID);

		if (node.hasPrimRuntimeResourceID)
		{
			Resource& primResource = headerLibrary.GetResource(node.primRuntimeResourceID);
			RenderPrimitive renderPrimitive = RenderPrimitive(primResource);

			primResource.LoadResourceData();
			renderPrimitive.Deserialize();

			writer.String("Meshes");
			writer.StartArray();

			const std::vector<RenderPrimitive::Mesh*>& meshes = renderPrimitive.GetMeshes();
			std::vector<Resource*> references;

			primResource.GetReferences(references);

			for (size_t j = 0; j < meshes.size(); ++j)
			{
				writer.StartObject();

				writer.String("MATIRuntimeResourceID");
				writer.Uint64(references[meshes[j]->GetMaterialID()]->GetRuntimeResourceID());

				writer.EndObject();
			}

			writer.EndArray();

			writer.String("PRIMFileName");
			writer.String(primResource.GetName().c_str());

			primResource.DeleteResourceData();

			primRuntimeResourceIDs.insert(node.primRuntimeResourceID);
		}

		writer.EndObject();
	}

	writer.EndArray();

	writer.String("RootEntityIndex");
	writer.Uint(rootEntityIndex);

	writer.EndObject();

	std::string jsonFilePath = std::format("{}\\scene.json", outputFolderPath);
	std::ofstream jsonFile = std::ofstream(jsonFilePath);

	jsonFile << sceneJsonBuffer.GetString();

	jsonFile.close();
}

unsigned long long Map::FindPRIMRuntimeResourceID(Resource& tempResource, const std::vector<EntityTemplate::TemplateSubEntity>& entityTemplates, const std::unordered_map<std::string, unsigned int>& properties)
{
	bool hasPrimRuntimeResourceID = false;
	unsigned long long primRuntimeResourceID = -1;
	auto iterator = properties.find("m_ResourceID");

	if (iterator != properties.end())
	{
		const unsigned long long runtimeResourceID2 = ResourceUtility::GetRuntimeResourceID(tempResource, iterator->second);
		const Resource& resource = headerLibrary.GetResource(runtimeResourceID2);

		if (resource.GetResourceHeaderHeader().m_type == 'PRIM')
		{
			hasPrimRuntimeResourceID = true;
			primRuntimeResourceID = runtimeResourceID2;
		}
	}

	if (!hasPrimRuntimeResourceID)
	{
		iterator = properties.find("m_GizmoGeomRID");

		if (iterator != properties.end())
		{
			const unsigned long long runtimeResourceID2 = ResourceUtility::GetRuntimeResourceID(tempResource, iterator->second);
			const Resource& resource = headerLibrary.GetResource(runtimeResourceID2);

			if (resource.GetResourceHeaderHeader().m_type == 'PRIM')
			{
				hasPrimRuntimeResourceID = true;
				primRuntimeResourceID = runtimeResourceID2;
			}
		}
	}

	return primRuntimeResourceID;
}

void Map::FindProperties(Node& mapNode, Resource& tempResource, const std::vector<EntityTemplate::TemplateSubEntity>& entityTemplates, const std::unordered_map<std::string, unsigned int>& properties)
{
	const unsigned long long primRuntimeResourceID = FindPRIMRuntimeResourceID(tempResource, entityTemplates, properties);

	if (primRuntimeResourceID != -1)
	{
		mapNode.hasPrimRuntimeResourceID = true;
		mapNode.primRuntimeResourceID = primRuntimeResourceID;
	}

	auto iterator = properties.find("m_mTransform");

	if (iterator != properties.end())
	{
		const void* data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tempResource.GetResourceData()) + iterator->second);

		mapNode.hasTransform = true;
		mapNode.transform = *static_cast<const SMatrix43*>(data);
	}

	iterator = properties.find("m_vGlobalSize");

	if (iterator != properties.end())
	{
		const void* data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tempResource.GetResourceData()) + iterator->second);
		mapNode.globalSize = *static_cast<const SVector3*>(data);

		if (mapNode.globalSize.x != 0 && mapNode.globalSize.y != 0 && mapNode.globalSize.z != 0)
		{
			mapNode.hasGlobalSize = true;
		}
	}

	iterator = properties.find("m_eidParent");

	if (iterator != properties.end())
	{
		const void* data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tempResource.GetResourceData()) + iterator->second);
		mapNode.eIDParent = *static_cast<const unsigned int*>(data);

		if (mapNode.eIDParent != -1)
		{
			mapNode.hasEIDParent = true;
		}
	}

	iterator = properties.find("m_bVisible");

	if (iterator != properties.end())
	{
		const void* data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tempResource.GetResourceData()) + iterator->second);
		mapNode.hasVisible = true;
		mapNode.isVisible = *static_cast<const bool*>(data);
	}

	iterator = properties.find("m_rGeomentity");

	if (iterator != properties.end())
	{
		const void* data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tempResource.GetResourceData()) + iterator->second);
		mapNode.hasGeomEntity = true;
		mapNode.geomEntityIndex = *static_cast<const unsigned int*>(data);
	}
}

void Map::ScaleTransform(SMatrix43& transform, const SVector3& scale)
{
	transform.XAxis *= scale.x;
	transform.YAxis *= scale.y;
	transform.ZAxis *= scale.z;
}
