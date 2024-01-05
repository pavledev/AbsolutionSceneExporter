#pragma once

#include <string>

#include "StringUtility.h"
#include "Hash.h"
#include "Resources/Resource.h"
#include "Global.h"

class ResourceUtility
{
public:
	static std::string ConvertResourceIDToFilePath(std::string runtimeFolderPath, std::string& resourceID);
	static std::string GetResourceName(const std::string& resourceID);

	static unsigned int CalculateArrayElementsCount(const unsigned int elementsStartOffset, const unsigned int elementsEndOffset, const unsigned int elementSize)
	{
		if (elementsEndOffset - elementsStartOffset == 0)
		{
			return 0;
		}

		return (elementsEndOffset - elementsStartOffset) / elementSize;
	}

	static unsigned long long GetRuntimeResourceID(Resource& tempResource, const unsigned int dataOffset)
	{
		std::vector<Resource*> references;

		tempResource.GetReferences(references);

		BinaryReader binaryReader = BinaryReader(tempResource.GetResourceData(), tempResource.GetResourceDataSize());

		binaryReader.Seek(dataOffset + 4);

		unsigned int referenceIndex = binaryReader.Read<unsigned int>();

		if (referenceIndex != -1)
		{
			return references[referenceIndex]->GetRuntimeResourceID();
		}

		return -1;
	}
};
