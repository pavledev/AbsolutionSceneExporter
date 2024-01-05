#pragma once

#include <string>

#include "DirectXTex/DirectXTex.h"

#include "Glacier/Render/ZTextureMap.h"
#include "Glacier/Render/ERenderFormat.h"
#include "Glacier/Render/SRenderFormatDesc.h"
#include "Glacier/Render/ERenderResourceMipInterpolation.h"

#include "IO/BinaryReader.h"

class Texture
{
public:
    ~Texture();
    void Deserialize(const std::string& texPath);
    void Deserialize(void* data, const unsigned int dataSize);
    void Deserialize(BinaryReader& binaryReader);
    ZTextureMap::SMipLevel GetMipLevel(unsigned int level);
    static DXGI_FORMAT GetDXGIFormat(const ERenderFormat renderFormat);
    static ERenderFormat GetRenderFormat(const DXGI_FORMAT dxgiFormat);
    static DirectX::TEX_DIMENSION GetTexDimension(const ZTextureMap::EDimensions dimension);
    static void GetRenderFormatDescription(SRenderFormatDesc* pDesc, const ERenderFormat& eFormat);
    DirectX::TexMetadata GenerateTexMetadata();
    void ConvertTextureToTGA(const std::string& outputFilePath);
    void ConvertTextureToTGA(const std::wstring& outputFilePath);
    static std::string GetErrorMessage(HRESULT hresult);

private:
    ZTextureMap::STextureMapHeader textureMapHeader;
    unsigned char* data;
    DirectX::TexMetadata texMetadata;
    DirectX::ScratchImage scratchImage;
    DirectX::ScratchImage convertedScratchImage;
};
