#include <format>

#include "Glacier/Resource/ZRuntimeResourceID.h"
#include "Hash.h"
#include "Registry/ResourceIDRegistry.h"
#include "Utility/StringUtility.h"

ZRuntimeResourceID::ZRuntimeResourceID()
{
	m_IDHigh = 0;
	m_IDLow = 0;
}

ZRuntimeResourceID::ZRuntimeResourceID(unsigned long long runtimeResourceID)
{
	m_IDHigh = runtimeResourceID >> 32;
	m_IDLow = static_cast<unsigned int>(runtimeResourceID);
}

ZRuntimeResourceID::ZRuntimeResourceID(unsigned int idHigh, unsigned int idLow)
{
	m_IDHigh = idHigh;
	m_IDLow = idLow;
}

ZRuntimeResourceID::operator long long() const
{
	return GetID();
}

unsigned long long ZRuntimeResourceID::GetID() const
{
	return (static_cast<unsigned long long>(m_IDHigh) << 32) | m_IDLow;
}

ZRuntimeResourceID ZRuntimeResourceID::QueryRuntimeResourceID(const std::string& idResource)
{
	Hash::MD5Hash md5Hash = Hash::MD5(idResource);
	unsigned int idHigh = 0;
	unsigned int idLow = 0;

	idHigh = (idHigh << 8) | ((md5Hash.a >> 8) & 0xFF);
	idHigh = (idHigh << 8) | ((md5Hash.a >> 16) & 0xFF);
	idHigh = (idHigh << 8) | ((md5Hash.a >> 24) & 0xFF);

	idLow = md5Hash.b & 0xFF;
	idLow = (idLow << 8) | ((md5Hash.b >> 8) & 0xFF);
	idLow = (idLow << 8) | ((md5Hash.b >> 16) & 0xFF);
	idLow = (idLow << 8) | ((md5Hash.b >> 24) & 0xFF);

	return ZRuntimeResourceID(idHigh, idLow);
}

std::string ZRuntimeResourceID::QueryResourceID(const ZRuntimeResourceID& ridResource)
{
	return ResourceIDRegistry::GetInstance().GetResourceID(ridResource);
}

bool ZRuntimeResourceID::operator==(const ZRuntimeResourceID& other) const
{
	return GetID() == other.GetID();
}

bool ZRuntimeResourceID::operator!=(const ZRuntimeResourceID& other) const
{
	return GetID() != other.GetID();
}

void ZRuntimeResourceID::SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
{
    writer.StartObject();

    writer.String("m_IDHigh");
    writer.String(StringUtility::ConvertValueToHexString(m_IDHigh).c_str());
    writer.String("m_IDLow");
    writer.String(StringUtility::ConvertValueToHexString(m_IDLow).c_str());
    //writer.String("resourceID");
    //writer.String(QueryResourceID(*this).c_str());

    writer.EndObject();
}

ZRuntimeResourceID* ZRuntimeResourceID::DeserializeFromJson(const rapidjson::Value& object)
{
	ZRuntimeResourceID* runtimeResourceID = new ZRuntimeResourceID();

	runtimeResourceID->m_IDHigh = std::strtoul(object["m_IDHigh"].GetString(), nullptr, 16);
	runtimeResourceID->m_IDLow = std::strtoul(object["m_IDLow"].GetString(), nullptr, 16);

	return runtimeResourceID;
}

const bool ZRuntimeResourceID::IsLibraryResource() const
{
	return m_IDHigh >> 31;
}
