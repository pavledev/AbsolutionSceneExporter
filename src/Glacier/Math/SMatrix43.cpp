#include "Glacier/Math/SMatrix43.h"

SMatrix43::SMatrix43()
{
    m11 = 1; m12 = 0; m13 = 0;
    m21 = 0; m22 = 1; m23 = 0;
    m31 = 0; m32 = 0; m33 = 1;
    m41 = 0; m42 = 0; m43 = 0;
}

void SMatrix43::SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
{
    writer.StartObject();

    writer.String("Rot");
    //Rot.ConvertRotationMatrixToEulerAngles().SerializeToJson(writer);
    Rot.SerializeToJson(writer);

    writer.String("Trans");
    Trans.SerializeToJson(writer);

    writer.EndObject();
}

SMatrix43* SMatrix43::DeserializeFromJson(const rapidjson::Value& object)
{
    SMatrix43* matrix = new SMatrix43();

    //matrix->Rot = SMatrix33::ConvertEulerAnglesToRotationMatrix(*SVector3::DeserializeFromJson(object["Rot"].GetObj()));
    matrix->Rot = *SMatrix33::DeserializeFromJson(object["Rot"].GetObj());
    matrix->Trans = *SVector3::DeserializeFromJson(object["Trans"].GetObj());

    return matrix;
}

bool SMatrix43::operator==(const SMatrix43& other) const
{
    return Rot == other.Rot && Trans == other.Trans;
}
