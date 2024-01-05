#pragma once

#include <string>

#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

class ZRuntimeResourceID
{
public:
	ZRuntimeResourceID();
	ZRuntimeResourceID(unsigned long long runtimeResourceID);
	ZRuntimeResourceID(unsigned int idHigh, unsigned int idLow);
	operator long long() const;
	unsigned long long GetID() const;
	bool operator==(const ZRuntimeResourceID& other) const;
	bool operator!=(const ZRuntimeResourceID& other) const;
	static ZRuntimeResourceID QueryRuntimeResourceID(const std::string& idResource);
	static std::string QueryResourceID(const ZRuntimeResourceID& ridResource);
	void SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	static ZRuntimeResourceID* DeserializeFromJson(const rapidjson::Value& object);
	const bool IsLibraryResource() const;

private:
	unsigned int m_IDHigh;
	unsigned int m_IDLow;
};
