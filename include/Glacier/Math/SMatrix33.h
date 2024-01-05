#pragma once

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "SVector3.h"

struct SMatrix44;

struct SMatrix33
{
	SMatrix33();
	SMatrix33(float m11, float m12, float m13,
		float m21, float m22, float m23,
		float m31, float m32, float m33);
	void SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	static SMatrix33* DeserializeFromJson(const rapidjson::Value& object);
	bool operator==(const SMatrix33& other) const;
	const SVector3& operator[](const unsigned int row) const;
	SVector3& operator[](const unsigned int row);

	union
	{
		struct
		{
			SVector3 XAxis;
			SVector3 YAxis;
			SVector3 ZAxis;
		};

		struct
		{
			float m11;
			float m12;
			float m13;
			float m21;
			float m22;
			float m23;
			float m31;
			float m32;
			float m33;
		};

		float v[9];
		SVector3 r[3];
	};
};
