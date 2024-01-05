#include <cmath>

#include "Glacier/Math/SMatrix33.h"

SMatrix33::SMatrix33()
{
    m11 = 1; m12 = 0; m13 = 0;
    m21 = 0; m22 = 1; m23 = 0;
    m31 = 0; m32 = 0; m33 = 1;
}

SMatrix33::SMatrix33(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
{
    this->m11 = m11; this->m12 = m12; this->m13 = m13;
    this->m21 = m21; this->m22 = m22; this->m23 = m23;
    this->m31 = m31; this->m32 = m32; this->m33 = m33;
}

void SMatrix33::SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
{
    writer.StartObject();

    writer.String("XAxis");
    XAxis.SerializeToJson(writer);

    writer.String("YAxis");
    YAxis.SerializeToJson(writer);

    writer.String("ZAxis");
    ZAxis.SerializeToJson(writer);

    writer.EndObject();
}

SMatrix33* SMatrix33::DeserializeFromJson(const rapidjson::Value& object)
{
    SMatrix33* matrix = new SMatrix33();

    matrix->XAxis = *SVector3::DeserializeFromJson(object["XAxis"].GetObj());
    matrix->YAxis = *SVector3::DeserializeFromJson(object["YAxis"].GetObj());
    matrix->ZAxis = *SVector3::DeserializeFromJson(object["ZAxis"].GetObj());

    return matrix;
}

bool SMatrix33::operator==(const SMatrix33& other) const
{
    return XAxis == other.XAxis && YAxis == other.YAxis && ZAxis == other.ZAxis;
}

const SVector3& SMatrix33::operator[](const unsigned int row) const
{
    return r[row];
}

SVector3& SMatrix33::operator[](const unsigned int row)
{
    return r[row];
}
