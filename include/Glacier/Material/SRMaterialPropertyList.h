#pragma once

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "SRMaterialProperties.h"

struct SRMaterialPropertyList : SRMaterialProperties
{
	void SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);

	unsigned int lPropertyList;
	unsigned int lPropertyCount;
};
