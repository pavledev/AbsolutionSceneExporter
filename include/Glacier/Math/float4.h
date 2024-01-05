#pragma once

#include <xmmintrin.h>

#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

struct float4
{
	void SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	static void DeserializeFromJson(float4& float4, const rapidjson::Value& object);

	__m128 m;
};
