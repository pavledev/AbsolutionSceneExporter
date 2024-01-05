#pragma once

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

struct SVector2
{
	SVector2() = default;
	SVector2(const float x, const float y);
	void SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	static SVector2* DeserializeFromJson(const rapidjson::Value& object);
	bool operator==(const SVector2& other) const;

	union
	{
		struct
		{
			float x;
			float y;
		};

		float v[2];
	};
};
