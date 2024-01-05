#include "Glacier/Material/SRMaterialPropertyList.h"

void SRMaterialPropertyList::SerializeToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
{
    writer.StartObject();

    writer.String("lPropertyList");
    writer.Uint(lPropertyList);

    writer.String("lPropertyCount");
    writer.Uint(lPropertyCount);

    writer.String("lMaterialClassType");
    writer.Uint(lMaterialClassType);

    writer.String("lMaterialEffectIndex");
    writer.Uint(lMaterialEffectIndex);

    writer.String("lMaterialClassFlags");
    writer.Uint(lMaterialClassFlags);

    writer.String("lTransparencyFlags");
    writer.Uint(lTransparencyFlags);

    writer.String("lMaterialDescriptor");
    writer.Uint(lMaterialDescriptor);

    writer.String("lImpactMaterial");
    writer.Uint(lImpactMaterial);

    writer.String("lEffectResource");
    writer.Uint(lEffectResource);

    writer.EndObject();
}
