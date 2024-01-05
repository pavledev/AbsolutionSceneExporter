#include <string>
#include <vector>
#include <format>
#include <filesystem>

#include "Global.h"
#include "Registry/PropertyRegistry.h"
#include "Registry/ResourceIDRegistry.h"
#include "Map.h"

extern "C"
{
    __declspec(dllexport) void LoadData(unsigned int sceneIndex, const char* runtimeFolderPath, const char* assetsFolderPath, const char* outputPath);
    __declspec(dllexport) void ExportModel(const unsigned long long primRuntimeResourceID);
    __declspec(dllexport) void ExportAllModels();
    __declspec(dllexport) const char* GetMaterialJson(const unsigned long long matiRuntimeResourceID);
    __declspec(dllexport) const char* GetSceneJson();
}

void CreateFolders()
{
    if (!std::filesystem::exists(outputFolderPath))
    {
        std::filesystem::create_directories(outputFolderPath);
    }

    std::string modelsFolderPath = std::format("{}\\Models", outputFolderPath);
    std::string materialsFolderPath = std::format("{}\\Materials", outputFolderPath);
    std::string texturesFolderPath = std::format("{}\\Textures", outputFolderPath);

    if (!std::filesystem::exists(modelsFolderPath))
    {
        std::filesystem::create_directories(modelsFolderPath);
    }

    if (!std::filesystem::exists(materialsFolderPath))
    {
        std::filesystem::create_directories(materialsFolderPath);
    }

    if (!std::filesystem::exists(texturesFolderPath))
    {
        std::filesystem::create_directories(texturesFolderPath);
    }
}

void LoadData(unsigned int sceneIndex, const char* runtimeFolderPath, const char* assetsFolderPath, const char* outputPath)
{
    static const char* scenes[] =
    {
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l01/l01_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l01b/l01b_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l02b/l02b_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l02c/l02c_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l02d/l02d_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l03/l03_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l04b/l04b_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l04c/l04c_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l04d/l04d_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l04e/l04e_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l05a/l05a_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l05b/l05b_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l05c/l05c_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l06a/l06a_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l06d/l06d_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l07a/l07a_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l08a/l08a_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l08b/l08b_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l09/l09_main.entity).pc_headerlib",
        "[[assembly:/Common/PC.layoutconfig].pc_layoutdef](assembly:/scenes/l10/l10_main.entity).pc_headerlib"
    };

    static unsigned long long sceneTempRuntimeResourceIDs[] =
    {
        0x800007A7A45DBE71,
        0x8000055450379EBE,
        0x80000A11D0691C83,
        0x80000C7ECE5B2712,
        0x80000C95C8296388,
        0x8000077E6B95BEED,
        0x8000053FF51D7907,
        0x800006AB979EBA64,
        0x800008760C5BF6C3,
        0x800002021BA1A9CB,
        0x8000094CD3285297,
        0x80000934793D5071,
        0x8000063880AF864B,
        0x8000072A63433197,
        0x800009846094DB5C,
        0x80000B1588B2C681,
        0x800003C1A1F0FD63,
        0x80000A6BC7DB2865,
        0x8000043163639D28,
        0x80000486B92DD006
    };

    outputFolderPath = outputPath;

    CreateFolders();

    PropertyRegistry::GetInstance().Load(assetsFolderPath);
    ResourceIDRegistry::GetInstance().Load(assetsFolderPath);

    static std::string globalHeaderLibraryResourceID = "[[assembly:/common/globalresources.ini].pc_resourcelibdef].pc_headerlib";
    static bool areGlobalHeaderLibraryResourcesLoaded = false;
    std::string resourceID = scenes[sceneIndex];

    if (!areGlobalHeaderLibraryResourcesLoaded)
    {
        globalHeaderLibrary.ParseHeaderLibrary(runtimeFolderPath, globalHeaderLibraryResourceID);

        areGlobalHeaderLibraryResourcesLoaded = true;
    }

    headerLibrary.ParseHeaderLibrary(runtimeFolderPath, resourceID);

    Map map;

    map.ExportMap(sceneTempRuntimeResourceIDs[sceneIndex], outputFolderPath);
}

void ExportModel(const unsigned long long primRuntimeResourceID)
{
    Resource& primResource = headerLibrary.GetResource(primRuntimeResourceID);
    RenderPrimitive renderPrimitive = RenderPrimitive(primResource);

    primResource.LoadResourceData();
    renderPrimitive.Deserialize();
    renderPrimitive.ConvertToOBJ(outputFolderPath);
    primResource.DeleteResourceData();
}

void ExportAllModels()
{
    for (auto it = primRuntimeResourceIDs.begin(); it != primRuntimeResourceIDs.end(); ++it)
    {
        ExportModel(*it);
    }
}

const char* GetMaterialJson(const unsigned long long matiRuntimeResourceID)
{
    Resource& matiResource = headerLibrary.GetResource(matiRuntimeResourceID);
    RenderMaterialInstance renderMaterialInstance = RenderMaterialInstance(matiResource);

    matiResource.LoadResourceData();
    renderMaterialInstance.Deserialize();
    renderMaterialInstance.SerializeToJson();
    matiResource.DeleteResourceData();

    return materialJsonBuffer.GetString();
}

const char* GetSceneJson()
{
    return sceneJsonBuffer.GetString();
}
