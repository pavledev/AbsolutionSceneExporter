#pragma once

#include <DirectXMath.h>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

struct float4;
struct SVector4;

struct SVector3
{
	SVector3();
	SVector3(const float x, const float y, const float z);
	SVector3(const float4& vector);
	void SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	static SVector3* DeserializeFromJson(const rapidjson::Value& object);
	bool operator==(const SVector3& other) const;
	SVector3& operator*=(const float value);
	SVector3 operator*(const float value) const;

	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};

		float v[3];
	};
};
