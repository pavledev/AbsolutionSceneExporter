#include "Glacier/Math/SVector3.h"
#include "Glacier/Math/float4.h"

SVector3::SVector3()
{
    x = 0;
    y = 0;
    z = 0;
}

SVector3::SVector3(const float x, const float y, const float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

SVector3::SVector3(const float4& vector)
{
    this->x = vector.m.m128_f32[0];
    this->y = vector.m.m128_f32[1];
    this->z = vector.m.m128_f32[2];
}

void SVector3::SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
{
    writer.StartObject();

    writer.String("x");
    writer.Double(x);

    writer.String("y");
    writer.Double(y);

    writer.String("z");
    writer.Double(z);

    writer.EndObject();
}

SVector3* SVector3::DeserializeFromJson(const rapidjson::Value& object)
{
    SVector3* vector = new SVector3();

    vector->x = object["x"].GetFloat();
    vector->y = object["y"].GetFloat();
    vector->z = object["z"].GetFloat();

    return vector;
}

bool SVector3::operator==(const SVector3& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

SVector3& SVector3::operator*=(const float value)
{
    x *= value;
    y *= value;
    z *= value;

    return *this;
}

SVector3 SVector3::operator*(const float value) const
{
    return SVector3(x * value, y * value, z * value);
}
